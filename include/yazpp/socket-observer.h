/* This file is part of the yazpp toolkit.
 * Copyright (C) Index Data 
 * All rights reserved.
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of Index Data nor the names of its contributors
 *       may be used to endorse or promote products derived from this
 *       software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE REGENTS AND CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
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
        virtual ~ISocketObservable();
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
        virtual ~ISocketObserver();
    };

};
#endif
/*
 * Local variables:
 * c-basic-offset: 4
 * c-file-style: "Stroustrup"
 * indent-tabs-mode: nil
 * End:
 * vim: shiftwidth=4 tabstop=8 expandtab
 */

