/*
 * Copyright (c) 1998-1999, Index Data.
 * See the file LICENSE for details.
 * Sebastian Hammer, Adam Dickmeiss
 * 
 * $Log: yaz-socket-observer.h,v $
 * Revision 1.3  1999-03-23 14:17:57  adam
 * More work on timeout handling. Work on yaz-client.
 *
 * Revision 1.2  1999/02/02 14:01:17  adam
 * First WIN32 port of YAZ++.
 *
 * Revision 1.1.1.1  1999/01/28 09:41:07  adam
 * First implementation of YAZ++.
 *
 */

#ifndef YAZ_SOCKET_OBSERVER_H
#define YAZ_SOCKET_OBSERVER_H

#define YAZ_SOCKET_OBSERVE_READ 1
#define YAZ_SOCKET_OBSERVE_WRITE 2
#define YAZ_SOCKET_OBSERVE_EXCEPT 4
#define YAZ_SOCKET_OBSERVE_TIMEOUT 8

/**
   Forward reference
 */
class IYazSocketObserver;

/** Socket Observable.
   This interface implements notification of socket events.
   The module interested in (observing) the sockets
   must implement the IYazSocketObserver interface. The
   IYazSocketObserver only have to implement one function, so it's
   quite simple to observe sockets change state.
   The socket events below specifies read, write, exception,
   and timeout respectively:
   <pre>
    YAZ_SOCKET_OBSERVE_READ
    YAZ_SOCKET_OBSERVE_WRITE
    YAZ_SOCKET_OBSERVE_EXCEPT
    YAZ_SOCKET_OBSERVE_TIMEOUT
    </pre>
    The maskObserver method specifies which of these events the
    observer is intertested in.
*/
class YAZ_EXPORT IYazSocketObservable {
 public:
    /// Add an observer interested in socket fd
    virtual void addObserver(int fd, IYazSocketObserver *observer) = 0;
    /// Delete an observer
    virtual void deleteObserver(IYazSocketObserver *observer) = 0;
    /// Delete all observers
    virtual void deleteObservers() = 0;
    /// Specify the events that the observer is intersted in.
    virtual void maskObserver(IYazSocketObserver *observer, int mask) = 0;
    /// Specify timeout
    virtual void timeoutObserver(IYazSocketObserver *observer,
				 unsigned timeout)=0;
};

/** Socket Observer.
   The IYazSocketObserver interface implements a module interested
   socket events. Look for objects that implements the
   IYazSocketObservable interface!
*/
class YAZ_EXPORT IYazSocketObserver {
 public:
    /// Notify the observer that something happened to socket
    virtual void socketNotify(int event) = 0;
};

#endif
