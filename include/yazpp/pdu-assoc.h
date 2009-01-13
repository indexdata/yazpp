/* This file is part of the yazpp toolkit.
 * Copyright (C) 1998-2009 Index Data and Mike Taylor
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

#ifndef YAZ_PDU_ASSOC_INCLUDED
#define YAZ_PDU_ASSOC_INCLUDED

#include <yaz/comstack.h>
#include <yazpp/socket-observer.h>
#include <yazpp/pdu-observer.h>

namespace yazpp_1 {
/** Simple Protocol Data Unit Assocation.
    This object sends - and receives PDU's using the COMSTACK
    network utility. To use the association in client role, use
    the method connect. The server role is initiated by using the
    listen method.
 */
class YAZ_EXPORT PDU_Assoc : public IPDU_Observable, yazpp_1::ISocketObserver {
    friend class PDU_AssocThread;
 private:
    enum { 
        Connecting,
        Listen,
        Ready,
        Closed,
        Writing,
        Accepting
    } m_state;
    class PDU_Queue {
    public:
        PDU_Queue(const char *buf, int len);
        ~PDU_Queue();
        char *m_buf;
        int m_len;
        PDU_Queue *m_next;
    };
    PDU_Assoc *m_parent;
    PDU_Assoc *m_children;
    PDU_Assoc *m_next;
    COMSTACK m_cs;
    yazpp_1::ISocketObservable *m_socketObservable;
    IPDU_Observer *m_PDU_Observer;
    char *m_input_buf;
    int m_input_len;
    PDU_Queue *m_queue_out;
    PDU_Queue *m_queue_in;
    int flush_PDU();
    int *m_destroyed;
    int m_idleTime;
    int m_log;
    void init(yazpp_1::ISocketObservable *socketObservable);
    bool m_session_is_dead;
 public:
    COMSTACK comstack(const char *type_and_host, void **vp);
    /// Create object using specified socketObservable
    PDU_Assoc(yazpp_1::ISocketObservable *socketObservable);
    /// Create Object using existing comstack
    PDU_Assoc(yazpp_1::ISocketObservable *socketObservable,
                  COMSTACK cs);
    /// Close socket and destroy object.
    /// virtual ~PDU_Assoc();
    /// Clone the object
    IPDU_Observable *clone();
    /// Send PDU
    int send_PDU(const char *buf, int len);
    /// connect to server (client role)
    int connect(IPDU_Observer *observer, const char *addr);
    /// listen for clients (server role)
    int listen(IPDU_Observer *observer, const char *addr);
    /// Socket notification
    void socketNotify(int event);
    /// Close socket
    void shutdown();
    /// Close and destroy
    void destroy();
    /// Set Idle Time
    void idleTime (int timeout);
    /// Child start...
    virtual void childNotify(COMSTACK cs);
    /// close session
    void close_session();
    const char *getpeername();
};

class YAZ_EXPORT PDU_AssocThread : public PDU_Assoc {
 public:
    PDU_AssocThread(yazpp_1::ISocketObservable *socketObservable);
 private:
    void childNotify(COMSTACK cs);

};
};

#endif

/*
 * Local variables:
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * End:
 * vim: shiftwidth=4 tabstop=8 expandtab
 */

