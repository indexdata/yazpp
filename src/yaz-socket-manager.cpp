/* This file is part of the yazpp toolkit.
 * Copyright (C) Index Data 
 * See the file LICENSE for details.
 */

#if HAVE_CONFIG_H
#include <config.h>
#endif
#if HAVE_SYS_TIME_H
#include <sys/time.h>
#endif
#if HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#if HAVE_UNISTD_H
#include <unistd.h>
#endif

#include <errno.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include <time.h>

#include <yaz/log.h>

#include <yazpp/socket-manager.h>
#include <yaz/poll.h>

using namespace yazpp_1;

struct SocketManager::SocketEntry {
    ISocketObserver *observer;
    int fd;
    unsigned mask;
    int timeout;
    int timeout_this;
    time_t last_activity;
    SocketEntry *next;
};

struct SocketManager::SocketEvent {
    ISocketObserver *observer;
    int event;
    SocketEvent *next;          // front in queue
    SocketEvent *prev;          // back in queue
};

struct SocketManager::Rep {
    void putEvent(SocketEvent *event);
    SocketEvent *getEvent();
    void removeEvent(ISocketObserver *observer);
    void inspect_poll_result(int res, struct yaz_poll_fd *fds, int no_fds,
                             int timeout);
    SocketEntry **lookupObserver(ISocketObserver *observer);
    SocketEntry *observers;       // all registered observers
    SocketEvent *queue_front;
    SocketEvent *queue_back;
    int log;
};

SocketManager::SocketEntry **SocketManager::Rep::lookupObserver(
    ISocketObserver *observer)
{
    SocketEntry **se;

    for (se = &observers; *se; se = &(*se)->next)
        if ((*se)->observer == observer)
            break;
    return se;
}

int SocketManager::getNumberOfObservers()
{
    int i = 0;
    SocketEntry *se;
    for (se = m_p->observers; se; se = se->next, i++)
        ;
    return i;
}

void SocketManager::addObserver(int fd, ISocketObserver *observer)
{
    SocketEntry *se;

    se = *m_p->lookupObserver(observer);
    if (!se)
    {
        se = new SocketEntry;
        se->next= m_p->observers;
        m_p->observers = se;
        se->observer = observer;
    }
    se->fd = fd;
    se->mask = 0;
    se->last_activity = 0;
    se->timeout = -1;
}

void SocketManager::deleteObserver(ISocketObserver *observer)
{
    SocketEntry **se = m_p->lookupObserver(observer);
    if (*se)
    {
        m_p->removeEvent(observer);
        SocketEntry *se_tmp = *se;
        *se = (*se)->next;
        delete se_tmp;
    }
}

void SocketManager::deleteObservers()
{
    SocketEntry *se = m_p->observers;

    while (se)
    {
        SocketEntry *se_next = se->next;
        delete se;
        se = se_next;
    }
    m_p->observers = 0;
}

void SocketManager::maskObserver(ISocketObserver *observer, int mask)
{
    SocketEntry *se;

    yaz_log(m_p->log, "obs=%p read=%d write=%d except=%d", observer,
                    mask & SOCKET_OBSERVE_READ,
                    mask & SOCKET_OBSERVE_WRITE,
                    mask & SOCKET_OBSERVE_EXCEPT);

    se = *m_p->lookupObserver(observer);
    if (se)
        se->mask = mask;
}

void SocketManager::timeoutObserver(ISocketObserver *observer,
                                        int timeout)
{
    SocketEntry *se;

    se = *m_p->lookupObserver(observer);
    if (se)
        se->timeout = timeout;
}

void SocketManager::Rep::inspect_poll_result(int res, struct yaz_poll_fd *fds,
                                             int no_fds, int timeout)

{
    yaz_log(log, "yaz_poll returned res=%d", res);
    time_t now = time(0);
    int i;
    int no_put_events = 0;
    int no_lost_observers = 0;

    for (i = 0; i < no_fds; i++)
    {
        SocketEntry *p;
        for (p = observers; p; p = p->next)
            if (p->fd == fds[i].fd)
                break;
        if (!p)
        {
            // m_p->observers list changed since poll started
            no_lost_observers++;
            continue;
        }

        enum yaz_poll_mask output_mask = fds[i].output_mask;

        int mask = 0;
        if (output_mask & yaz_poll_read)
            mask |= SOCKET_OBSERVE_READ;

        if (output_mask & yaz_poll_write)
            mask |= SOCKET_OBSERVE_WRITE;

        if (output_mask & yaz_poll_except)
            mask |= SOCKET_OBSERVE_EXCEPT;

        if (mask)
        {
            SocketEvent *event = new SocketEvent;
            p->last_activity = now;
            event->observer = p->observer;
            event->event = mask;
            putEvent(event);
            no_put_events++;
            yaz_log(log, "putEvent I/O mask=%d", mask);
        }
        else if (res == 0 && p->timeout_this == timeout)
        {
            SocketEvent *event = new SocketEvent;
            assert(p->last_activity);
            yaz_log(log, "putEvent timeout fd=%d, now = %ld "
                    "last_activity=%ld timeout=%d",
                    p->fd, now, p->last_activity, p->timeout);
            p->last_activity = now;
            event->observer = p->observer;
            event->event = SOCKET_OBSERVE_TIMEOUT;
            putEvent(event);
            no_put_events++;

        }
    }
    SocketEvent *event = getEvent();
    if (event)
    {
        event->observer->socketNotify(event->event);
        delete event;
    }
    else
    {
        if (no_lost_observers == 0)
        {
            // bug #2035
            yaz_log(YLOG_WARN, "unhandled socket event. yaz_poll returned %d",
                    res);
            yaz_log(YLOG_WARN, "no_put_events=%d no_fds=%d i=%d timeout=%d",
                    no_put_events, no_fds, i, timeout);
        }
    }
}

int SocketManager::processEvent()
{
    SocketEntry *p;
    SocketEvent *event = m_p->getEvent();
    int timeout = -1;
    yaz_log(m_p->log, "SocketManager::processEvent manager=%p", this);
    if (event)
    {
        event->observer->socketNotify(event->event);
        delete event;
        return 1;
    }

    int res;
    time_t now = time(0);
    int i;
    int no_fds = 0;
    for (p = m_p->observers; p; p = p->next)
        no_fds++;

    if (!no_fds)
        return 0;
    struct yaz_poll_fd *fds = new yaz_poll_fd [no_fds];
    for (i = 0, p = m_p->observers; p; p = p->next, i++)
    {
        fds[i].fd = p->fd;
        int input_mask = 0;
        if (p->mask & SOCKET_OBSERVE_READ)
            input_mask += yaz_poll_read;
        if (p->mask & SOCKET_OBSERVE_WRITE)
            input_mask += yaz_poll_write;
        if (p->mask & SOCKET_OBSERVE_EXCEPT)
            input_mask += yaz_poll_except;
        if (p->timeout > 0 ||
            (p->timeout == 0 && (p->mask & SOCKET_OBSERVE_WRITE) == 0))
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
            yaz_log(m_p->log, "SocketManager::select timeout_this=%d",
                    p->timeout_this);
        }
        else
            p->timeout_this = -1;
        fds[i].input_mask = (enum yaz_poll_mask) input_mask;
    }

    int pass = 0;
    while ((res = yaz_poll(fds, no_fds, timeout, 0)) < 0 && pass < 10)
    {
        if (errno == EINTR)
        {
            delete [] fds;
            return 1;
        }
        yaz_log(YLOG_ERRNO|YLOG_WARN, "yaz_poll");
        yaz_log(YLOG_WARN, "errno=%d timeout=%d", errno, timeout);
    }

    if (res >= 0)
        m_p->inspect_poll_result(res, fds, no_fds, timeout);

    delete [] fds;
    return res >= 0 ? 1 : -1;
}

//    n p    n p  ......   n p    n p
//   front                        back

void SocketManager::Rep::putEvent(SocketEvent *event)
{
    // put in back of queue
    if (queue_back)
    {
        queue_back->prev = event;
        assert(queue_front);
    }
    else
    {
        assert(!queue_front);
        queue_front = event;
    }
    event->next = queue_back;
    event->prev = 0;
    queue_back = event;
}

SocketManager::SocketEvent *SocketManager::Rep::getEvent()
{
    // get from front of queue
    SocketEvent *event = queue_front;
    if (!event)
        return 0;
    assert(queue_back);
    queue_front = event->prev;
    if (queue_front)
    {
        assert(queue_back);
        queue_front->next = 0;
    }
    else
        queue_back = 0;
    return event;
}

void SocketManager::Rep::removeEvent(ISocketObserver *observer)
{
    SocketEvent *ev = queue_back;
    while (ev)
    {
        SocketEvent *ev_next = ev->next;
        if (observer == ev->observer)
        {
            if (ev->prev)
                ev->prev->next = ev->next;
            else
                queue_back = ev->next;
            if (ev->next)
                ev->next->prev = ev->prev;
            else
                queue_front = ev->prev;
            delete ev;
        }
        ev = ev_next;
    }
}

SocketManager::SocketManager()
{
    m_p = new Rep;
    m_p->observers = 0;
    m_p->queue_front = 0;
    m_p->queue_back = 0;
    m_p->log = YLOG_DEBUG;
}

SocketManager::~SocketManager()
{
    deleteObservers();
    delete m_p;
}
/*
 * Local variables:
 * c-basic-offset: 4
 * c-file-style: "Stroustrup"
 * indent-tabs-mode: nil
 * End:
 * vim: shiftwidth=4 tabstop=8 expandtab
 */

