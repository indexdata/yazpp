/*
 * Copyright (c) 1998-1999, Index Data.
 * See the file LICENSE for details.
 * Sebastian Hammer, Adam Dickmeiss
 * 
 * $Log: yaz-pdu-assoc.cpp,v $
 * Revision 1.3  1999-02-02 14:01:20  adam
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

#include <log.h>
#include <tcpip.h>

Yaz_PDU_Assoc::Yaz_PDU_Assoc(IYazSocketObservable *socketObservable,
			     COMSTACK cs)
{
    m_state = Closed;
    m_cs = cs;
    m_socketObservable = socketObservable;
    m_PDU_Observer = 0;
    m_queue_out = 0;
    m_input_buf = 0;
    m_input_len = 0;
    m_children = 0;
    m_parent = 0;
    m_next = 0;
    m_destroyed = 0;
}

IYaz_PDU_Observable *Yaz_PDU_Assoc::clone()
{
    Yaz_PDU_Assoc *copy = new Yaz_PDU_Assoc(m_socketObservable, 0);
    return copy;
}

Yaz_PDU_Assoc::~Yaz_PDU_Assoc()
{
    destroy();
}

void Yaz_PDU_Assoc::socketNotify(int event)
{
    logf (LOG_LOG, "socketNotify p=%p event = %d", this, event);
    if (m_state == Connected)
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
		logf(LOG_FATAL, "cs_listen failed");
		return;
	    }
	    if (!(new_line = cs_accept(m_cs)))
		return;
	    
	    Yaz_PDU_Assoc *assoc = new Yaz_PDU_Assoc (m_socketObservable,
						      new_line);
	    assoc->m_parent = this;
	    assoc->m_next = m_children;
	    m_children = assoc;
	    
	    assoc->m_PDU_Observer = m_PDU_Observer->clone(assoc);
	    assoc->m_state = Ready;
	    assoc->m_socketObservable->addObserver(cs_fileno(new_line), assoc);
	    assoc->m_socketObservable->maskObserver(assoc,
						    YAZ_SOCKET_OBSERVE_READ|
						    YAZ_SOCKET_OBSERVE_EXCEPT);
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
		    logf (LOG_LOG, "Connection closed by client");
		    close();
		    m_PDU_Observer->failNotify();
		    return;
		}
		// lock it, so we know if recv_PDU deletes it.
		int destroyed = 0;
		m_destroyed = &destroyed;

		m_PDU_Observer->recv_PDU(m_input_buf, res);
		if (destroyed)   // it really was destroyed, return now.
		    return;
	    } while (m_cs && cs_more (m_cs));
	}
    }
}

void Yaz_PDU_Assoc::close()
{
    m_socketObservable->deleteObserver(this);
    m_state = Closed;
    if (m_cs)
    {
	logf (LOG_LOG, "Yaz_PDU_Assoc::close fd=%d", cs_fileno(m_cs));
	cs_close (m_cs);
    }
    m_cs = 0;
    while (m_queue_out)
    {
	PDU_Queue *q_this = m_queue_out;
	m_queue_out = m_queue_out->m_next;
	delete q_this;
    }
//   free (m_input_buf);
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

    if (m_state != Ready)
	return 1;
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
        logf (LOG_LOG, "put %d bytes (incomplete write)", q->m_len);
        return r;
    }
    logf (LOG_LOG, "put %d bytes fd=%d", q->m_len, cs_fileno(m_cs));
    // whole packet sent... delete this and proceed to next ...
    m_queue_out = q->m_next;
    logf (LOG_LOG, "m_queue_out = %p", m_queue_out);
    delete q;
    // don't select on write if queue is empty ...
    if (!m_queue_out)
	m_socketObservable->maskObserver(this, YAZ_SOCKET_OBSERVE_READ|
					 YAZ_SOCKET_OBSERVE_EXCEPT);
    return r;
}

int Yaz_PDU_Assoc::send_PDU(const char *buf, int len)
{
    logf (LOG_LOG, "send_PDU");
    PDU_Queue **pq = &m_queue_out;
    int is_idle = (*pq ? 0 : 1);
    
    if (!m_cs)
    {
	logf (LOG_LOG, "send_PDU failed, m_cs == 0");
        return 0;
    }
    while (*pq)
        pq = &(*pq)->m_next;
    *pq = new PDU_Queue(buf, len);
    if (is_idle)
    {
        return flush_PDU ();
    }
    else
    {
	logf (LOG_LOG, "cannot send_PDU fd=%d", cs_fileno(m_cs));
    }
    return 0;
}

COMSTACK Yaz_PDU_Assoc::comstack()
{
    if (!m_cs)
    {
        CS_TYPE cs_type = tcpip_type;
        int protocol = PROTO_Z3950;
        m_cs = cs_create (cs_type, 0, protocol);
    }
    return m_cs;
}

void Yaz_PDU_Assoc::listen(IYaz_PDU_Observer *observer,
			   const char *addr)
{
    close();
    void *ap;
    COMSTACK cs = comstack();

    logf (LOG_LOG, "Yaz_PDU_Assoc::listen %s", addr);
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

void Yaz_PDU_Assoc::connect(IYaz_PDU_Observer *observer,
			    const char *addr)
{
    logf (LOG_LOG, "Yaz_PDU_Assoc::connect %s", addr);
    close();
    m_PDU_Observer = observer;
    COMSTACK cs = comstack();
    void *ap = cs_straddr (cs, addr);
    if (!ap)
    {
	logf (LOG_LOG, "cs_straddr failed");
	return;
    }
    int res = cs_connect (cs, ap);
    if (res < 0)
    {
	logf (LOG_DEBUG, "Yaz_PDU_Assoc::connect failed");
        close ();
    }
    else
    {
	logf (LOG_LOG, "Yaz_PDU_Assoc::connect fd=%d", cs_fileno(cs));
	m_socketObservable->addObserver(cs_fileno(cs), this);
	m_socketObservable->maskObserver(this, YAZ_SOCKET_OBSERVE_READ|
					 YAZ_SOCKET_OBSERVE_EXCEPT|
					 YAZ_SOCKET_OBSERVE_WRITE);
	if (res == 1)
	    m_state = Connecting;
	else
	    m_state = Connected;
    }
}
