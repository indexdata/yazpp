/*
 * Copyright (c) 2000-2001, Index Data.
 * See the file LICENSE for details.
 * 
 * $Id: yaz-z-server.h,v 1.2 2001-03-26 14:43:49 adam Exp $
 */

#include <yaz++/yaz-z-assoc.h>

class Yaz_Z_Server;

class YAZ_EXPORT Yaz_I_Facility {
 public:
    virtual const char *get_name(Yaz_Z_Server *server) = 0;
    virtual int open(Yaz_Z_Server *server,
		     Z_InitRequest *initRequest,
		     Z_InitResponse *initResponse) = 0;
    virtual int recv_PDU(Yaz_Z_Server *server, Z_APDU *apdu) = 0;
    virtual int close(Yaz_Z_Server *server) = 0;
};

class YAZ_EXPORT Yaz_Facility_Retrieval : public Yaz_I_Facility {
 public:
    const char *get_name(Yaz_Z_Server *server);
    int open(Yaz_Z_Server *server,
	     Z_InitRequest *initRequest,
	     Z_InitResponse *initResponse);
    int recv_PDU(Yaz_Z_Server *server, Z_APDU *apdu);
    int close(Yaz_Z_Server *server);
};

class YAZ_EXPORT Yaz_Z_Server : public Yaz_Z_Assoc {
public:
    Yaz_Z_Server(IYaz_PDU_Observable *the_PDU_Observable);
    virtual void recv_Z_PDU(Z_APDU *apdu);
    virtual void recv_Z_init (Z_InitRequest *initRequest,
			      Z_InitResponse *initResponse) = 0;
    virtual void recv_Z_search (Z_SearchRequest *searchRequest,
				Z_SearchResponse *searchResponse) = 0;
    virtual void recv_Z_present (Z_PresentRequest *presentRequest,
				 Z_PresentResponse *presentResponse) = 0;
    virtual void recv_Z_record (const char *resultSetName,
				int position,
				int *format,
				Z_RecordComposition *comp,
				Z_NamePlusRecord *namePlusRecord,
				Z_Records *diagnostics) = 0;
    Z_Records *Yaz_Z_Server::create_nonSurrogateDiagnostics (
	int error, const char *addinfo);
    void create_databaseRecord (Z_NamePlusRecord *rec,
				const char *dbname, int format,
				const void *buf, int len);
    void create_surrogateDiagnostics(Z_NamePlusRecord *rec,
				     const char *dbname, int error,
				     char *const addinfo);
 private:
    Z_Records *pack_records (const char *resultSetName,
			     int start, int num,
			     Z_RecordComposition *comp,
			     int *next, int *pres,
			     int *oid);
    void fetch_via_piggyback (Z_SearchRequest *searchRequest,
			      Z_SearchResponse *searchResponse);
    void fetch_via_present (Z_PresentRequest *req, Z_PresentResponse *res);
    int m_preferredMessageSize;
    int m_maximumRecordSize;
};
