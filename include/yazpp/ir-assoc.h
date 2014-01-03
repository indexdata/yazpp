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

#include <yazpp/z-assoc.h>
#include <yazpp/z-query.h>

namespace yazpp_1 {
/** Information Retrieval Assocation.
    This object implements the client - and server role of a generic
    Z39.50 Association.
*/
class YAZ_EXPORT IR_Assoc: public Z_Assoc {
 public:
    /// Create object using the PDU Observer specified
    IR_Assoc(IPDU_Observable *the_PDU_Observable);
    /// Destroy assocation and close PDU Observer
    virtual ~IR_Assoc();
    /// Receive Z39.50 PDU
    void recv_Z_PDU(Z_APDU *apdu, int len);
    void recv_GDU(Z_GDU *apdu, int len);
    /// Set Database Names
    void set_databaseNames (int num, const char **list);
    void set_databaseNames(const char *dblist, const char *sep);
    /// Get Database Names
    void get_databaseNames (int *num, char ***list);

    void client(const char *addr);

    /// Set Preferred Record Syntax
    void set_preferredRecordSyntax (int value);
    void set_preferredRecordSyntax (const char *syntax);
    /// Get Preferred Record Syntax
    void get_preferredRecordSyntax (int *val);
    void get_preferredRecordSyntax (const char **syntax);

    /// Set ElementSetName
    void set_elementSetName (const char *elementSetName);
    /// Get ElementSetName
    void get_elementSetName (const char **elementSetName);
    void get_elementSetName (Z_ElementSetNames **elementSetNames);

    int get_lastReceived();
    void set_lastReceived(int lastReceived);

    /// Settings
    void set_proxy(const char *str);
    const char *get_proxy();
    const char *get_host();

    void set_cookie(const char *str);
    const char *get_cookie();

    /// Send Services
    int send_initRequest(char* pRefId=NULL);
    int send_searchRequest(Yaz_Z_Query *query, char* pResultSetId = NULL, char* pRefId = NULL);
    int send_presentRequest(Odr_int start, Odr_int number, char* pResultSetId = NULL, char* pRefId = NULL);
    int send_deleteResultSetRequest(char* pResultSetId = 0, char* pRefId = 0);

    /// Recv Services
    virtual void recv_initRequest(Z_InitRequest *initRequest);
    virtual void recv_initResponse(Z_InitResponse *initResponse);
    virtual void recv_searchRequest(Z_SearchRequest *searchRequest);
    virtual void recv_presentRequest(Z_PresentRequest *presentRequest);
    virtual void recv_searchResponse(Z_SearchResponse *searchResponse);
    virtual void recv_presentResponse(Z_PresentResponse *presentResponse);
    virtual void recv_extendedServicesResponse(Z_ExtendedServicesResponse *extendedServicesResponse);
 private:
    char *m_proxy;
    char *m_host;
    char *m_cookie;
    int m_num_databaseNames;
    char **m_databaseNames;
    char *m_preferredRecordSyntax;
    Z_ElementSetNames *m_elementSetNames;
    int m_lastReceived;
    int m_log;
};
};
/*
 * Local variables:
 * c-basic-offset: 4
 * c-file-style: "Stroustrup"
 * indent-tabs-mode: nil
 * End:
 * vim: shiftwidth=4 tabstop=8 expandtab
 */

