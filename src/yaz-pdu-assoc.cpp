/*
 * Copyright (c) 1998-1999, Index Data.
 * See the file LICENSE for details.
 * Sebastian Hammer, Adam Dickmeiss
 * 
 * $Log: yaz-pdu-assoc.cpp,v $
 * Revision 1.12  2000-09-06 14:23:45  adam
 * WIN32 updates.
 *
 * Revision 1.11  2000/09/04 08:29:22  adam
 * Fixed memory leak(s). Added re-use of associations, rather than
 * re-init, when maximum number of targets are in use.
 *
 * Revision 1.10  2000/08/10 08:42:42  adam
 * Fixes for {set,get}_APDU_log.
 *
 * Revision 1.9  1999/12/06 13:52:45  adam
 * Modified for new location of YAZ header files. Experimental threaded
 * operation.
 *
 * Revision 1.8  1999/04/28 13:04:03  adam
 * Fixed setting of proxy otherInfo so that database(s) are removed.
 *
 * Revision 1.7  1999/04/21 12:09:01  adam
 * Many improvements. Modified to proxy server to work with "sessions"
 * based on cookies.
 *
 * Revision 1.6  1999/04/20 10:30:05  adam
 * Implemented various stuff for client and proxy. Updated calls
 * to ODR to reflect new name parameter.
 *
 * Revision 1.5  1999/04/09 11:46:57  adam
 * Added object Yaz_Z_Assoc. Much more functional client.
 *
 * Revision 1.4  1999/03/23 14:17:57  adam
 * More work on timeout handling. Work on yaz-client.
 *
 * Revision 1.3  1999/02/02 14:01:20  adam
 * First WIN32 port of YAZ++.
 *
 * Revision 1.2  1999/01/28 13:08:44  adam
 * Yaz_PDU_Assoc better encapsulated. Memory leak fix in
 * yaz-socket-manager.cc.
 *
 * Revision 1.1.1.1  1999/01/28 09:41:07  adam
 * First implementation of YAZ++.
 *
 */

#include <assert.h>

#include <yaz-pdu-assoc.h>

#include <yaz/log.h>
#include <yaz/tcpip.h>

Yaz_PDU_Assoc::Yaz_PDU_Assoc(IYazSocketObservable *socketObservable)
{
    m_state = Closed;
    m_cs = 0;
    m_socketObservable = socketObservable;
    m_PDU_Observer = 0;
    m_queue_out = 0;
    m_input_buf = 0;
    m_input_len = 0;
    m_children = 0;
    m_parent = 0;
    m_next = 0;
    m_destroyed = 0;
    m_idleTime = 0;
    m_log = LOG_DEBUG;
}

IYaz_PDU_Observable *Yaz_PDU_Assoc::clone()
{
    Yaz_PDU_Assoc *copy = new Yaz_PDU_Assoc(m_socketObservable);
    return copy;
}

Yaz_PDU_Assoc::~Yaz_PDU_Assoc()
{
    destroy();
}

void Yaz_PDU_Assoc::socketNotify(int event)
{
    logf (m_log, "Yaz_PDU_Assoc::socketNotify p=%p event = %d", this, event);
    if (0 /* m_state == Connected */)
    {
	m_state = Ready;
	m_socketObservable->maskObserver(this, YAZ_SOCKET_OBSERVE_READ|
					 YAZ_SOCKET_OBSERVE_EXCEPT);
	m_PDU_Observer->connectNotify();
	flush_PDU();
    }
    else if (m_state == Connecting)
    {
	if (event & YAZ_SOCKET_OBSERVE_READ)
	{
	    close();
	    m_PDU_Observer->failNotify();
	}
	else if (event & YAZ_SOCKET_OBSERVE_TIMEOUT)
        {
	    m_PDU_Observer->timeoutNotify();
        }
        else
	{
	    m_state = Ready;
	    m_socketObservable->maskObserver(this, YAZ_SOCKET_OBSERVE_READ|
					     YAZ_SOCKET_OBSERVE_EXCEPT);
	    m_PDU_Observer->connectNotify();
	    flush_PDU();
	}
    }
    else if (m_state == Listen)
    {
	if (event & YAZ_SOCKET_OBSERVE_READ)
	{
	    int res;
	    COMSTACK new_line;
	    
	    if ((res = cs_listen(m_cs, 0, 0)) == 1)
		return;
	    if (res < 0)
	    {
		logf(LOG_FATAL|LOG_ERRNO, "cs_listen failed");
		return;
	    }
	    if (!(new_line = cs_accept(m_cs)))
		return;
	    
	    /* 1. create socket-manager 
               2. create pdu-assoc
               3. create top-level object
                    setup observer for child fileid in pdu-assoc
               4. start thread
	    */
	    int fd = cs_fileno(new_line);
	    cs_fileno(new_line) = -1;  
	    cs_close (new_line);        /* potential problem ... */
#if 1
	    childNotify(fd);
#else
	    Yaz_PDU_Assoc *assoc = new Yaz_PDU_Assoc (m_socketObservable);
	    assoc->m_parent = this;
	    assoc->m_next = m_children;
	    m_children = assoc;
 
	    assoc->m_PDU_Observer = m_PDU_Observer->clone(assoc);
	    socket(fd);
#endif
	}
    }
    else if (m_state == Ready)
    {
	if (event & YAZ_SOCKET_OBSERVE_WRITE)
	{
	    flush_PDU();
	}
	if (event & YAZ_SOCKET_OBSERVE_READ)
	{
	    do
	    {
		int res = cs_get (m_cs, &m_input_buf, &m_input_len);
		if (res == 1)
		    return;
		else if (res <= 0)
		{
		    logf (m_log, "Connection closed by peer");
		    close();
		    m_PDU_Observer->failNotify();
		    return;
		}
		// lock it, so we know if recv_PDU deletes it.
		int destroyed = 0;
		m_destroyed = &destroyed;

		m_PDU_Observer->recv_PDU(m_input_buf, res);
                m_destroyed = 0;
		if (destroyed)   // it really was destroyed, return now.
		    return;
	    } while (m_cs && cs_more (m_cs));
	}
	if (event & YAZ_SOCKET_OBSERVE_TIMEOUT)
	{
	    m_PDU_Observer->timeoutNotify();
	}
    }
}

void Yaz_PDU_Assoc::close()
{
    m_socketObservable->deleteObserver(this);
    m_state = Closed;
    if (m_cs)
    {
	logf (m_log, "Yaz_PDU_Assoc::close fd=%d", cs_fileno(m_cs));
	cs_close (m_cs);
    }
    m_cs = 0;
    while (m_queue_out)
    {
	PDU_Queue *q_this = m_queue_out;
	m_queue_out = m_queue_out->m_next;
	delete q_this;
    }
    xfree (m_input_buf);
    m_input_buf = 0;
    m_input_len = 0;
}

void Yaz_PDU_Assoc::destroy()
{
    close();
    if (m_destroyed)
	*m_destroyed = 1;
    Yaz_PDU_Assoc **c;

    // delete from parent's child list (if any)
    if (m_parent)
    {
	c = &m_parent->m_children;
	while (*c != this)
	{
	    assert (*c);
	    c = &(*c)->m_next;
	}
	*c = (*c)->m_next;
    }
    // delete all children ...
    c = &m_children;
    while (*c)
    {
	Yaz_PDU_Assoc *here = *c;
	*c = (*c)->m_next;
	here->m_parent = 0;
	delete here;
    }
}

Yaz_PDU_Assoc::PDU_Queue::PDU_Queue(const char *buf, int len)
{
    m_buf = (char *) malloc (len);
    memcpy (m_buf, buf, len);
    m_len = len;
    m_next = 0;
}

Yaz_PDU_Assoc::PDU_Queue::~PDU_Queue()
{
    free (m_buf);
}

int Yaz_PDU_Assoc::flush_PDU()
{
    int r;
    
    logf (m_log, "Yaz_PDU_Assoc::flush_PDU");
    if (m_state != Ready)
    {
        logf (m_log, "YAZ_PDU_Assoc::flush_PDU, not ready");
	return 1;
    }
    PDU_Queue *q = m_queue_out;
    if (!q)
    {
	m_socketObservable->maskObserver(this, YAZ_SOCKET_OBSERVE_READ|
					 YAZ_SOCKET_OBSERVE_EXCEPT);
        return 0;
    }
    r = cs_put (m_cs, q->m_buf, q->m_len);
    if (r < 0)
    {
        close();
	m_PDU_Observer->failNotify();
        return r;
    }
    if (r == 1)
    {
	m_socketObservable->maskObserver(this, YAZ_SOCKET_OBSERVE_READ|
					 YAZ_SOCKET_OBSERVE_EXCEPT|
					 YAZ_SOCKET_OBSERVE_WRITE);
        logf (m_log, "Yaz_PDU_Assoc::flush_PDU put %d bytes (incomplete)",
	      q->m_len);
        return r;
    }
    logf (m_log, "Yaz_PDU_Assoc::flush_PDU put %d bytes", q->m_len);
    // whole packet sent... delete this and proceed to next ...
    m_queue_out = q->m_next;
    delete q;
    // don't select on write if queue is empty ...
    if (!m_queue_out)
	m_socketObservable->maskObserver(this, YAZ_SOCKET_OBSERVE_READ|
					 YAZ_SOCKET_OBSERVE_EXCEPT);
    return r;
}

int Yaz_PDU_Assoc::send_PDU(const char *buf, int len)
{
    logf (m_log, "Yaz_PDU_Assoc::send_PDU");
    PDU_Queue **pq = &m_queue_out;
    int is_idle = (*pq ? 0 : 1);
    
    if (!m_cs)
    {
	logf (m_log, "Yaz_PDU_Assoc::send_PDU failed, m_cs == 0");
        return -1;
    }
    while (*pq)
        pq = &(*pq)->m_next;
    *pq = new PDU_Queue(buf, len);
    if (is_idle)
        return flush_PDU ();
    else
	logf (m_log, "Yaz_PDU_Assoc::cannot send_PDU fd=%d",
	      cs_fileno(m_cs));
    return 0;
}

COMSTACK Yaz_PDU_Assoc::comstack()
{
    if (!m_cs)
    {
        CS_TYPE cs_type = tcpip_type;
        m_cs = cs_create (cs_type, 0, PROTO_Z3950);
    }
    return m_cs;
}

void Yaz_PDU_Assoc::listen(IYaz_PDU_Observer *observer,
			   const char *addr)
{
    close();
    void *ap;
    COMSTACK cs = comstack();

    logf (m_log, "Yaz_PDU_Assoc::listen %s", addr);
    m_PDU_Observer = observer;
    if (!cs)
        return;
    ap = cs_straddr (cs, addr);
    if (!ap)
        return;
    if (cs_bind(cs, ap, CS_SERVER) < 0)
        return;
    m_socketObservable->addObserver(cs_fileno(cs), this);
    m_socketObservable->maskObserver(this, YAZ_SOCKET_OBSERVE_READ|
				     YAZ_SOCKET_OBSERVE_EXCEPT);
    m_state = Listen;
}

void Yaz_PDU_Assoc::idleTime(int idleTime)
{
    m_idleTime = idleTime;
    logf (m_log, "Yaz_PDU_Assoc::idleTime(%d)", idleTime);
    m_socketObservable->timeoutObserver(this, m_idleTime);
}

void Yaz_PDU_Assoc::connect(IYaz_PDU_Observer *observer,
			    const char *addr)
{
    logf (m_log, "Yaz_PDU_Assoc::connect %s", addr);
    close();
    m_PDU_Observer = observer;
    COMSTACK cs = comstack();
    void *ap = cs_straddr (cs, addr);
    if (!ap)
    {
	logf (m_log, "cs_straddr failed");
	return;
    }
    int res = cs_connect (cs, ap);
    logf (m_log, "Yaz_PDU_Assoc::connect fd=%d res=%d", cs_fileno(cs), res);
    m_socketObservable->addObserver(cs_fileno(cs), this);
    m_socketObservable->maskObserver(this, YAZ_SOCKET_OBSERVE_READ|
					   YAZ_SOCKET_OBSERVE_EXCEPT|
					   YAZ_SOCKET_OBSERVE_WRITE);
    m_state = Connecting;
}

void Yaz_PDU_Assoc::socket(IYaz_PDU_Observer *observer, int fd)
{
    close();
    m_PDU_Observer = observer;
    if (fd >= 0)
    {
	CS_TYPE cs_type = tcpip_type;
	m_cs = cs_createbysocket(fd, cs_type, 0, PROTO_Z3950);
	m_state = Ready;
	m_socketObservable->addObserver(fd, this);
	m_socketObservable->maskObserver(this,
					 YAZ_SOCKET_OBSERVE_READ|
					 YAZ_SOCKET_OBSERVE_EXCEPT);
	m_socketObservable->timeoutObserver(this, m_idleTime);
    }
}

#if 1

// Single-threaded... Only useful for non-blocking handlers
void Yaz_PDU_Assoc::childNotify(int fd)
{
    /// Clone PDU Observable (keep socket manager)
    IYaz_PDU_Observable *new_observable = clone();

    /// Clone PDU Observer
    IYaz_PDU_Observer *observer = m_PDU_Observer->clone(new_observable);

    /// Attach new socket to it
    new_observable->socket(observer, fd);
}
#else

#include <yaz-socket-manager.h>

#ifdef WIN32
#include <process.h>
#else
#include <pthread.h>
#endif

void
#ifdef WIN32
__cdecl
#endif 
    events(void *p)
{
    Yaz_SocketManager *s = (Yaz_SocketManager *) p;

    logf (LOG_LOG, "thread started");
    while (s->processEvent() > 0)
	;
    logf (LOG_LOG, "thread finished");
#ifdef WIN32
#else
    return 0;
#endif
}

void Yaz_PDU_Assoc::childNotify(int fd)
{
    Yaz_SocketManager *socket_observable = new Yaz_SocketManager;
    Yaz_PDU_Assoc *new_observable = new Yaz_PDU_Assoc (socket_observable);
    
    /// Clone PDU Observer
    IYaz_PDU_Observer *observer = m_PDU_Observer->clone(new_observable);
    
    /// Attach new socket to it
    new_observable->socket(observer, fd);

#ifdef WIN32
    long t_id;
    t_id = _beginthread (events, 0, socket_observable);
    if (t_id == -1)
    {
        logf (LOG_FATAL|LOG_ERRNO, "_beginthread failed");
        exit (1);
    }
#else
    pthread_t type;

    int id = pthread_create (&type, 0, events, socket_observable);
    logf (LOG_LOG, "pthread_create returned id=%d", id);
#endif
}
// Threads end
#endif
