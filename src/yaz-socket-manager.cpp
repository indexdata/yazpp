/*
 * Copyright (c) 1998-2004, Index Data.
 * See the file LICENSE for details.
 * 
 * $Id: yaz-socket-manager.cpp,v 1.24 2004-01-07 13:40:06 adam Exp $
 */
#include <assert.h>
#ifdef WIN32
#include <winsock.h>
#else
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#endif
#include <errno.h>
#include <string.h>

#include <yaz/log.h>
#include <yaz++/socket-manager.h>

Yaz_SocketManager::YazSocketEntry **Yaz_SocketManager::lookupObserver(
    IYazSocketObserver *observer)
{
    YazSocketEntry **se;
    
    for (se = &m_observers; *se; se = &(*se)->next)
	if ((*se)->observer == observer)
	    break;
    return se;
}

void Yaz_SocketManager::addObserver(int fd, IYazSocketObserver *observer)
{
    YazSocketEntry *se;

    se = *lookupObserver(observer);
    if (!se)
    {
	se = new YazSocketEntry;
	se->next= m_observers;
	m_observers = se;
	se->observer = observer;
    }
    se->fd = fd;
    se->mask = 0;
    se->last_activity = 0;
    se->timeout = -1;
}

void Yaz_SocketManager::deleteObserver(IYazSocketObserver *observer)
{
    YazSocketEntry **se = lookupObserver(observer);
    if (*se)
    {
	removeEvent (observer);
	YazSocketEntry *se_tmp = *se;
	*se = (*se)->next;
	delete se_tmp;
    }
}

void Yaz_SocketManager::deleteObservers()
{
    YazSocketEntry *se = m_observers;
    
    while (se)
    {
	YazSocketEntry *se_next = se->next;
	delete se;
	se = se_next;
    }
    m_observers = 0;
}

void Yaz_SocketManager::maskObserver(IYazSocketObserver *observer, int mask)
{
    YazSocketEntry *se;

    yaz_log(m_log, "obs=%p read=%d write=%d except=%d", observer,
	            mask & YAZ_SOCKET_OBSERVE_READ,
	            mask & YAZ_SOCKET_OBSERVE_WRITE,
	            mask & YAZ_SOCKET_OBSERVE_EXCEPT);

    se = *lookupObserver(observer);
    if (se)
	se->mask = mask;
}

void Yaz_SocketManager::timeoutObserver(IYazSocketObserver *observer,
					int timeout)
{
    YazSocketEntry *se;

    se = *lookupObserver(observer);
    if (se)
	se->timeout = timeout;
}

int Yaz_SocketManager::processEvent()
{
    YazSocketEntry *p;
    YazSocketEvent *event = getEvent();
    int timeout = -1;
    yaz_log (m_log, "Yaz_SocketManager::processEvent manager=%p", this);
    if (event)
    {
	event->observer->socketNotify(event->event);
	delete event;
	return 1;
    }

    fd_set in, out, except;
    int res;
    int max = 0;
    int no = 0;

    FD_ZERO(&in);
    FD_ZERO(&out);
    FD_ZERO(&except);

    time_t now = time(0);
    for (p = m_observers; p; p = p->next)
    {
	int fd = p->fd;
	if (p->mask)
	    no++;
	if (p->mask & YAZ_SOCKET_OBSERVE_READ)
        {
            yaz_log (m_log, "Yaz_SocketManager::select fd=%d read", fd);
	    FD_SET(fd, &in);
        }
	if (p->mask & YAZ_SOCKET_OBSERVE_WRITE)
        {
            yaz_log (m_log, "Yaz_SocketManager::select fd=%d write", fd);
	    FD_SET(fd, &out);
        }
	if (p->mask & YAZ_SOCKET_OBSERVE_EXCEPT)
        {
            yaz_log (m_log, "Yaz_SocketManager::select fd=%d except", fd);
	    FD_SET(fd, &except);
        }
	if (fd > max)
	    max = fd;
	if (p->timeout >= 0)
	{
	    int timeout_this;
	    timeout_this = p->timeout;
	    if (p->last_activity)
		timeout_this -= now - p->last_activity;
	    else
		p->last_activity = now;
	    if (timeout_this < 0 || timeout_this > 2147483646)
		timeout_this = 0;
	    if (timeout == -1 || timeout_this < timeout)
		timeout = timeout_this;
            p->timeout_this = timeout_this;
            yaz_log (m_log, "Yaz_SocketManager::select timeout_this=%d", 
                     p->timeout_this);
	}
    }
    if (!no)
    {
	yaz_log (m_log, "no pending events return 0");
	if (!m_observers)
	    yaz_log (m_log, "no observers");
	return 0;
    }

    struct timeval to;
    to.tv_sec = timeout;
    to.tv_usec = 0;
    
    yaz_log (m_log, "Yaz_SocketManager::select begin no=%d timeout=%d",
             no, timeout);
    while ((res = select(max + 1, &in, &out, &except,
			 timeout== -1 ? 0 : &to)) < 0)
	if (errno != EINTR)
	{
	    yaz_log (LOG_LOG|LOG_WARN, "select");
	    return -1;
	}
    now = time(0);
    for (p = m_observers; p; p = p->next)
    {
	int fd = p->fd;
	int mask = 0;
	if (FD_ISSET(fd, &in))
	    mask |= YAZ_SOCKET_OBSERVE_READ;

	if (FD_ISSET(fd, &out))
	    mask |= YAZ_SOCKET_OBSERVE_WRITE;

	if (FD_ISSET(fd, &except))
	    mask |= YAZ_SOCKET_OBSERVE_EXCEPT;
	
	if (mask)
	{
	    YazSocketEvent *event = new YazSocketEvent;
	    p->last_activity = now;
	    event->observer = p->observer;
	    event->event = mask;
	    putEvent (event);

	    yaz_log (m_log, "putEvent I/O mask=%d", mask);
	}
	else if (p->timeout && (now - p->last_activity) >= p->timeout)
	{
	    YazSocketEvent *event = new YazSocketEvent;
            assert (p->last_activity);
	    yaz_log (m_log, "putEvent timeout, now = %ld last_activity=%ld timeout=%d",
                     now, p->last_activity, p->timeout);
	    p->last_activity = now;
	    event->observer = p->observer;
	    event->event = YAZ_SOCKET_OBSERVE_TIMEOUT;
	    putEvent (event);
	}
    }
    if ((event = getEvent()))
    {
	event->observer->socketNotify(event->event);
	delete event;
	return 1;
    }
    yaz_log (LOG_WARN, "unhandled event in processEvent");
    return 1;
}


//    n p    n p  ......   n p    n p
//   front                        back

void Yaz_SocketManager::putEvent(YazSocketEvent *event)
{
    // put in back of queue
    if (m_queue_back)
    {
	m_queue_back->prev = event;
	assert (m_queue_front);
    }
    else
    {
	assert (!m_queue_front);
	m_queue_front = event;
    }
    event->next = m_queue_back;
    event->prev = 0;
    m_queue_back = event;
}

Yaz_SocketManager::YazSocketEvent *Yaz_SocketManager::getEvent()
{
    // get from front of queue
    YazSocketEvent *event = m_queue_front;
    if (!event)
	return 0;
    assert (m_queue_back);
    m_queue_front = event->prev;
    if (m_queue_front)
    {
	assert (m_queue_back);
	m_queue_front->next = 0;
    }
    else
	m_queue_back = 0;
    return event;
}

void Yaz_SocketManager::removeEvent(IYazSocketObserver *observer)
{
    YazSocketEvent *ev = m_queue_back;
    while (ev)
    {
	YazSocketEvent *ev_next = ev->next;
	if (observer == ev->observer)
	{
	    if (ev->prev)
		ev->prev->next = ev->next;
	    else
		m_queue_back = ev->next;
	    if (ev->next)
		ev->next->prev = ev->prev;
	    else
		m_queue_front = ev->prev;
	    delete ev;
	}
	ev = ev_next;
    }
}

Yaz_SocketManager::Yaz_SocketManager()
{
    m_observers = 0;
    m_queue_front = 0;
    m_queue_back = 0;
    m_log = LOG_DEBUG;
}

Yaz_SocketManager::~Yaz_SocketManager()
{
    deleteObservers();
}
