/*
 * Copyright (c) 1998-1999, Index Data.
 * See the file LICENSE for details.
 * Sebastian Hammer, Adam Dickmeiss
 * 
 * $Log: yaz-socket-manager.cpp,v $
 * Revision 1.1.1.1  1999-01-28 09:41:07  adam
 * First implementation of YAZ++.
 *
 */
#include <assert.h>
#ifdef WINDOWS
#include <winsock.h>
#else
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#endif
#include <errno.h>

#include <log.h>
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
    YazSocketEvent *event = getEvent();
    if (event)
    {
	event->observer->socketNotify(event->event);
	return 1;
    }

    fd_set in, out, except;
    int res;
    int max = 0;
    int no = 0;
    struct timeval to;
    struct timeval *timeout = &to;

    FD_ZERO(&in);
    FD_ZERO(&out);
    FD_ZERO(&except);

    timeout = &to; /* hang on select */
    to.tv_sec = 5*60;
    to.tv_usec = 0;

    for (YazSocketEntry *p = m_observers; p; p = p->next)
    {
	int fd = p->fd;
	logf (LOG_LOG, "fd = %d mask=%d", fd, p->mask);
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
    }
    if (!no)
	return 0;
    while ((res = select(max + 1, &in, &out, &except, timeout)) < 0)
	if (errno != EINTR)
	    return -1;

    for (YazSocketEntry * p = m_observers; p; p = p->next)
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
	    event->observer = p->observer;
	    event->event = mask;
	    putEvent (event);
	}
    }
    if ((event = getEvent()))
    {
	event->observer->socketNotify(event->event);
	return 1;
    }
    return 0;
}

void Yaz_SocketManager::putEvent(YazSocketEvent *event)
{
    logf (LOG_LOG, "putEvent p=%p event=%d", event, event->event);
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
}

Yaz_SocketManager::~Yaz_SocketManager()
{
    deleteObservers();
}
