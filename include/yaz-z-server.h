/*
 * Copyright (c) 2000, Index Data.
 * See the file LICENSE for details.
 * 
 * $Id: yaz-z-server.h,v 1.2 2000-09-12 12:09:53 adam Exp $
 */

#include <yaz-z-assoc.h>

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
				Z_DefaultDiagFormat *diagnostics) = 0;
 private:
    Z_Records *pack_records (const char *resultSetName,
			     int start, int *num,
			     Z_RecordComposition *comp,
			     int *next, int *pres,
			     int *oid);
    void piggyback (Z_SearchRequest *searchRequest,
		    Z_SearchResponse *searchResponse);
    int m_preferredMessageSize;
    int m_maximumRecordSize;
};
