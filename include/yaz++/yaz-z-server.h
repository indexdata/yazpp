/*
 * Copyright (c) 2000-2001, Index Data.
 * See the file LICENSE for details.
 * 
 * $Id: yaz-z-server.h,v 1.10 2001-05-17 14:18:03 adam Exp $
 */

#include <yaz++/yaz-z-assoc.h>
#if HAVE_YAZ_URSULA_H
#include <yaz/zes-ursula.h>
#endif

class Yaz_Z_Server;

class YAZ_EXPORT Yaz_Z_ServerUtility {
 public:
    void create_databaseRecord (ODR odr, Z_NamePlusRecord *rec,
				const char *dbname, int format,
				const void *buf, int len);
    void create_surrogateDiagnostics(ODR odr, Z_NamePlusRecord *rec,
				     const char *dbname, int error,
				     char *const addinfo);
    
    Z_Records *create_nonSurrogateDiagnostics (ODR odr, int error,
    					       const char *addinfo);
};

class YAZ_EXPORT IYaz_Server_Facility {
 public:
    virtual int init(Yaz_Z_Server *server,
		     Z_InitRequest *initRequest,
		     Z_InitResponse *initResponse) = 0;
    virtual int recv(Yaz_Z_Server *server, Z_APDU *apdu) = 0;
};


#if HAVE_YAZ_URSULA_H
class YAZ_EXPORT Yaz_Facility_Ursula : public IYaz_Server_Facility {
 public:
//    virtual void ursula_service (Z_ExtendedServicesRequest *req,
//				 Z_UrsPDU *u,
//				 Z_ExtendedServicesResponse *res) = 0;
    virtual void ursula_service (Z_ExtendedServicesRequest *req,
			 Z_UrsPDU *u_req,
			 Z_ExtendedServicesResponse *res,
			 Z_UrsPDU *u_res) =0;

    int init(Yaz_Z_Server *server,
	     Z_InitRequest *initRequest,
	     Z_InitResponse *initResponse);
    int recv(Yaz_Z_Server *server, Z_APDU *apdu);
};
#endif

class YAZ_EXPORT Yaz_Facility_ILL : public IYaz_Server_Facility {
 public:
    virtual void ill_service (Z_ExtendedServicesRequest *req,
			      Z_ItemOrder *io,
			      Z_ExtendedServicesResponse *res) = 0;

    int init(Yaz_Z_Server *server,
	     Z_InitRequest *initRequest,
	     Z_InitResponse *initResponse);
    int recv(Yaz_Z_Server *server, Z_APDU *apdu);
};

class YAZ_EXPORT Yaz_Facility_Update : public IYaz_Server_Facility {
 public:
    virtual void update_service (Z_ExtendedServicesRequest *req,
				 Z_IUUpdate *io,
				 Z_ExtendedServicesResponse *res) = 0;

    virtual void update_service0 (Z_ExtendedServicesRequest *req,
				 Z_IU0Update *io,
				 Z_ExtendedServicesResponse *res) = 0;

    int init(Yaz_Z_Server *server,
	     Z_InitRequest *initRequest,
	     Z_InitResponse *initResponse);
    int recv(Yaz_Z_Server *server, Z_APDU *apdu);
};


class YAZ_EXPORT Yaz_Facility_Retrieval : public IYaz_Server_Facility,
    public Yaz_Z_ServerUtility {
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
    int init(Yaz_Z_Server *server,
	     Z_InitRequest *initRequest,
	     Z_InitResponse *initResponse);
    int recv(Yaz_Z_Server *server, Z_APDU *apdu);

    ODR odr_encode();
    ODR odr_decode();
 private:
    Z_Records *pack_records (Yaz_Z_Server *s,
			     const char *resultSetName,
			     int start, int num,
			     Z_RecordComposition *comp,
			     int *next, int *pres,
			     int *oid);

    void fetch_via_piggyback (Yaz_Z_Server *s,
			      Z_SearchRequest *searchRequest,
			      Z_SearchResponse *searchResponse);
    void fetch_via_present (Yaz_Z_Server *s,
			    Z_PresentRequest *req, Z_PresentResponse *res);

    int m_preferredMessageSize;
    int m_maximumRecordSize;
    ODR m_odr_encode;
    ODR m_odr_decode;
};

class YAZ_EXPORT Yaz_Z_Server_Facility_Info {
    friend class Yaz_Z_Server;
    IYaz_Server_Facility *m_facility;
    char *m_name;
    Yaz_Z_Server_Facility_Info *m_next;
};



class YAZ_EXPORT Yaz_Z_Server : public Yaz_Z_Assoc {
public:
    Yaz_Z_Server(IYaz_PDU_Observable *the_PDU_Observable);
    virtual ~Yaz_Z_Server();
    virtual void recv_Z_PDU(Z_APDU *apdu);
    void facility_add(IYaz_Server_Facility *facility, const char *name);
    void facility_reset ();


 private:
    Yaz_Z_Server_Facility_Info *m_facilities;
};

class YAZ_EXPORT Yaz_USMARC {
 public:
    const char *get_record(int position);
};
