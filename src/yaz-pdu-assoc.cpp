/*
 * Copyright (c) 1998-2003, Index Data.
 * See the file LICENSE for details.
 * 
 * $Id: yaz-pdu-assoc.cpp,v 1.37 2003-10-23 11:45:08 adam Exp $
 */

#include <assert.h>
#include <string.h>
#include <yaz/log.h>
#include <yaz/tcpip.h>

#include <yaz++/pdu-assoc.h>


void Yaz_PDU_Assoc::init(IYazSocketObservable *socketObservable)
{
    m_state = Closed;
    m_cs = 0;
    m_socketObservable = socketObservable;
    m_PDU_Observer = 0;
    m_queue_out = 0;
    m_queue_in = 0;
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
	yaz_log(m_log, "maskObserver 1");
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
	        yaz_log(m_log, "maskObserver 2");
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
	    yaz_log (m_log, "cs_rcvconnect");
	    int res = cs_rcvconnect (m_cs);
	    if (res == 1)
	    {
		unsigned mask = YAZ_SOCKET_OBSERVE_EXCEPT;
		if (m_cs->io_pending & CS_WANT_WRITE)
		    mask |= YAZ_SOCKET_OBSERVE_WRITE;
		if (m_cs->io_pending & CS_WANT_READ)
		    mask |= YAZ_SOCKET_OBSERVE_READ;
	        yaz_log(m_log, "maskObserver 3");
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
	            yaz_log(m_log, "maskObserver 4");
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
#if 0
		PDU_Queue **pq = &m_queue_in;
		while (*pq)
		    pq = &(*pq)->m_next;
		
		*pq = new PDU_Queue(m_input_buf, res);
#else
		m_PDU_Observer->recv_PDU(m_input_buf, res);
#endif
		if (destroyed)   // it really was destroyed, return now.
		    return;
                m_destroyed = 0;
	    } while (m_cs && cs_more (m_cs));
	    if (m_cs && m_state == Ready)
            {
	        yaz_log(m_log, "maskObserver 5");
		m_socketObservable->maskObserver(this,
						 YAZ_SOCKET_OBSERVE_EXCEPT|
						 YAZ_SOCKET_OBSERVE_READ);
	    }
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
    m_buf = (char *) xmalloc (len);
    memcpy (m_buf, buf, len);
    m_len = len;
    m_next = 0;
}

Yaz_PDU_Assoc::PDU_Queue::~PDU_Queue()
{
    xfree (m_buf);
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
	yaz_log(m_log, "maskObserver 6");
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

	mask |= YAZ_SOCKET_OBSERVE_WRITE;
	yaz_log(m_log, "maskObserver 7");
	m_socketObservable->maskObserver(this, mask);
        yaz_log (m_log, "Yaz_PDU_Assoc::flush_PDU cs_put %d bytes fd=%d (inc)",
		 q->m_len, cs_fileno(m_cs));
        return r;
    } 
    yaz_log (m_log, "Yaz_PDU_Assoc::flush_PDU cs_put %d bytes", q->m_len);
    // whole packet sent... delete this and proceed to next ...
    m_queue_out = q->m_next;
    delete q;
    // don't select on write if queue is empty ...
    if (!m_queue_out)
    {
        m_state = Ready;
	yaz_log(m_log, "maskObserver 8");
	m_socketObservable->maskObserver(this, YAZ_SOCKET_OBSERVE_READ|
					 YAZ_SOCKET_OBSERVE_EXCEPT);
    }
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
    return cs_create_host(type_and_host, 2, vp);
}

int Yaz_PDU_Assoc::listen(IYaz_PDU_Observer *observer,
			  const char *addr)
{
    close();

    m_PDU_Observer = observer;
    void *ap;
    m_cs = comstack(addr, &ap);

    if (!m_cs)
        return -1;
    if (cs_bind(m_cs, ap, CS_SERVER) < 0)
        return -2;
    m_socketObservable->addObserver(cs_fileno(m_cs), this);
    yaz_log(m_log, "maskObserver 9");
    m_socketObservable->maskObserver(this, YAZ_SOCKET_OBSERVE_READ|
				     YAZ_SOCKET_OBSERVE_EXCEPT);
    yaz_log (m_log, "Yaz_PDU_Assoc::listen ok fd=%d", cs_fileno(m_cs));
    m_state = Listen;
    return 0;
}

void Yaz_PDU_Assoc::idleTime(int idleTime)
{
    m_idleTime = idleTime;
    yaz_log (m_log, "Yaz_PDU_Assoc::idleTime(%d)", idleTime);
    m_socketObservable->timeoutObserver(this, m_idleTime);
}

int Yaz_PDU_Assoc::connect(IYaz_PDU_Observer *observer,
			    const char *addr)
{
    yaz_log (m_log, "Yaz_PDU_Assoc::connect %s", addr);
    close();
    m_PDU_Observer = observer;
    void *ap;
    m_cs = comstack(addr, &ap);
    if (!m_cs)
	return -1;
    int res = cs_connect (m_cs, ap);
    yaz_log (m_log, "Yaz_PDU_Assoc::connect fd=%d res=%d", cs_fileno(m_cs),
	     res);
    m_socketObservable->addObserver(cs_fileno(m_cs), this);

    if (res == 0)
    {   // Connect complete
	m_state = Connecting;
	unsigned mask = YAZ_SOCKET_OBSERVE_EXCEPT;
	mask |= YAZ_SOCKET_OBSERVE_WRITE;
	mask |= YAZ_SOCKET_OBSERVE_READ;
	yaz_log(m_log, "maskObserver 11");
	m_socketObservable->maskObserver(this, mask);
    }
    else if (res > 0)
    {   // Connect pending
	m_state = Connecting;
	unsigned mask = YAZ_SOCKET_OBSERVE_EXCEPT;
	if (m_cs->io_pending & CS_WANT_WRITE)
	    mask |= YAZ_SOCKET_OBSERVE_WRITE;
	if (m_cs->io_pending & CS_WANT_READ)
	    mask |= YAZ_SOCKET_OBSERVE_READ;
	yaz_log(m_log, "maskObserver 11");
	m_socketObservable->maskObserver(this, mask);
    }
    else
    {   // Connect failed immediately
	// Since m_state is Closed we can distinguish this case from
        // normal connect in socketNotify handler
	yaz_log(m_log, "maskObserver 12");
	m_socketObservable->maskObserver(this, YAZ_SOCKET_OBSERVE_WRITE|
					 YAZ_SOCKET_OBSERVE_EXCEPT);
    }
    return 0;
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

const char*Yaz_PDU_Assoc::getpeername()
{
    return cs_addrstr(m_cs);
}
