/*
 * Copyright (c) 2000, Index Data.
 * See the file LICENSE for details.
 * 
 * $Log: yaz-z-server.cpp,v $
 * Revision 1.4  2000-10-11 11:58:17  adam
 * Moved header files to include/yaz++. Switched to libtool and automake.
 * Configure script creates yaz++-config script.
 *
 * Revision 1.3  2000/09/21 21:43:20  adam
 * Better high-level server API.
 *
 * Revision 1.2  2000/09/12 12:09:53  adam
 * More work on high-level server.
 *
 * Revision 1.1  2000/09/08 10:23:42  adam
 * Added skeleton of yaz-z-server.
 *
 */

#include <yaz/log.h>
#include <yaz++/yaz-z-server.h>


Yaz_Z_Server::Yaz_Z_Server(IYaz_PDU_Observable *the_PDU_Observable)
    : Yaz_Z_Assoc(the_PDU_Observable)
{
}

/*
 * database record.
 */
void Yaz_Z_Server::create_databaseRecord (
    Z_NamePlusRecord *rec, const char *dbname, int format, const void *buf, int len)
{
    rec->databaseName = dbname ? odr_strdup (odr_encode(), dbname) : 0;
    rec->which = Z_NamePlusRecord_databaseRecord;
    rec->u.databaseRecord = z_ext_record (odr_encode(), format,
					  (const char *) buf, len);
}

/*
 * surrogate diagnostic.
 */
void Yaz_Z_Server::create_surrogateDiagnostics(
    Z_NamePlusRecord *rec, const char *dbname, int error, char *const addinfo)
{
    int oid[OID_SIZE];
    int *err = (int *)odr_malloc (odr_encode(), sizeof(*err));
    oident bib1;
    Z_DiagRec *drec = (Z_DiagRec *)odr_malloc (odr_encode(), sizeof(*drec));
    Z_DefaultDiagFormat *dr = (Z_DefaultDiagFormat *)
	odr_malloc (odr_encode(), sizeof(*dr));
    
    bib1.proto = PROTO_Z3950;
    bib1.oclass = CLASS_DIAGSET;
    bib1.value = VAL_BIB1;

    yaz_log(LOG_DEBUG, "SurrogateDiagnotic: %d -- %s", error, addinfo);
    *err = error;
    rec->databaseName = dbname ? odr_strdup (odr_encode(), dbname) : 0;
    rec->which = Z_NamePlusRecord_surrogateDiagnostic;
    rec->u.surrogateDiagnostic = drec;
    drec->which = Z_DiagRec_defaultFormat;
    drec->u.defaultFormat = dr;
    dr->diagnosticSetId = odr_oiddup (odr_encode(),
                                      oid_ent_to_oid(&bib1, oid));
    dr->condition = err;
    dr->which = Z_DefaultDiagFormat_v2Addinfo;
    dr->u.v2Addinfo = odr_strdup (odr_encode(), addinfo ? addinfo : "");
}

Z_Records *Yaz_Z_Server::create_nonSurrogateDiagnostics (
    int error, const char *addinfo)
{
    int oid[OID_SIZE];
    Z_Records *rec = (Z_Records *)
        odr_malloc (odr_encode(), sizeof(*rec));
    oident bib1;
    int *err = (int *)
        odr_malloc (odr_encode(), sizeof(*err));
    Z_DiagRec *drec = (Z_DiagRec *)
        odr_malloc (odr_encode(), sizeof(*drec));
    Z_DefaultDiagFormat *dr = (Z_DefaultDiagFormat *)
        odr_malloc (odr_encode(), sizeof(*dr));

    bib1.proto = PROTO_Z3950;
    bib1.oclass = CLASS_DIAGSET;
    bib1.value = VAL_BIB1;

    *err = error;
    rec->which = Z_Records_NSD;
    rec->u.nonSurrogateDiagnostic = dr;
    dr->diagnosticSetId =
        odr_oiddup (odr_encode(), oid_ent_to_oid(&bib1, oid));
    dr->condition = err;
    dr->which = Z_DefaultDiagFormat_v2Addinfo;
    dr->u.v2Addinfo = odr_strdup (odr_encode(), addinfo ? addinfo : "");
    return rec;
}

Z_Records *Yaz_Z_Server::pack_records (const char *resultSetName,
				       int start, int xnum,
				       Z_RecordComposition *comp,
				       int *next, int *pres,
				       int *format)
{
    int recno, total_length = 0, toget = xnum, dumped_records = 0;
    Z_Records *records =
	(Z_Records *) odr_malloc (odr_encode(), sizeof(*records));
    Z_NamePlusRecordList *reclist =
	(Z_NamePlusRecordList *) odr_malloc (odr_encode(), sizeof(*reclist));
    Z_NamePlusRecord **list =
	(Z_NamePlusRecord **) odr_malloc (odr_encode(), sizeof(*list) * toget);

    records->which = Z_Records_DBOSD;
    records->u.databaseOrSurDiagnostics = reclist;
    reclist->num_records = 0;
    reclist->records = list;
    *pres = Z_PRES_SUCCESS;
    *next = 0;

    yaz_log(LOG_LOG, "Request to pack %d+%d", start, toget);
    yaz_log(LOG_LOG, "pms=%d, mrs=%d", m_preferredMessageSize,
	    m_maximumRecordSize);
    for (recno = start; reclist->num_records < toget; recno++)
    {
	Z_NamePlusRecord *this_rec =
	    (Z_NamePlusRecord *) odr_malloc (odr_encode(), sizeof(*this_rec));
	this_rec->databaseName = 0;
	this_rec->which = Z_NamePlusRecord_databaseRecord;
	this_rec->u.databaseRecord = 0;

	int this_length = 0;

	recv_Z_record (resultSetName, recno, format, comp, this_rec, records);

	if (records->which != Z_Records_DBOSD)
	{
	    *pres = Z_PRES_FAILURE;
	    return records;
	}

	if (this_rec->which == Z_NamePlusRecord_databaseRecord &&
	    this_rec->u.databaseRecord == 0)
	{   // handler did not return a record..
	    create_surrogateDiagnostics(this_rec, 0, 14, 0);
	}
	/*
	 * we get the number of bytes allocated on the stream before any
	 * allocation done by the backend - this should give us a reasonable
	 * idea of the total size of the data so far.
	 */
	total_length = odr_total(odr_encode()) - dumped_records;
	this_length = odr_total(odr_encode()) - total_length;
	yaz_log(LOG_LOG, "  fetched record, len=%d, total=%d",
		this_length, total_length);
	if (this_length + total_length > m_preferredMessageSize)
	{
	    /* record is small enough, really */
	    if (this_length <= m_preferredMessageSize)
	    {
	    	yaz_log(LOG_LOG, "  Dropped last normal-sized record");
		*pres = Z_PRES_PARTIAL_2;
		break;
	    }
	    if (this_length >= m_maximumRecordSize)
	    {   /* too big entirely */
	    	yaz_log(LOG_LOG, "Record > maxrcdsz");
		reclist->records[reclist->num_records] = this_rec;
		create_surrogateDiagnostics(this_rec,
					    this_rec->databaseName, 17, 0);
		reclist->num_records++;
		*next = recno + 1;
		dumped_records += this_length;
		continue;
	    }
	    else /* record can only be fetched by itself */
	    {
	    	yaz_log(LOG_LOG, "  Record > prefmsgsz");
	    	if (toget > 1)
		{
		    yaz_log(LOG_DEBUG, "  Dropped it");
		    reclist->records[reclist->num_records] = this_rec;
		    create_surrogateDiagnostics(this_rec,
						this_rec->databaseName,
						16, 0);
		    reclist->num_records++;
		    // *next = freq.last_in_set ? 0 : recno + 1;
		    *next = recno + 1;
		    dumped_records += this_length;
		    continue;
		}
	    }
	}
	reclist->records[reclist->num_records] = this_rec;
	reclist->num_records++;
	*next = recno + 1;
    }
    return records;
}

void Yaz_Z_Server::fetch_via_piggyback (Z_SearchRequest *req,
					Z_SearchResponse *res)
{
    bool_t *sr = (bool_t *)odr_malloc (odr_encode(), sizeof(*sr));
    *sr = 1;
    
    int toget = 0;
    
    Z_RecordComposition comp, *compp = 0;
    int hits = *res->resultCount;
    
    int *nulint = (int *)odr_malloc (odr_encode(), sizeof(*nulint));
    *nulint = 0;
    
    comp.which = Z_RecordComp_simple;
    /* how many records does the user agent want, then? */
    if (hits <= *req->smallSetUpperBound)
    {
	toget = hits;
	if ((comp.u.simple = req->smallSetElementSetNames))
	    compp = &comp;
    }
    else if (hits < *req->largeSetLowerBound)
    {
	toget = *req->mediumSetPresentNumber;
	if (toget > hits)
	    toget = hits;
	if ((comp.u.simple = req->mediumSetElementSetNames))
	    compp = &comp;
    }
    
    if (toget && !res->records)
    {
	res->presentStatus = (int *) odr_malloc (odr_encode(), sizeof(int));
	*res->presentStatus = Z_PRES_SUCCESS;
	res->records =
	    pack_records(req->resultSetName, 1, toget, compp, 
			 res->nextResultSetPosition,
			 res->presentStatus,
			 req->preferredRecordSyntax);
	if (!res->records)
	    return;
	if (res->records->which == Z_Records_DBOSD)
	    *res->numberOfRecordsReturned =
		res->records->u.databaseOrSurDiagnostics->num_records;
	res->searchStatus = sr;
	res->resultSetStatus = 0;
    }
    else
    {
	if (hits)
	    *res->nextResultSetPosition = 1;
	res->numberOfRecordsReturned = nulint;
	res->searchStatus = sr;
	res->resultSetStatus = 0;
	res->presentStatus = 0;
    }
}

void Yaz_Z_Server::fetch_via_present (Z_PresentRequest *req,
				      Z_PresentResponse *res)
{
    res->records = pack_records (req->resultSetId,*req->resultSetStartPoint, 
				 *req->numberOfRecordsRequested,
				 req->recordComposition,
				 res->nextResultSetPosition,
				 res->presentStatus,
				 req->preferredRecordSyntax);
    if (res->records->which == Z_Records_DBOSD)
	*res->numberOfRecordsReturned =
	    res->records->u.databaseOrSurDiagnostics->num_records;
}
    
void Yaz_Z_Server::recv_Z_PDU (Z_APDU *apdu_request)
{   
    Z_APDU *apdu_response;
    switch (apdu_request->which)
    {
    case Z_APDU_initRequest:
        logf (LOG_LOG, "got InitRequest");
	apdu_response = create_Z_PDU(Z_APDU_initResponse);
	recv_Z_init (apdu_request->u.initRequest,
		     apdu_response->u.initResponse);
	m_preferredMessageSize =
	    *apdu_request->u.initRequest->preferredMessageSize;
	m_maximumRecordSize =
	    *apdu_request->u.initRequest->maximumRecordSize;
	send_Z_PDU(apdu_response);
        break;
    case Z_APDU_searchRequest:
        logf (LOG_LOG, "got SearchRequest");
	apdu_response = create_Z_PDU(Z_APDU_searchResponse);
	recv_Z_search (apdu_request->u.searchRequest,
		       apdu_response->u.searchResponse);
	if (!apdu_response->u.searchResponse->records)
	{
	    fetch_via_piggyback(apdu_request->u.searchRequest,
				apdu_response->u.searchResponse);
	}
	send_Z_PDU(apdu_response);
        break;
    case Z_APDU_presentRequest:
        logf (LOG_LOG, "got PresentRequest");
	apdu_response = create_Z_PDU(Z_APDU_presentResponse);
	recv_Z_present (apdu_request->u.presentRequest,
			apdu_response->u.presentResponse);
	if (!apdu_response->u.presentResponse->records)
	    fetch_via_present(apdu_request->u.presentRequest,
			      apdu_response->u.presentResponse);
	send_Z_PDU(apdu_response);
        break;
    }
}

