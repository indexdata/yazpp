/*
 * Copyright (c) 1998-2005, Index Data.
 * See the file LICENSE for details.
 * 
 * $Id: socket-manager.h,v 1.5 2005-06-08 13:28:05 adam Exp $
 */

#ifndef YAZ_SOCKET_MANAGER_INCLUDED
#define YAZ_SOCKET_MANAGER_INCLUDED

#include <yaz++/socket-observer.h>
#include <time.h>

namespace yazpp_1 {

/** Simple Socket Manager.
    Implements a stand-alone simple model that uses select(2) to
    observe socket events.
*/
class YAZ_EXPORT SocketManager : public ISocketObservable {
 private:
    struct SocketEntry {
	ISocketObserver *observer;
	int fd;
	unsigned mask;
	int timeout;
        int timeout_this;
	time_t last_activity;
	SocketEntry *next;
    };
    SocketEntry *m_observers;       // all registered observers
    struct SocketEvent {
	ISocketObserver *observer;
	int event;
	SocketEvent *next;          // front in queue
	SocketEvent *prev;          // back in queue
    };
    SocketEvent *m_queue_front;
    SocketEvent *m_queue_back;
    
    SocketEntry **SocketManager::lookupObserver
	(ISocketObserver *observer);
    SocketEvent *SocketManager::getEvent();
    void putEvent(SocketEvent *event);
    void removeEvent(ISocketObserver *observer);
    int m_log;
 public:
    /// Add an observer
    virtual void addObserver(int fd, ISocketObserver *observer);
    /// Delete an observer
    virtual void deleteObserver(ISocketObserver *observer);
    /// Delete all observers
    virtual void deleteObservers();
    /// Set event mask for observer
    virtual void maskObserver(ISocketObserver *observer, int mask);
    /// Set timeout
    virtual void timeoutObserver(ISocketObserver *observer,
				 int timeout);
    /// Process one event. return > 0 if event could be processed;
    int processEvent();
    SocketManager();
    virtual ~SocketManager();
};

};

#endif
