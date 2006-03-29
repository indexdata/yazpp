/*
 * Copyright (c) 2000-2005, Index Data.
 * See the file LICENSE for details.
 * 
 * $Id: z-server.h,v 1.1 2006-03-29 13:14:15 adam Exp $
 */

#include <yazpp/z-assoc.h>

namespace yazpp_1 {

class Z_Server;

class YAZ_EXPORT Z_ServerUtility {
 public:
    void create_databaseRecord (ODR odr, Z_NamePlusRecord *rec,
                                const char *dbname, int format,
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
                            int *format,
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
                             int *next, int *pres,
                             int *oid);

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
 * indent-tabs-mode: nil
 * End:
 * vim: shiftwidth=4 tabstop=8 expandtab
 */

