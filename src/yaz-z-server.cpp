/*
 * Copyright (c) 2000-2003, Index Data.
 * See the file LICENSE for details.
 * 
 * $Id: yaz-z-server.cpp,v 1.19 2004-11-30 21:10:31 adam Exp $
 */

#include <yaz/ylog.h>
#include <yaz++/z-server.h>

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

void Yaz_Z_Server::recv_GDU (Z_GDU *apdu, int len)
{
    if (apdu->which == Z_GDU_Z3950)
	recv_Z_PDU(apdu->u.z3950, len);
    else
	delete this;
}

void Yaz_Z_Server::recv_Z_PDU (Z_APDU *apdu_request, int len)
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
	send_Z_PDU(apdu_response, 0);
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
	    yaz_log (YLOG_WARN, "unhandled request = %d", apdu_request->which);
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
    int *err = (int *)odr_malloc (odr, sizeof(*err));
    Z_DiagRec *drec = (Z_DiagRec *)odr_malloc (odr, sizeof(*drec));
    Z_DefaultDiagFormat *dr = (Z_DefaultDiagFormat *)
    	odr_malloc (odr, sizeof(*dr));
    
    yaz_log(YLOG_DEBUG, "SurrogateDiagnotic: %d -- %s", error, addinfo);
    *err = error;
    rec->databaseName = dbname ? odr_strdup (odr, dbname) : 0;
    rec->which = Z_NamePlusRecord_surrogateDiagnostic;
    rec->u.surrogateDiagnostic = drec;
    drec->which = Z_DiagRec_defaultFormat;
    drec->u.defaultFormat = dr;
    dr->diagnosticSetId =
        yaz_oidval_to_z3950oid (odr, CLASS_DIAGSET, VAL_BIB1);

    dr->condition = err;
    dr->which = Z_DefaultDiagFormat_v2Addinfo;
    dr->u.v2Addinfo = odr_strdup (odr, addinfo ? addinfo : "");
}

Z_Records *Yaz_Z_ServerUtility::create_nonSurrogateDiagnostics (
    ODR odr, int error, const char *addinfo)
{
    Z_Records *rec = (Z_Records *)
        odr_malloc (odr, sizeof(*rec));
    int *err = (int *)
        odr_malloc (odr, sizeof(*err));
    Z_DiagRec *drec = (Z_DiagRec *)
        odr_malloc (odr, sizeof(*drec));
    Z_DefaultDiagFormat *dr = (Z_DefaultDiagFormat *)
        odr_malloc (odr, sizeof(*dr));

    *err = error;
    rec->which = Z_Records_NSD;
    rec->u.nonSurrogateDiagnostic = dr;
    dr->diagnosticSetId =
        yaz_oidval_to_z3950oid (odr, CLASS_DIAGSET, VAL_BIB1);

    dr->condition = err;
    dr->which = Z_DefaultDiagFormat_v2Addinfo;
    dr->u.v2Addinfo = odr_strdup (odr, addinfo ? addinfo : "");
    return rec;
}

void Yaz_Z_ServerUtility::create_diagnostics (
    ODR odr, int error, const char *addinfo,
    Z_DiagRec ***dreca, int *num)
{
    Z_DiagRec *drec = (Z_DiagRec *) odr_malloc (odr, sizeof(*drec));
    Z_DefaultDiagFormat *dr = (Z_DefaultDiagFormat *)
        odr_malloc (odr, sizeof(*dr));
    
    *num = 1;
    *dreca = (Z_DiagRec **) odr_malloc (odr, sizeof(*dreca));
    (*dreca)[0] = drec;
        
    drec->which = Z_DiagRec_defaultFormat;
    drec->u.defaultFormat = dr;
    dr->diagnosticSetId =
        yaz_oidval_to_z3950oid (odr, CLASS_DIAGSET, VAL_BIB1);
    dr->condition = odr_intdup (odr, error);
    dr->which = Z_DefaultDiagFormat_v2Addinfo;
    dr->u.v2Addinfo = odr_strdup (odr, addinfo ? addinfo : "");
}
