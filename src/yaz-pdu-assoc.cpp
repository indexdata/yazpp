/*
 * Copyright (c) 1998-2001, Index Data.
 * See the file LICENSE for details.
 * 
 * $Log: yaz-pdu-assoc.cpp,v $
 * Revision 1.24  2001-08-13 16:39:12  adam
 * PDU_Assoc keeps track of children. Using yaz_log instead of logf.
 *
 * Revision 1.23  2001/03/26 14:43:49  adam
 * New threaded PDU association.
 *
 * Revision 1.22  2001/01/29 11:18:24  adam
 * Server sets OPTIONS search and present.
 *
 * Revision 1.21  2000/11/20 14:17:36  adam
 * Yet another WIN32 fix for connect notify.
 *
 * Revision 1.20  2000/11/20 11:27:33  adam
 * Fixes for connect operation (timeout and notify fix).
 *
 * Revision 1.19  2000/11/01 14:22:59  adam
 * Added fd parameter for method IYaz_PDU_Observer::clone.
 *
 * Revision 1.18  2000/10/24 12:29:57  adam
 * Fixed bug in proxy where a Yaz_ProxyClient could be owned by
 * two Yaz_Proxy's (fatal).
 *
 * Revision 1.17  2000/10/11 11:58:16  adam
 * Moved header files to include/yaz++. Switched to libtool and automake.
 * Configure script creates yaz++-config script.
 *
 * Revision 1.16  2000/09/22 09:54:11  heikki
 * minor
 *
 * Revision 1.15  2000/09/21 21:43:20  adam
 * Better high-level server API.
 *
 * Revision 1.14  2000/09/12 12:09:53  adam
 * More work on high-level server.
 *
 * Revision 1.13  2000/09/08 10:23:42  adam
 * Added skeleton of yaz-z-server.
 *
 * Revision 1.12  2000/09/06 14:23:45  adam
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
 *g
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
#include <string.h>
#include <yaz/log.h>
#include <yaz/tcpip.h>

#include <yaz++/yaz-pdu-assoc.h>


void Yaz_PDU_Assoc::init(IYazSocketObservable *socketObservable)
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

Yaz_PDU_Assoc::Yaz_PDU_Assoc(IYazSocketObservable *socketObservable)
{
    init (socketObservable);
}

Yaz_PDU_Assoc::Yaz_PDU_Assoc(IYazSocketObservable *socketObservable,
			     COMSTACK cs)
{
    init(socketObservable);
    m_cs = cs;
    unsigned mask = 0;
    if (cs->io_pending & CS_WANT_WRITE)
	mask |= YAZ_SOCKET_OBSERVE_WRITE;
    if (cs->io_pending & CS_WANT_READ)
	mask |= YAZ_SOCKET_OBSERVE_READ;
    m_socketObservable->addObserver(cs_fileno(cs), this);
    if (!mask)
    {
	yaz_log (m_log, "new PDU_Assoc. Ready");
	m_state = Ready;
	flush_PDU();
    }
    else
    {
	yaz_log (m_log, "new PDU_Assoc. Accepting");
	// assume comstack is accepting...
	m_state = Accepting;
	m_socketObservable->addObserver(cs_fileno(cs), this);
	m_socketObservable->maskObserver(this,
					 mask |YAZ_SOCKET_OBSERVE_EXCEPT);
    }
}


IYaz_PDU_Observable *Yaz_PDU_Assoc::clone()
{
    Yaz_PDU_Assoc *copy = new Yaz_PDU_Assoc(m_socketObservable);
    return copy;
}

void Yaz_PDU_Assoc::socketNotify(int event)
{
    yaz_log (m_log, "Yaz_PDU_Assoc::socketNotify p=%p state=%d event = %d",
	  this, m_state, event);
    if (event & YAZ_SOCKET_OBSERVE_EXCEPT)
    {
        close();
        m_PDU_Observer->failNotify();
        return;
    }
    else if (event & YAZ_SOCKET_OBSERVE_TIMEOUT)
    {
        m_PDU_Observer->timeoutNotify();
        return;
    }
    switch (m_state)
    {
    case Accepting:
	if (!cs_accept (m_cs))
	{
	    yaz_log (m_log, "Yaz_PDU_Assoc::cs_accept failed");
	    m_cs = 0;
	    close();
	    m_PDU_Observer->failNotify();
	}
	else
	{
	    unsigned mask = 0;
	    if (m_cs->io_pending & CS_WANT_WRITE)
		mask |= YAZ_SOCKET_OBSERVE_WRITE;
	    if (m_cs->io_pending & CS_WANT_READ)
		mask |= YAZ_SOCKET_OBSERVE_READ;
	    if (!mask)
	    {   // accept is complete. turn to ready state and write if needed
		m_state = Ready;
		flush_PDU();
	    }
	    else  
	    {   // accept still incomplete.
		m_socketObservable->maskObserver(this,
					     mask|YAZ_SOCKET_OBSERVE_EXCEPT);
	    }
	}
	break;
    case Connecting:
	if (event & YAZ_SOCKET_OBSERVE_READ && 
	    event & YAZ_SOCKET_OBSERVE_WRITE)
	{
	    // For Unix: if both read and write is set, then connect failed.
	    close();
	    m_PDU_Observer->failNotify();
	}
	else
	{
	    yaz_log (m_log, "cs_connect again");
	    int res = cs_connect (m_cs, 0);
	    if (res == 1)
	    {
		unsigned mask = YAZ_SOCKET_OBSERVE_EXCEPT;
		if (m_cs->io_pending & CS_WANT_WRITE)
		    mask |= YAZ_SOCKET_OBSERVE_WRITE;
		if (m_cs->io_pending & CS_WANT_READ)
		    mask |= YAZ_SOCKET_OBSERVE_READ;
		m_socketObservable->maskObserver(this, mask);
	    }
	    else
	    {
		m_state = Ready;
		if (m_PDU_Observer)
		    m_PDU_Observer->connectNotify();
		flush_PDU();
	    }
	}
	break;
    case Listen:
	if (event & YAZ_SOCKET_OBSERVE_READ)
	{
	    int res;
	    COMSTACK new_line;
	    
	    if ((res = cs_listen(m_cs, 0, 0)) == 1)
		return;
	    if (res < 0)
	    {
		yaz_log(LOG_FATAL|LOG_ERRNO, "cs_listen failed");
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
	    yaz_log (m_log, "new session: parent fd=%d child fd=%d",
		     cs_fileno(m_cs), cs_fileno(new_line));
	    childNotify (new_line);
	}
	break;
    case Writing:
        if (event & (YAZ_SOCKET_OBSERVE_READ|YAZ_SOCKET_OBSERVE_WRITE))
            flush_PDU();
        break;
    case Ready:
	if (event & (YAZ_SOCKET_OBSERVE_READ|YAZ_SOCKET_OBSERVE_WRITE))
	{
	    do
	    {
		int res = cs_get (m_cs, &m_input_buf, &m_input_len);
		if (res == 1)
                {
                    unsigned mask = YAZ_SOCKET_OBSERVE_EXCEPT;
                    if (m_cs->io_pending & CS_WANT_WRITE)
                        mask |= YAZ_SOCKET_OBSERVE_WRITE;
                    if (m_cs->io_pending & CS_WANT_READ)
                        mask |= YAZ_SOCKET_OBSERVE_READ;
		    m_socketObservable->maskObserver(this, mask);
		    return;
                }
		else if (res <= 0)
		{
		    yaz_log (m_log, "Yaz_PDU_Assoc::Connection closed by peer");
		    close();
		    if (m_PDU_Observer)
			m_PDU_Observer->failNotify(); // problem here..
		    return;
		}
		// lock it, so we know if recv_PDU deletes it.
		int destroyed = 0;
		m_destroyed = &destroyed;

		if (!m_PDU_Observer)
		    return;

		m_PDU_Observer->recv_PDU(m_input_buf, res);
                m_destroyed = 0;
		if (destroyed)   // it really was destroyed, return now.
		    return;
	    } while (m_cs && cs_more (m_cs));
	    if (m_cs)
		m_socketObservable->maskObserver(this,
						 YAZ_SOCKET_OBSERVE_EXCEPT|
						 YAZ_SOCKET_OBSERVE_READ);
	}
	break;
    case Closed:
	yaz_log (m_log, "CLOSING state=%d event was %d", m_state, event);
	close();
	m_PDU_Observer->failNotify();
	break;
    default:
	yaz_log (m_log, "Unknown state=%d event was %d", m_state, event);
	close();
	m_PDU_Observer->failNotify();
    }
}

void Yaz_PDU_Assoc::close()
{
    Yaz_PDU_Assoc *ch;
    for (ch = m_children; ch; ch = ch->m_next)
	ch->close();

    m_socketObservable->deleteObserver(this);
    m_state = Closed;
    if (m_cs)
    {
	yaz_log (m_log, "Yaz_PDU_Assoc::close fd=%d", cs_fileno(m_cs));
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
    yaz_log (m_log, "Yaz_PDU_Assoc::destroy this=%p", this);
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
    
    if (m_state != Ready && m_state != Writing)
    {
        yaz_log (m_log, "YAZ_PDU_Assoc::flush_PDU, not ready");
	return 1;
    }
    PDU_Queue *q = m_queue_out;
    if (!q)
    {
	m_state = Ready;
	yaz_log (m_log, "YAZ_PDU_Assoc::flush_PDU queue empty");
	m_socketObservable->maskObserver(this, YAZ_SOCKET_OBSERVE_READ|
					 YAZ_SOCKET_OBSERVE_WRITE|
					 YAZ_SOCKET_OBSERVE_EXCEPT);
        return 0;
    }
    r = cs_put (m_cs, q->m_buf, q->m_len);
    if (r < 0)
    {
        yaz_log (m_log, "Yaz_PDU_Assoc::flush_PDU cs_put failed");
        close();
	m_PDU_Observer->failNotify();
        return r;
    }
    if (r == 1)
    {
        unsigned mask = YAZ_SOCKET_OBSERVE_EXCEPT;
        m_state = Writing;
        if (m_cs->io_pending & CS_WANT_WRITE)
            mask |= YAZ_SOCKET_OBSERVE_WRITE;
        if (m_cs->io_pending & CS_WANT_READ)
            mask |= YAZ_SOCKET_OBSERVE_READ;
 
	m_socketObservable->maskObserver(this, mask);
        yaz_log (m_log, "Yaz_PDU_Assoc::flush_PDU cs_put %d bytes (incomp)",
		 q->m_len);
        return r;
    } 
    m_state = Ready;
    yaz_log (m_log, "Yaz_PDU_Assoc::flush_PDU cs_put %d bytes", q->m_len);
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
    yaz_log (m_log, "Yaz_PDU_Assoc::send_PDU");
    PDU_Queue **pq = &m_queue_out;
    int is_idle = (*pq ? 0 : 1);
    
    if (!m_cs)
    {
	yaz_log (m_log, "Yaz_PDU_Assoc::send_PDU failed, m_cs == 0");
        return -1;
    }
    while (*pq)
        pq = &(*pq)->m_next;
    *pq = new PDU_Queue(buf, len);
    if (is_idle)
        return flush_PDU ();
    else
	yaz_log (m_log, "Yaz_PDU_Assoc::cannot send_PDU fd=%d",
		 cs_fileno(m_cs));
    return 0;
}

COMSTACK Yaz_PDU_Assoc::comstack(const char *type_and_host, void **vp)
{
    return cs_create_host(type_and_host, 0, vp);
}

void Yaz_PDU_Assoc::listen(IYaz_PDU_Observer *observer,
			   const char *addr)
{
    close();

    yaz_log (LOG_LOG, "Adding listener %s", addr);

    m_PDU_Observer = observer;
    void *ap;
    m_cs = comstack(addr, &ap);

    if (!m_cs)
        return;
    if (cs_bind(m_cs, ap, CS_SERVER) < 0)
        return;
    m_socketObservable->addObserver(cs_fileno(m_cs), this);
    m_socketObservable->maskObserver(this, YAZ_SOCKET_OBSERVE_READ|
				     YAZ_SOCKET_OBSERVE_EXCEPT);
    yaz_log (m_log, "Yaz_PDU_Assoc::listen ok fd=%d", cs_fileno(m_cs));
    m_state = Listen;
}

void Yaz_PDU_Assoc::idleTime(int idleTime)
{
    m_idleTime = idleTime;
    yaz_log (m_log, "Yaz_PDU_Assoc::idleTime(%d)", idleTime);
    m_socketObservable->timeoutObserver(this, m_idleTime);
}

void Yaz_PDU_Assoc::connect(IYaz_PDU_Observer *observer,
			    const char *addr)
{
    yaz_log (m_log, "Yaz_PDU_Assoc::connect %s", addr);
    close();
    m_PDU_Observer = observer;
    void *ap;
    m_cs = comstack(addr, &ap);
    int res = cs_connect (m_cs, ap);
    yaz_log (m_log, "Yaz_PDU_Assoc::connect fd=%d res=%d", cs_fileno(m_cs),
	     res);
    m_socketObservable->addObserver(cs_fileno(m_cs), this);

    if (res >= 0)
    {   // Connect pending or complet
	m_state = Connecting;
	unsigned mask = YAZ_SOCKET_OBSERVE_EXCEPT;
	if (m_cs->io_pending & CS_WANT_WRITE)
	    mask |= YAZ_SOCKET_OBSERVE_WRITE;
	if (m_cs->io_pending & CS_WANT_READ)
	    mask |= YAZ_SOCKET_OBSERVE_READ;
	m_socketObservable->maskObserver(this, mask);
    }
    else
    {   // Connect failed immediately
	// Since m_state is Closed we can distinguish this case from
        // normal connect in socketNotify handler
	m_socketObservable->maskObserver(this, YAZ_SOCKET_OBSERVE_WRITE|
					 YAZ_SOCKET_OBSERVE_EXCEPT);
    }
}

// Single-threaded... Only useful for non-blocking handlers
void Yaz_PDU_Assoc::childNotify(COMSTACK cs)
{

 
    Yaz_PDU_Assoc *new_observable =
	new Yaz_PDU_Assoc (m_socketObservable, cs);
    
    new_observable->m_next = m_children;
    m_children = new_observable;
    new_observable->m_parent = this;

    // Clone PDU Observer
    new_observable->m_PDU_Observer = m_PDU_Observer->sessionNotify
	(new_observable, cs_fileno(cs));
}
