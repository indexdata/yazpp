/*
 * Copyright (c) 2000-2001, Index Data.
 * See the file LICENSE for details.
 * 
 * $Id: yaz-z-server.h,v 1.3 2001-03-27 14:47:45 adam Exp $
 */

#include <yaz++/yaz-z-assoc.h>

class Yaz_Z_Server;

class YAZ_EXPORT IYaz_Server_Facility {
 public:
    virtual int init(Yaz_Z_Server *server,
		     Z_InitRequest *initRequest,
		     Z_InitResponse *initResponse) = 0;
    virtual int recv(Yaz_Z_Server *server, Z_APDU *apdu) = 0;
};


class YAZ_EXPORT Yaz_Facility_ILL : public IYaz_Server_Facility {
 public:
    int init(Yaz_Z_Server *server,
	     Z_InitRequest *initRequest,
	     Z_InitResponse *initResponse);
    int recv(Yaz_Z_Server *server, Z_APDU *apdu);

    virtual int ill_init (Z_InitRequest *initRequest,
			  Z_InitResponse *initResponse) = 0;

    void create_databaseRecord (Z_NamePlusRecord *rec,
				const char *dbname, int format,
				const void *buf, int len);
    void create_surrogateDiagnostics(Z_NamePlusRecord *rec,
				     const char *dbname, int error,
				     char *const addinfo);
    virtual ODR odr_encode();
 private:
    ODR m_odr;
};

class YAZ_EXPORT Yaz_Facility_Retrieval : public IYaz_Server_Facility {
 public:
    int init(Yaz_Z_Server *server,
	     Z_InitRequest *initRequest,
	     Z_InitResponse *initResponse);
    int recv(Yaz_Z_Server *server, Z_APDU *apdu);

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
    void create_databaseRecord (Z_NamePlusRecord *rec,
				const char *dbname, int format,
				const void *buf, int len);
    void create_surrogateDiagnostics(Z_NamePlusRecord *rec,
				     const char *dbname, int error,
				     char *const addinfo);
    virtual ODR odr_encode();
 private:
    Z_Records *pack_records (const char *resultSetName,
			     int start, int num,
			     Z_RecordComposition *comp,
			     int *next, int *pres,
			     int *oid);

    Z_Records *create_nonSurrogateDiagnostics (int error,
					       const char *addinfo);
    
    void fetch_via_piggyback (Z_SearchRequest *searchRequest,
			      Z_SearchResponse *searchResponse);
    void fetch_via_present (Z_PresentRequest *req, Z_PresentResponse *res);

    int m_preferredMessageSize;
    int m_maximumRecordSize;
    ODR m_odr;
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
