/*
 * Copyright (c) 2000, Index Data.
 * See the file LICENSE for details.
 * 
 * $Log: yaz-z-server.cpp,v $
 * Revision 1.2  2000-09-12 12:09:53  adam
 * More work on high-level server.
 *
 * Revision 1.1  2000/09/08 10:23:42  adam
 * Added skeleton of yaz-z-server.
 *
 */

#include <yaz/log.h>
#include <yaz-z-server.h>


Yaz_Z_Server::Yaz_Z_Server(IYaz_PDU_Observable *the_PDU_Observable)
    : Yaz_Z_Assoc(the_PDU_Observable)
{
}

Z_Records *Yaz_Z_Server::pack_records (const char *resultSetName,
				       int start, int *num,
				       Z_RecordComposition *comp,
				       int *next, int *pres,
				       int *format)
{
#if 0
    int recno, total_length = 0, toget = *num, dumped_records = 0;
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
    *num = 0;
    *next = 0;

    yaz_log(LOG_LOG, "Request to pack %d+%d", start, toget);
    yaz_log(LOG_DEBUG, "pms=%d, mrs=%d", m_preferredMessageSize,
    	m_maximumRecordSize);
    for (recno = start; reclist->num_records < toget; recno++)
    {
	Z_NamePlusRecord *rec =
	    (Z_NamePlusRecord *) odr_malloc (odr_encode(), sizeof(*rec));
	rec->databaseName = 0;
	rec->which = Z_NamePlusRecord_databaseRecord;
	rec->u.databaseRecord = 0;

	Z_DefaultDiagFormat *diagnostics = (Z_DefaultDiagFormat *)
	    odr_malloc (odr_encode(), sizeof(*diagnostics));
	diagnostics->diagnosticSetId = 0;
	diagnostics->condition = 0;
	diagnostics->which = Z_DefaultDiagFormat_v2Addinfo;
	
	recv_Z_record (resultSetName, recno, format, comp, rec, diagnostics);

    	bend_fetch_rr freq;
	Z_NamePlusRecord *thisrec;
	int this_length = 0;
	/*
	 * we get the number of bytes allocated on the stream before any
	 * allocation done by the backend - this should give us a reasonable
	 * idea of the total size of the data so far.
	 */
	total_length = odr_total(a->encode) - dumped_records;
	freq.errcode = 0;
	freq.errstring = 0;
	freq.basename = 0;
	freq.len = 0;
	freq.record = 0;
	freq.last_in_set = 0;
	freq.setname = setname;
	freq.surrogate_flag = 0;
	freq.number = recno;
	freq.comp = comp;
	freq.request_format = format;
	freq.request_format_raw = oid;
	freq.output_format = format;
	freq.output_format_raw = 0;
	freq.stream = a->encode;
	freq.print = a->print;
	freq.surrogate_flag = 0;
	freq.referenceId = referenceId;
	(*a->init->bend_fetch)(a->backend, &freq);
	/* backend should be able to signal whether error is system-wide
	   or only pertaining to current record */
	if (freq.errcode)
	{
	    if (!freq.surrogate_flag)
	    {
		*pres = Z_PRES_FAILURE;
		return diagrec(a, freq.errcode, freq.errstring);
	    }
	    reclist->records[reclist->num_records] =
		surrogatediagrec(a, freq.basename, freq.errcode,
				 freq.errstring);
	    reclist->num_records++;
	    *next = freq.last_in_set ? 0 : recno + 1;
	    continue;
	}
	if (freq.len >= 0)
	    this_length = freq.len;
	else
	    this_length = odr_total(a->encode) - total_length;
	yaz_log(LOG_DEBUG, "  fetched record, len=%d, total=%d",
	    this_length, total_length);
	if (this_length + total_length > a->preferredMessageSize)
	{
	    /* record is small enough, really */
	    if (this_length <= a->preferredMessageSize)
	    {
	    	yaz_log(LOG_DEBUG, "  Dropped last normal-sized record");
		*pres = Z_PRES_PARTIAL_2;
		break;
	    }
	    /* record can only be fetched by itself */
	    if (this_length < a->maximumRecordSize)
	    {
	    	yaz_log(LOG_DEBUG, "  Record > prefmsgsz");
	    	if (toget > 1)
		{
		    yaz_log(LOG_DEBUG, "  Dropped it");
		    reclist->records[reclist->num_records] =
		   	 surrogatediagrec(a, freq.basename, 16, 0);
		    reclist->num_records++;
		    *next = freq.last_in_set ? 0 : recno + 1;
		    dumped_records += this_length;
		    continue;
		}
	    }
	    else /* too big entirely */
	    {
	    	yaz_log(LOG_DEBUG, "Record > maxrcdsz");
		reclist->records[reclist->num_records] =
		    surrogatediagrec(a, freq.basename, 17, 0);
		reclist->num_records++;
		*next = freq.last_in_set ? 0 : recno + 1;
		dumped_records += this_length;
		continue;
	    }
	}

	if (!(thisrec = (Z_NamePlusRecord *)
	      odr_malloc(a->encode, sizeof(*thisrec))))
	    return 0;
	if (!(thisrec->databaseName = (char *)odr_malloc(a->encode,
	    strlen(freq.basename) + 1)))
	    return 0;
	strcpy(thisrec->databaseName, freq.basename);
	thisrec->which = Z_NamePlusRecord_databaseRecord;

	if (freq.output_format_raw)
	{
	    struct oident *ident = oid_getentbyoid(freq.output_format_raw);
	    freq.output_format = ident->value;
	}
	thisrec->u.databaseRecord = z_ext_record(a->encode, freq.output_format,
						 freq.record, freq.len);
	if (!thisrec->u.databaseRecord)
	    return 0;
	reclist->records[reclist->num_records] = thisrec;
	reclist->num_records++;
	*next = freq.last_in_set ? 0 : recno + 1;
    }
    *num = reclist->num_records;
    return records;
#endif
    return 0;
}

void Yaz_Z_Server::piggyback (Z_SearchRequest *req,
			      Z_SearchResponse *res)
{
    bool_t *sr = (bool_t *)odr_malloc (odr_encode(), sizeof(*sr));
    *sr = 1;
    
    int *next = (int *)odr_malloc (odr_encode(), sizeof(*next));
    *next = 0;
    
    int *toget = (int *)odr_malloc (odr_encode(), sizeof(*toget));
    *toget = 0;
    
    int *presst = (int *)odr_malloc (odr_encode(), sizeof(*presst));
    *presst = 0;
    
    Z_RecordComposition comp, *compp = 0;
    int hits = *res->resultCount;
    
    int *nulint = (int *)odr_malloc (odr_encode(), sizeof(*nulint));
    *nulint = 0;
    
    res->records = 0;
    
    comp.which = Z_RecordComp_simple;
    /* how many records does the user agent want, then? */
    if (hits <= *req->smallSetUpperBound)
    {
	*toget = hits;
	if ((comp.u.simple = req->smallSetElementSetNames))
	    compp = &comp;
    }
    else if (hits < *req->largeSetLowerBound)
    {
	*toget = *req->mediumSetPresentNumber;
	if (*toget > hits)
	    *toget = hits;
	if ((comp.u.simple = req->mediumSetElementSetNames))
	    compp = &comp;
    }
    else
	*toget = 0;
    
    if (*toget && !res->records)
    {
	res->records =
	    pack_records(req->resultSetName, 1,
			 toget, compp, next, presst,
			 req->preferredRecordSyntax);
	if (!res->records)
	    return;
	res->numberOfRecordsReturned = toget;
	res->nextResultSetPosition = next;
	res->searchStatus = sr;
	res->resultSetStatus = 0;
	res->presentStatus = presst;
    }
    else
    {
	if (hits)
	    *next = 1;
	res->numberOfRecordsReturned = nulint;
	res->nextResultSetPosition = next;
	res->searchStatus = sr;
	res->resultSetStatus = 0;
	res->presentStatus = 0;
    }
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
	send_Z_PDU(apdu_response);
        break;
    case Z_APDU_searchRequest:
        logf (LOG_LOG, "got SearchRequest");
	apdu_response = create_Z_PDU(Z_APDU_searchResponse);
	recv_Z_search (apdu_request->u.searchRequest,
		       apdu_response->u.searchResponse);
	if (!apdu_response->u.searchResponse->records)
	{
	    piggyback(apdu_request->u.searchRequest,
		      apdu_response->u.searchResponse);
	}
	send_Z_PDU(apdu_response);
        break;
    case Z_APDU_presentRequest:
        logf (LOG_LOG, "got PresentRequest");
	apdu_response = create_Z_PDU(Z_APDU_presentResponse);
	recv_Z_present (apdu_request->u.presentRequest,
			apdu_response->u.presentResponse);
	send_Z_PDU(apdu_response);
        break;
    }
}

