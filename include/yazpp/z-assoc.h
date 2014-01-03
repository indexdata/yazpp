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

#ifndef YAZ_Z_ASSOC_INCLUDED
#define YAZ_Z_ASSOC_INCLUDED

#include <yaz/srw.h>
#include <yaz/proto.h>
#include <yaz/odr.h>
#include <yazpp/pdu-observer.h>

namespace yazpp_1 {
    class Z_Assoc_priv;

/** Z39.50 Assocation.
    This object implements the client - and server role of a generic
    Z39.50 Association.
*/
class YAZ_EXPORT Z_Assoc : public IPDU_Observer {
  public:
        /// Create object using the PDU Observer specified
    Z_Assoc(IPDU_Observable *the_PDU_Observable);
    /// Destroy assocation and close PDU Observer
    virtual ~Z_Assoc();
    /// Receive PDU
    void recv_PDU(const char *buf, int len);
    /// Connect notification
    virtual void connectNotify() = 0;
    /// Failure notification
    virtual void failNotify() = 0;
    /// Timeout notification
    virtual void timeoutNotify() = 0;
    /// Timeout specify
    void timeout(int timeout);
    /// Begin Z39.50 client role
    int client(const char *addr);
    /// Begin Z39.50 server role
    int server(const char *addr);
    /// Close connection
    void close();
    /// Decode Z39.50 PDU.
    Z_GDU *decode_GDU(const char *buf, int len);
    /// Encode Z39.50 PDU.
    int encode_GDU(Z_GDU *apdu, char **buf, int *len);
    /// Send Z39.50 PDU
    int send_Z_PDU(Z_APDU *apdu, int *len);
    int send_GDU(Z_GDU *apdu, int *len);
    /// Receive Z39.50 PDU
    virtual void recv_GDU(Z_GDU *apdu, int len) = 0;
    /// Create Z39.50 PDU with reasonable defaults
    Z_APDU *create_Z_PDU(int type);
    /// Request Alloc
    ODR odr_encode ();
    ODR odr_decode ();
    ODR odr_print ();

    void set_APDU_log(const char *fname);
    const char *get_APDU_log();

    /// OtherInformation
    void get_otherInfoAPDU(Z_APDU *apdu, Z_OtherInformation ***oip);
    Z_OtherInformationUnit *update_otherInformation (
        Z_OtherInformation **otherInformationP, int createFlag,
        const Odr_oid *oid, int categoryValue, int deleteFlag);
    void set_otherInformationString(
        Z_APDU *apdu,
        const Odr_oid *oid, int categoryValue, const char *str);
    void set_otherInformationString (
        Z_OtherInformation **otherInformationP,
        const Odr_oid *oid, int categoryValue,
        const char *str);
    Z_ReferenceId *getRefID(char* str);
    Z_ReferenceId **get_referenceIdP(Z_APDU *apdu);
    void transfer_referenceId(Z_APDU *from, Z_APDU *to);

    const char *get_hostname();

    int set_APDU_yazlog(int v);
  private:
    Z_Assoc_priv *m_p;
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

