/*
 * Copyright (c) 1998-1999, Index Data.
 * See the file LICENSE for details.
 * Sebastian Hammer, Adam Dickmeiss
 * 
 * $Log: yaz-socket-manager.cpp,v $
 * Revision 1.9  2000-08-07 14:19:59  adam
 * Fixed serious bug regarding timeouts. Improved logging for proxy.
 *
 * Revision 1.8  1999/12/06 13:52:45  adam
 * Modified for new location of YAZ header files. Experimental threaded
 * operation.
 *
 * Revision 1.7  1999/04/28 13:02:08  adam
 * Added include of string.h.
 *
 * Revision 1.6  1999/04/21 12:09:01  adam
 * Many improvements. Modified to proxy server to work with "sessions"
 * based on cookies.
 *
 * Revision 1.5  1999/04/09 11:46:57  adam
 * Added object Yaz_Z_Assoc. Much more functional client.
 *
 * Revision 1.4  1999/03/23 14:17:57  adam
 * More work on timeout handling. Work on yaz-client.
 *
 * Revision 1.3  1999/02/02 14:01:23  adam
 * First WIN32 port of YAZ++.
 *
 * Revision 1.2  1999/01/28 13:08:48  adam
 * Yaz_PDU_Assoc better encapsulated. Memory leak fix in
 * yaz-socket-manager.cc.
 *
 * Revision 1.1.1.1  1999/01/28 09:41:07  adam
 * First implementation of YAZ++.
 *
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
#include <yaz-socket-manager.h>


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
    se->timeout = 0;
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

    se = *lookupObserver(observer);
    if (se)
	se->mask = mask;
}

void Yaz_SocketManager::timeoutObserver(IYazSocketObserver *observer,
					unsigned timeout)
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
    unsigned timeout = 0;
    logf (m_log, "processEvent");
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
	    FD_SET(fd, &in);
	if (p->mask & YAZ_SOCKET_OBSERVE_WRITE)
	    FD_SET(fd, &out);
	if (p->mask & YAZ_SOCKET_OBSERVE_EXCEPT)
	    FD_SET(fd, &except);
	if (fd > max)
	    max = fd;
	if (p->timeout)
	{
	    unsigned timeout_this;
	    timeout_this = p->timeout;
	    if (p->last_activity)
		timeout_this -= now - p->last_activity;
	    if (timeout_this < 1)
		timeout_this = 1;
	    if (!timeout || timeout_this < timeout)
		timeout = timeout_this;
	}
    }
    if (!no)
    {
	logf (m_log, "no pending events return 0");
	if (!m_observers)
	    logf (m_log, "no observers");
	return 0;
    }

    struct timeval to;
    to.tv_sec = timeout;
    to.tv_usec = 0;
    
    logf (m_log, "select pending=%d timeout=%d", no, timeout);
    while ((res = select(max + 1, &in, &out, &except, timeout ? &to : 0)) < 0)
	if (errno != EINTR)
	    return -1;
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
	}
	else if (p->timeout && p->last_activity && 
		 now >= p->last_activity + (int) (p->timeout))
	{
	    YazSocketEvent *event = new YazSocketEvent;
	    logf (LOG_LOG, "timeout now = %ld last_activity=%ld timeout=%d",
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
    return 0;
}

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
