/* This file is part of the yazpp toolkit.
 * Copyright (C) Index Data 
 * See the file LICENSE for details.
 */

#if HAVE_CONFIG_H
#include <config.h>
#endif
#include <yaz/log.h>
#include <yazpp/z-server.h>
#include <yaz/oid_db.h>

using namespace yazpp_1;

Z_Server::Z_Server(IPDU_Observable *the_PDU_Observable)
    : Z_Assoc(the_PDU_Observable)
{
    m_facilities = 0;
}

Z_Server::~Z_Server()
{
    facility_reset();
}

void Z_Server::facility_reset ()
{
    Z_Server_Facility_Info *p = m_facilities;
    while (p)
    {
        Z_Server_Facility_Info *p_next = p->m_next;

        delete [] p->m_name;
        delete p;
        p = p_next;
    }
    m_facilities = 0;
}

void Z_Server::facility_add(IServer_Facility *facility,
                            const char *name)
{
    Z_Server_Facility_Info **p = &m_facilities;
    while (*p)
        p = &(*p)->m_next;

    *p = new Z_Server_Facility_Info;

    (*p)->m_next = 0;
    (*p)->m_name = new char [strlen(name)+1];
    strcpy ((*p)->m_name, name);
    (*p)->m_facility = facility;
}

void Z_Server::recv_GDU (Z_GDU *apdu, int len)
{
    if (apdu->which == Z_GDU_Z3950)
        recv_Z_PDU(apdu->u.z3950, len);
    else
        delete this;
}

void Z_Server::recv_Z_PDU (Z_APDU *apdu_request, int len)
{
    Z_Server_Facility_Info *f = m_facilities;

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
void Z_ServerUtility::create_databaseRecord (
    ODR odr, Z_NamePlusRecord *rec, const char *dbname, const Odr_oid *format,
    const void *buf, int len)
{
    Odr_oid *oid = odr_oiddup(odr, format);
    rec->databaseName = dbname ? odr_strdup (odr, dbname) : 0;
    rec->which = Z_NamePlusRecord_databaseRecord;
    rec->u.databaseRecord = z_ext_record_oid(odr, oid,
                                             (const char *) buf, len);
}

/*
 * surrogate diagnostic.
 */
void Z_ServerUtility::create_surrogateDiagnostics(
    ODR odr, Z_NamePlusRecord *rec, const char *dbname,
    int error, char *const addinfo)
{
    Odr_int *err = (Odr_int *)odr_malloc (odr, sizeof(*err));
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
    dr->diagnosticSetId = odr_oiddup(odr, yaz_oid_diagset_bib_1);

    dr->condition = err;
    dr->which = Z_DefaultDiagFormat_v2Addinfo;
    dr->u.v2Addinfo = odr_strdup (odr, addinfo ? addinfo : "");
}

Z_Records *Z_ServerUtility::create_nonSurrogateDiagnostics (
    ODR odr, int error, const char *addinfo)
{
    Z_Records *rec = (Z_Records *)
        odr_malloc (odr, sizeof(*rec));
    Odr_int *err = (Odr_int *)
        odr_malloc (odr, sizeof(*err));
    Z_DiagRec *drec = (Z_DiagRec *)
        odr_malloc (odr, sizeof(*drec));
    Z_DefaultDiagFormat *dr = (Z_DefaultDiagFormat *)
        odr_malloc (odr, sizeof(*dr));

    *err = error;
    rec->which = Z_Records_NSD;
    rec->u.nonSurrogateDiagnostic = dr;
    dr->diagnosticSetId = odr_oiddup(odr, yaz_oid_diagset_bib_1);

    dr->condition = err;
    dr->which = Z_DefaultDiagFormat_v2Addinfo;
    dr->u.v2Addinfo = odr_strdup (odr, addinfo ? addinfo : "");
    return rec;
}

void Z_ServerUtility::create_diagnostics (
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
    dr->diagnosticSetId = odr_oiddup(odr, yaz_oid_diagset_bib_1);
    dr->condition = odr_intdup (odr, error);
    dr->which = Z_DefaultDiagFormat_v2Addinfo;
    dr->u.v2Addinfo = odr_strdup (odr, addinfo ? addinfo : "");
}
/*
 * Local variables:
 * c-basic-offset: 4
 * c-file-style: "Stroustrup"
 * indent-tabs-mode: nil
 * End:
 * vim: shiftwidth=4 tabstop=8 expandtab
 */

