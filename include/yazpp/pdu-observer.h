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

#ifndef YAZ_PDU_OBSERVER_H
#define YAZ_PDU_OBSERVER_H

#include <yaz/yconfig.h>

namespace yazpp_1 {

class IPDU_Observer;

/** Protocol Data Unit Observable.
    This interface implements a Protocol Data Unit (PDU) network driver.
    The PDU's is not encoded/decoded by this interface. They are simply
    transmitted/received over the network. To use this interface the
    IPDU_Observer interface must be implemented.
 */
class YAZ_EXPORT IPDU_Observable {
 public:
    /// Send encoded PDU buffer of specified length
    virtual int send_PDU(const char *buf, int len) = 0;
    /// Connect with server specified by addr.
    virtual int connect(IPDU_Observer *observer, const char *addr) = 0;
    /// Listen on address addr.
    virtual int listen(IPDU_Observer *observer, const char *addr) = 0;
    /// Close connection
    virtual void shutdown() = 0;
    /// Make clone of this object using this interface
    virtual IPDU_Observable *clone() = 0;
    /// Destroy completely
    virtual void destroy() = 0;
    /// Set Idle Time
    virtual void idleTime (int timeout) = 0;
    /// Get peername
    virtual const char *getpeername() = 0;
    /// Close session
    virtual void close_session() = 0;

    virtual ~IPDU_Observable();
};

/** Protocol Data Unit Observer.
    This interface is used together with the IPDU_Observable interface
    and acts as a callback interface for it.
 */
class YAZ_EXPORT IPDU_Observer {
 public:
    /// A PDU has been received
    virtual void recv_PDU(const char *buf, int len) = 0;
    /// Called when Iyaz_PDU_Observable::connect was successful.
    virtual void connectNotify() = 0;
    /// Called whenever the connection was closed
    virtual void failNotify() = 0;
    /// Called whenever there is a timeout
    virtual void timeoutNotify() = 0;
    /// Make clone of observer using IPDU_Observable interface
    virtual IPDU_Observer *sessionNotify(
        IPDU_Observable *the_PDU_Observable, int fd) = 0;

    virtual ~IPDU_Observer();
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

