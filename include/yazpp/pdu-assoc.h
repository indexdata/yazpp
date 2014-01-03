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

#ifndef YAZ_PDU_ASSOC_INCLUDED
#define YAZ_PDU_ASSOC_INCLUDED

#include <yaz/comstack.h>
#include <yazpp/socket-observer.h>
#include <yazpp/pdu-observer.h>

namespace yazpp_1 {
    class PDU_Assoc_priv;

/** Simple Protocol Data Unit Assocation.
    This object sends - and receives PDU's using the COMSTACK
    network utility. To use the association in client role, use
    the method connect. The server role is initiated by using the
    listen method.
 */
class YAZ_EXPORT PDU_Assoc : public IPDU_Observable, yazpp_1::ISocketObserver {
    friend class PDU_AssocThread;
    PDU_Assoc_priv *m_p;
    IPDU_Observer *m_PDU_Observer;
    int flush_PDU();
 public:
    PDU_Assoc(yazpp_1::ISocketObservable *socketObservable);

    PDU_Assoc(yazpp_1::ISocketObservable *socketObservable, COMSTACK cs);

    COMSTACK get_comstack(); 

    virtual ~PDU_Assoc();

    // optional feature implemented by PDU_Assoc (also by PDU_Asso_Thread)
    virtual void childNotify(COMSTACK cs);

    // mefhods below are from IPDU_Observable
    IPDU_Observable *clone();
    int send_PDU(const char *buf, int len);
    int connect(IPDU_Observer *observer, const char *addr);
    int listen(IPDU_Observer *observer, const char *addr);
    void socketNotify(int event);
    void shutdown();
    void destroy();
    void idleTime(int timeout);
    void close_session();
    const char *getpeername();
    void set_cert_fname(const char *fname);
};

class YAZ_EXPORT PDU_AssocThread : public PDU_Assoc {
 public:
    PDU_AssocThread(yazpp_1::ISocketObservable *socketObservable);
    virtual ~PDU_AssocThread();
 private:
    void childNotify(COMSTACK cs);

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

