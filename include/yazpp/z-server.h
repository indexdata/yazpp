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

namespace yazpp_1 {

class Z_Server;

class YAZ_EXPORT Z_ServerUtility {
 public:
    void create_databaseRecord (ODR odr, Z_NamePlusRecord *rec,
                                const char *dbname, const Odr_oid *format,
                                const void *buf, int len);
    void create_surrogateDiagnostics(ODR odr, Z_NamePlusRecord *rec,
                                     const char *dbname, int error,
                                     char *const addinfo);

    Z_Records *create_nonSurrogateDiagnostics (ODR odr, int error,
                                               const char *addinfo);

    void create_diagnostics (
        ODR odr, int error, const char *addinfo,
        Z_DiagRec ***dreca, int *num);

    virtual ~Z_ServerUtility() = 0;
};

class YAZ_EXPORT IServer_Facility {
 public:
    virtual int init(Z_Server *server,
                     Z_InitRequest *initRequest,
                     Z_InitResponse *initResponse) = 0;
    virtual int recv(Z_Server *server, Z_APDU *apdu) = 0;

    virtual ~IServer_Facility() = 0;
};

class YAZ_EXPORT Yaz_Facility_ILL : public IServer_Facility {
 public:
    virtual void ill_service (Z_ExtendedServicesRequest *req,
                              Z_ItemOrder *io,
                              Z_ExtendedServicesResponse *res) = 0;

    int init(Z_Server *server,
             Z_InitRequest *initRequest,
             Z_InitResponse *initResponse);
    int recv(Z_Server *server, Z_APDU *apdu);
};

class YAZ_EXPORT Yaz_Facility_Update : public IServer_Facility {
 public:
    virtual void update_service (Z_ExtendedServicesRequest *req,
                                 Z_IUUpdate *io,
                                 Z_ExtendedServicesResponse *res) = 0;

    virtual void update_service0 (Z_ExtendedServicesRequest *req,
                                 Z_IU0Update *io,
                                 Z_ExtendedServicesResponse *res) = 0;

    int init(Z_Server *server,
             Z_InitRequest *initRequest,
             Z_InitResponse *initResponse);
    int recv(Z_Server *server, Z_APDU *apdu);
};


class YAZ_EXPORT Yaz_Facility_Retrieval : public IServer_Facility,
    public Z_ServerUtility {
 public:

    virtual int sr_init (Z_InitRequest *initRequest,
                         Z_InitResponse *initResponse) = 0;
    virtual void sr_search (Z_SearchRequest *searchRequest,
                            Z_SearchResponse *searchResponse) = 0;
    virtual void sr_present (Z_PresentRequest *presentRequest,
                             Z_PresentResponse *presentResponse) = 0;
    virtual void sr_record (const char *resultSetName,
                            int position,
                            Odr_oid *format,
                            Z_RecordComposition *comp,
                            Z_NamePlusRecord *namePlusRecord,
                            Z_Records *diagnostics) = 0;
    int init(Z_Server *server,
             Z_InitRequest *initRequest,
             Z_InitResponse *initResponse);
    int recv(Z_Server *server, Z_APDU *apdu);

    ODR odr_encode();
    ODR odr_decode();
 private:
    Z_Records *pack_records (Z_Server *s,
                             const char *resultSetName,
                             int start, int num,
                             Z_RecordComposition *comp,
                             Odr_int *next, Odr_int *pres,
                             Odr_oid *oid);

    void fetch_via_piggyback (Z_Server *s,
                              Z_SearchRequest *searchRequest,
                              Z_SearchResponse *searchResponse);
    void fetch_via_present (Z_Server *s,
                            Z_PresentRequest *req, Z_PresentResponse *res);

    int m_preferredMessageSize;
    int m_maximumRecordSize;
    ODR m_odr_encode;
    ODR m_odr_decode;
};

class YAZ_EXPORT Z_Server_Facility_Info {
    friend class Z_Server;
    IServer_Facility *m_facility;
    char *m_name;
    Z_Server_Facility_Info *m_next;
};



class YAZ_EXPORT Z_Server : public Z_Assoc {
public:
    Z_Server(IPDU_Observable *the_PDU_Observable);
    virtual ~Z_Server();
    void recv_Z_PDU(Z_APDU *apdu, int len);
    virtual void recv_GDU(Z_GDU *apdu, int len);
    void facility_add(IServer_Facility *facility, const char *name);
    void facility_reset ();


 private:
    Z_Server_Facility_Info *m_facilities;
};

class YAZ_EXPORT Yaz_USMARC {
 public:
    const char *get_record(size_t position);
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

