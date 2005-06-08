/*
 * Copyright (c) 1998-2005, Index Data.
 * See the file LICENSE for details.
 * 
 * $Id: socket-observer.h,v 1.5 2005-06-08 13:28:05 adam Exp $
 */

#ifndef YAZ_SOCKET_OBSERVER_H
#define YAZ_SOCKET_OBSERVER_H

#include <yaz/yconfig.h>

namespace yazpp_1 {
    
    enum SocketObserve {
	SOCKET_OBSERVE_READ=1,
	SOCKET_OBSERVE_WRITE=2,
	SOCKET_OBSERVE_EXCEPT=4,
	SOCKET_OBSERVE_TIMEOUT=8
    };

/**
   Forward reference
*/
    class ISocketObserver;
    
/** Socket Observable.
    This interface implements notification of socket events.
    The module interested in (observing) the sockets
    must implement the ISocketObserver interface. The
    ISocketObserver only have to implement one function, so it's
    quite simple to observe sockets change state.
    The socket events below specifies read, write, exception,
    and timeout respectively:
    <pre>
    SOCKET_OBSERVE_READ
    SOCKET_OBSERVE_WRITE
    SOCKET_OBSERVE_EXCEPT
    SOCKET_OBSERVE_TIMEOUT
    </pre>
    The maskObserver method specifies which of these events the
    observer is intertested in.
*/
class YAZ_EXPORT ISocketObservable {
 public:
    /// Add an observer interested in socket fd
    virtual void addObserver(int fd, ISocketObserver *observer) = 0;
    /// Delete an observer
    virtual void deleteObserver(ISocketObserver *observer) = 0;
    /// Delete all observers
    virtual void deleteObservers() = 0;
    /// Specify the events that the observer is intersted in.
    virtual void maskObserver(ISocketObserver *observer, int mask) = 0;
    /// Specify timeout
    virtual void timeoutObserver(ISocketObserver *observer,
				 int timeout)=0;
};

/** Socket Observer.
   The ISocketObserver interface implements a module interested
   socket events. Look for objects that implements the
   ISocketObservable interface!
*/
class YAZ_EXPORT ISocketObserver {
 public:
    /// Notify the observer that something happened to socket
    virtual void socketNotify(int event) = 0;
};

};
#endif
