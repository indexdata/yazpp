/*
 * Copyright (c) 2000-2001, Index Data.
 * See the file LICENSE for details.
 * 
 * $Id: yaz-z-server.cpp,v 1.13 2001-11-04 22:36:21 adam Exp $
 */

#include <yaz/log.h>
#include <yaz++/yaz-z-server.h>

Yaz_Z_Server::Yaz_Z_Server(IYaz_PDU_Observable *the_PDU_Observable)
    : Yaz_Z_Assoc(the_PDU_Observable)
{
    m_facilities = 0;
}

Yaz_Z_Server::~Yaz_Z_Server()
{
    facility_reset();
}

void Yaz_Z_Server::facility_reset ()
{
    Yaz_Z_Server_Facility_Info *p = m_facilities;
    while (p)
    {
	Yaz_Z_Server_Facility_Info *p_next = p->m_next;

	delete [] p->m_name;
	delete p;
	p = p_next;
    }
    m_facilities = 0;
}

void Yaz_Z_Server::facility_add(IYaz_Server_Facility *facility,
				const char *name)
{
    Yaz_Z_Server_Facility_Info **p = &m_facilities;
    while (*p)
	p = &(*p)->m_next;

    *p = new Yaz_Z_Server_Facility_Info;

    (*p)->m_next = 0;
    (*p)->m_name = new char [strlen(name)+1];
    strcpy ((*p)->m_name, name);
    (*p)->m_facility = facility;
}

void Yaz_Z_Server::recv_Z_PDU (Z_APDU *apdu_request)
{   
    Yaz_Z_Server_Facility_Info *f = m_facilities;
    
    if (apdu_request->which == Z_APDU_initRequest)
    {
	Z_APDU *apdu_response = create_Z_PDU(Z_APDU_initResponse);

	Z_InitRequest *req = apdu_request->u.initRequest;
	Z_InitResponse *resp = apdu_response->u.initResponse;
	
	if (ODR_MASK_GET(req->protocolVersion, Z_ProtocolVersion_1))
	{
	    ODR_MASK_SET(resp->protocolVersion, Z_ProtocolVersion_1);
	}
	if (ODR_MASK_GET(req->protocolVersion, Z_ProtocolVersion_2))
	{
	    ODR_MASK_SET(resp->protocolVersion, Z_ProtocolVersion_2);
	}
	if (ODR_MASK_GET(req->protocolVersion, Z_ProtocolVersion_3))
	{
	    ODR_MASK_SET(resp->protocolVersion, Z_ProtocolVersion_3);
	}
	while (f)
	{
	    f->m_facility->init(this, req, resp);
	    f = f->m_next;
	}
	transfer_referenceId(apdu_request, apdu_response);
	send_Z_PDU(apdu_response);
    }
    else
    {
	f = m_facilities;
	int taken = 0;
	while (f)
	{
	    taken = f->m_facility->recv(this, apdu_request);
	    if (taken)
		break;
	    f = f->m_next;
	}
	if (!taken)
	{
	    yaz_log (LOG_WARN, "unhandled request = %d", apdu_request->which);
	    delete this;
	}
    }
}

/*
 * database record.
 */
void Yaz_Z_ServerUtility::create_databaseRecord (
    ODR odr, Z_NamePlusRecord *rec, const char *dbname, int format,
    const void *buf, int len)
{
    rec->databaseName = dbname ? odr_strdup (odr, dbname) : 0;
    rec->which = Z_NamePlusRecord_databaseRecord;
    rec->u.databaseRecord = z_ext_record (odr, format,
					  (const char *) buf, len);
}

/*
 * surrogate diagnostic.
 */
void Yaz_Z_ServerUtility::create_surrogateDiagnostics(
    ODR odr, Z_NamePlusRecord *rec, const char *dbname,
    int error, char *const addinfo)
{
    int oid[OID_SIZE];
    int *err = (int *)odr_malloc (odr, sizeof(*err));
    oident bib1;
    Z_DiagRec *drec = (Z_DiagRec *)odr_malloc (odr, sizeof(*drec));
    Z_DefaultDiagFormat *dr = (Z_DefaultDiagFormat *)
    	odr_malloc (odr, sizeof(*dr));
    
    bib1.proto = PROTO_Z3950;
    bib1.oclass = CLASS_DIAGSET;
    bib1.value = VAL_BIB1;

    yaz_log(LOG_DEBUG, "SurrogateDiagnotic: %d -- %s", error, addinfo);
    *err = error;
    rec->databaseName = dbname ? odr_strdup (odr, dbname) : 0;
    rec->which = Z_NamePlusRecord_surrogateDiagnostic;
    rec->u.surrogateDiagnostic = drec;
    drec->which = Z_DiagRec_defaultFormat;
    drec->u.defaultFormat = dr;
    dr->diagnosticSetId = odr_oiddup (odr,
                                      oid_ent_to_oid(&bib1, oid));
    dr->condition = err;
    dr->which = Z_DefaultDiagFormat_v2Addinfo;
    dr->u.v2Addinfo = odr_strdup (odr, addinfo ? addinfo : "");
}

Z_Records *Yaz_Z_ServerUtility::create_nonSurrogateDiagnostics (
    ODR odr, int error, const char *addinfo)
{
    int oid[OID_SIZE];
    Z_Records *rec = (Z_Records *)
        odr_malloc (odr, sizeof(*rec));
    oident bib1;
    int *err = (int *)
        odr_malloc (odr, sizeof(*err));
    Z_DiagRec *drec = (Z_DiagRec *)
        odr_malloc (odr, sizeof(*drec));
    Z_DefaultDiagFormat *dr = (Z_DefaultDiagFormat *)
        odr_malloc (odr, sizeof(*dr));

    bib1.proto = PROTO_Z3950;
    bib1.oclass = CLASS_DIAGSET;
    bib1.value = VAL_BIB1;

    *err = error;
    rec->which = Z_Records_NSD;
    rec->u.nonSurrogateDiagnostic = dr;
    dr->diagnosticSetId =
        odr_oiddup (odr, oid_ent_to_oid(&bib1, oid));
    dr->condition = err;
    dr->which = Z_DefaultDiagFormat_v2Addinfo;
    dr->u.v2Addinfo = odr_strdup (odr, addinfo ? addinfo : "");
    return rec;
}
