/*
 * Copyright (c) 2000-2001, Index Data.
 * See the file LICENSE for details.
 * 
 * $Log: yaz-z-server-ill.cpp,v $
 * Revision 1.3  2001-04-03 14:37:19  adam
 * More work ILL-service.
 *
 * Revision 1.2  2001/03/29 15:14:26  adam
 * Minor updates.
 *
 * Revision 1.1  2001/03/27 14:47:45  adam
 * New server facility scheme.
 *
 */

#include <yaz/log.h>
#include <yaz++/yaz-z-server.h>

/*
 * database record.
 */
void Yaz_Facility_ILL::create_databaseRecord (
    Z_NamePlusRecord *rec, const char *dbname, int format,
    const void *buf, int len)
{
    rec->databaseName = dbname ? odr_strdup (m_odr, dbname) : 0;
    rec->which = Z_NamePlusRecord_databaseRecord;
    rec->u.databaseRecord = z_ext_record (m_odr, format,
					  (const char *) buf, len);
}

/*
 * surrogate diagnostic.
 */
void Yaz_Facility_ILL::create_surrogateDiagnostics(
    Z_NamePlusRecord *rec, const char *dbname, int error, char *const addinfo)
{
    int oid[OID_SIZE];
    int *err = (int *)odr_malloc (m_odr, sizeof(*err));
    oident bib1;
    Z_DiagRec *drec = (Z_DiagRec *)odr_malloc (m_odr, sizeof(*drec));
    Z_DefaultDiagFormat *dr = (Z_DefaultDiagFormat *)
	odr_malloc (m_odr, sizeof(*dr));
    
    bib1.proto = PROTO_Z3950;
    bib1.oclass = CLASS_DIAGSET;
    bib1.value = VAL_BIB1;

    yaz_log(LOG_DEBUG, "SurrogateDiagnotic: %d -- %s", error, addinfo);
    *err = error;
    rec->databaseName = dbname ? odr_strdup (m_odr, dbname) : 0;
    rec->which = Z_NamePlusRecord_surrogateDiagnostic;
    rec->u.surrogateDiagnostic = drec;
    drec->which = Z_DiagRec_defaultFormat;
    drec->u.defaultFormat = dr;
    dr->diagnosticSetId = odr_oiddup (m_odr,
                                      oid_ent_to_oid(&bib1, oid));
    dr->condition = err;
    dr->which = Z_DefaultDiagFormat_v2Addinfo;
    dr->u.v2Addinfo = odr_strdup (m_odr, addinfo ? addinfo : "");
}

ODR Yaz_Facility_ILL::odr_encode()
{
    return m_odr;
}

int Yaz_Facility_ILL::init(Yaz_Z_Server *s, Z_InitRequest *initRequest,
			   Z_InitResponse *initResponse)
{
    Z_Options *req = initRequest->options;
    Z_Options *res = initResponse->options;
    
    if (ODR_MASK_GET(req, Z_Options_extendedServices))
	ODR_MASK_SET(res, Z_Options_extendedServices);
    return ill_init (initRequest, initResponse);
}

int Yaz_Facility_ILL::recv(Yaz_Z_Server *s, Z_APDU *apdu_request)
{   
    Z_APDU *apdu_response;

    m_odr = s->odr_encode();
    if (apdu_request->which != Z_APDU_extendedServicesRequest)
	return 0;
    Z_ExtendedServicesRequest *req = apdu_request->u.extendedServicesRequest;
    if (!req->taskSpecificParameters || req->taskSpecificParameters->which !=
        Z_External_itemOrder)
        return 0;
    yaz_log (LOG_LOG, "got ill p=%p", this);
    apdu_response = s->create_Z_PDU(Z_APDU_extendedServicesResponse);
    ill_service(req, req->taskSpecificParameters->u.itemOrder,
        apdu_response->u.extendedServicesResponse);
    s->send_Z_PDU(apdu_response);
    return 1;
}
