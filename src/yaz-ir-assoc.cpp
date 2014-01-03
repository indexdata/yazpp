/* This file is part of the yazpp toolkit.
 * Copyright (C) Index Data 
 * See the file LICENSE for details.
 */

#if HAVE_CONFIG_H
#include <config.h>
#endif
#include <assert.h>

#include <yaz/log.h>
#include <yazpp/ir-assoc.h>
#include <yaz/oid_db.h>

using namespace yazpp_1;

IR_Assoc::IR_Assoc(IPDU_Observable *the_PDU_Observable)
    : Z_Assoc(the_PDU_Observable)
{
    m_num_databaseNames = 0;
    m_databaseNames = 0;
    m_preferredRecordSyntax = 0;
    m_elementSetNames = 0;
    m_lastReceived = 0;
    m_host = 0;
    m_proxy = 0;
    m_cookie = 0;
    m_log = YLOG_DEBUG;
    const char *db = "Default";
    set_databaseNames(1, &db);
}

IR_Assoc::~IR_Assoc()
{
    xfree(m_preferredRecordSyntax);
    if (m_elementSetNames)
        delete [] m_elementSetNames->u.generic;
    delete [] m_elementSetNames;
    delete [] m_host;
    delete [] m_proxy;
    delete [] m_cookie;
}

void IR_Assoc::get_databaseNames (int *num, char ***list)
{
    *num = m_num_databaseNames;
    *list = m_databaseNames;
}

typedef char *charp;
void IR_Assoc::set_databaseNames (int num, const char **list)
{
    int i;
    yaz_log (m_log, "IR_Assoc::set_databaseNames num=%d", num);
    for (i = 0; i<m_num_databaseNames; i++)
        delete [] m_databaseNames[i];
    delete [] m_databaseNames;
    m_num_databaseNames = num;

    m_databaseNames = new char *[num];
    for (i = 0; i<m_num_databaseNames; i++)
    {
        m_databaseNames[i] = new char[strlen(list[i])+1];
        strcpy(m_databaseNames[i], list[i]);
    }
}

void IR_Assoc::set_databaseNames(const char *dblist, const char *sep)
{
    const char **list = new const char* [strlen(dblist)];
    char *dbtmp = new char[strlen(dblist)+1];
    strcpy(dbtmp, dblist);
    int num = 0;
    int len = 0;
    for (char *cp = dbtmp; ; cp++)
        if (*cp && !strchr(sep, *cp))
            len++;
        else
        {
            if (len)
            {
                list[num] = cp - len;
                num++;
            }
            if (!*cp)
                break;
            *cp = '\0';
            len = 0;
        }
    set_databaseNames (num, list);
    delete [] dbtmp;
    delete [] list;
}

void IR_Assoc::set_preferredRecordSyntax (const char *syntax)
{
    xfree(m_preferredRecordSyntax);
    m_preferredRecordSyntax = 0;
    if (syntax && *syntax)
        m_preferredRecordSyntax = xstrdup(syntax);
}

void IR_Assoc::get_preferredRecordSyntax (const char **dst)
{
    if (m_preferredRecordSyntax)
        *dst = m_preferredRecordSyntax;
    else
        *dst = "";
}

void IR_Assoc::set_elementSetName (const char *elementSetName)
{
    if (m_elementSetNames)
        delete [] m_elementSetNames->u.generic;
    delete m_elementSetNames;
    m_elementSetNames = 0;
    if (elementSetName && *elementSetName)
    {
        m_elementSetNames = new Z_ElementSetNames;
        m_elementSetNames->which = Z_ElementSetNames_generic;
        m_elementSetNames->u.generic = new char[strlen(elementSetName)+1];
        strcpy (m_elementSetNames->u.generic, elementSetName);
    }
}

void IR_Assoc::get_elementSetName (Z_ElementSetNames **elementSetNames)
{
    *elementSetNames = m_elementSetNames;
}

void IR_Assoc::get_elementSetName (const char **elementSetName)
{
    if (!m_elementSetNames ||
        m_elementSetNames->which != Z_ElementSetNames_generic)
    {
        *elementSetName = 0;
        return;
    }
    *elementSetName = m_elementSetNames->u.generic;
}


void IR_Assoc::recv_GDU(Z_GDU *apdu, int len)
{
    if (apdu->which == Z_GDU_Z3950)
            recv_Z_PDU(apdu->u.z3950, len);
}

void IR_Assoc::recv_Z_PDU(Z_APDU *apdu, int len)
{
    yaz_log (m_log, "recv_Z_PDU %d bytes", len);
    m_lastReceived = apdu->which;
    switch (apdu->which)
    {
    case Z_APDU_initResponse:
        yaz_log (m_log, "recv InitResponse");
        recv_initResponse(apdu->u.initResponse);
        break;
    case Z_APDU_initRequest:
        yaz_log (m_log, "recv InitRequest");
        recv_initRequest(apdu->u.initRequest);
        break;
    case Z_APDU_searchRequest:
        yaz_log (m_log, "recv searchRequest");
        recv_searchRequest(apdu->u.searchRequest);
        break;
    case Z_APDU_searchResponse:
        yaz_log (m_log, "recv searchResponse");
        recv_searchResponse(apdu->u.searchResponse);
        break;
    case Z_APDU_presentRequest:
        yaz_log (m_log, "recv presentRequest");
        recv_presentRequest(apdu->u.presentRequest);
        break;
    case Z_APDU_presentResponse:
        yaz_log (m_log, "recv presentResponse");
        recv_presentResponse(apdu->u.presentResponse);
        break;
    case Z_APDU_extendedServicesResponse:
        yaz_log (m_log, "recv extendedServiceResponse");
        recv_extendedServicesResponse(apdu->u.extendedServicesResponse);
        break;
    }
}

int IR_Assoc::send_searchRequest(Yaz_Z_Query *query,
                                     char* pResultSetId,
                                     char* pRefId)
{
    Z_APDU *apdu = create_Z_PDU(Z_APDU_searchRequest);
    Z_SearchRequest *req = apdu->u.searchRequest;

    req->query = query->get_Z_Query();
    if (!req->query)
        return -1;
    get_databaseNames (&req->num_databaseNames, &req->databaseNames);
    const char *recordSyntax;
    get_preferredRecordSyntax(&recordSyntax);
    if (recordSyntax && *recordSyntax)
    {
        req->preferredRecordSyntax
            = yaz_string_to_oid_odr(yaz_oid_std(), CLASS_RECSYN, recordSyntax,
                                    odr_encode());
    }
    yaz_log (m_log, "send_searchRequest");
    assert (req->otherInfo == 0);
    if (m_cookie)
    {
        set_otherInformationString(&req->otherInfo, yaz_oid_userinfo_cookie,
                                   1, m_cookie);
        assert (req->otherInfo);
    }

    if ( pRefId )
    {
        req->referenceId = getRefID(pRefId);
    }

    if ( pResultSetId )
    {
        req->resultSetName = pResultSetId;
    }

    return send_Z_PDU(apdu, 0);
}

int IR_Assoc::send_presentRequest(Odr_int start,
                                  Odr_int number,
                                  char* pResultSetId,
                                  char* pRefId)
{
    Z_APDU *apdu = create_Z_PDU(Z_APDU_presentRequest);
    Z_PresentRequest *req = apdu->u.presentRequest;

    req->resultSetStartPoint = &start;
    req->numberOfRecordsRequested = &number;

    const char *recordSyntax;
    get_preferredRecordSyntax (&recordSyntax);
    if (recordSyntax && *recordSyntax)
    {
        req->preferredRecordSyntax =
            yaz_string_to_oid_odr(yaz_oid_std(), CLASS_RECSYN, recordSyntax,
                                  odr_encode());
    }
    Z_RecordComposition compo;
    Z_ElementSetNames *elementSetNames;
    get_elementSetName (&elementSetNames);
    if (elementSetNames)
    {
        req->recordComposition = &compo;
        compo.which = Z_RecordComp_simple;
        compo.u.simple = elementSetNames;
    }

    if (m_cookie)
        set_otherInformationString(&req->otherInfo, yaz_oid_userinfo_cookie,
                                   1, m_cookie);

    if ( pRefId )
    {
        req->referenceId = getRefID(pRefId);
    }

    if ( pResultSetId )
    {
        req->resultSetId = pResultSetId;
    }

    return send_Z_PDU(apdu, 0);
}

void IR_Assoc::set_proxy(const char *str)
{
    delete [] m_proxy;
    m_proxy = 0;
    if (str)
    {
        m_proxy = new char[strlen(str)+1];
        strcpy (m_proxy, str);
    }
}

void IR_Assoc::set_cookie(const char *str)
{
    delete [] m_cookie;
    m_cookie = 0;
    if (str)
    {
        m_cookie = new char[strlen(str)+1];
        strcpy(m_cookie, str);
    }
}

const char *IR_Assoc::get_cookie()
{
    return m_cookie;
}

void IR_Assoc::client(const char *addr)
{
    delete [] m_host;
    m_host = new char[strlen(addr)+1];
    strcpy(m_host, addr);
    const char *dbpart = strchr(m_host, '/');
    if (dbpart)
        set_databaseNames (dbpart+1, "+ ");
    Z_Assoc::client(m_proxy ? m_proxy : m_host);
}

const char *IR_Assoc::get_proxy()
{
    return m_proxy;
}

const char *IR_Assoc::get_host()
{
    return m_host;
}

void IR_Assoc::recv_searchRequest(Z_SearchRequest *searchRequest)
{
    Z_APDU *apdu = create_Z_PDU(Z_APDU_searchResponse);
    send_Z_PDU(apdu, 0);
}

void IR_Assoc::recv_presentRequest(Z_PresentRequest *presentRequest)
{
    Z_APDU *apdu = create_Z_PDU(Z_APDU_presentResponse);
    send_Z_PDU(apdu, 0);
}

void IR_Assoc::recv_initRequest(Z_InitRequest *initRequest)
{
    Z_APDU *apdu = create_Z_PDU(Z_APDU_initResponse);
    send_Z_PDU(apdu, 0);
}

void IR_Assoc::recv_searchResponse (Z_SearchResponse *searchResponse)
{
}

void IR_Assoc::recv_presentResponse (Z_PresentResponse *presentResponse)
{
}

void IR_Assoc::recv_initResponse(Z_InitResponse *initResponse)
{
}

void IR_Assoc::recv_extendedServicesResponse(Z_ExtendedServicesResponse *ExtendedServicesResponse)
{
}

int IR_Assoc::get_lastReceived()
{
    return m_lastReceived;
}

void IR_Assoc::set_lastReceived(int lastReceived)
{
    m_lastReceived = lastReceived;
}

int IR_Assoc::send_initRequest(char* pRefId)
{
    Z_APDU *apdu = create_Z_PDU(Z_APDU_initRequest);
    Z_InitRequest *req = apdu->u.initRequest;

    ODR_MASK_SET(req->options, Z_Options_search);
    ODR_MASK_SET(req->options, Z_Options_present);
    ODR_MASK_SET(req->options, Z_Options_namedResultSets);
    ODR_MASK_SET(req->options, Z_Options_triggerResourceCtrl);
    ODR_MASK_SET(req->options, Z_Options_scan);
    ODR_MASK_SET(req->options, Z_Options_sort);
    ODR_MASK_SET(req->options, Z_Options_extendedServices);
    ODR_MASK_SET(req->options, Z_Options_delSet);

    ODR_MASK_SET(req->protocolVersion, Z_ProtocolVersion_1);
    ODR_MASK_SET(req->protocolVersion, Z_ProtocolVersion_2);
    ODR_MASK_SET(req->protocolVersion, Z_ProtocolVersion_3);

    if ( pRefId )
    {
        req->referenceId = getRefID(pRefId);
    }

    if (m_proxy && m_host)
        set_otherInformationString(&req->otherInfo, yaz_oid_userinfo_proxy,
                                   1, m_host);
    if (m_cookie)
        set_otherInformationString(&req->otherInfo, yaz_oid_userinfo_cookie,
                                   1, m_cookie);
    return send_Z_PDU(apdu, 0);
}

int IR_Assoc::send_deleteResultSetRequest(char* pResultSetId, char* pRefId)
{
    char* ResultSetIds[1];

    Z_APDU *apdu = create_Z_PDU(Z_APDU_deleteResultSetRequest);
    Z_DeleteResultSetRequest *req = apdu->u.deleteResultSetRequest;

    if ( pResultSetId )
    {
        *req->deleteFunction = Z_DeleteResultSetRequest_list;
        req->num_resultSetList = 1;
        ResultSetIds[0] = pResultSetId;
        req->resultSetList = ResultSetIds;
    }
    else
    {
        *req->deleteFunction = Z_DeleteResultSetRequest_all;
    }

    if ( pRefId )
    {
        req->referenceId = getRefID(pRefId);
    }

    if (m_proxy && m_host)
        set_otherInformationString(&req->otherInfo, yaz_oid_userinfo_proxy,
                                   1, m_host);
    if (m_cookie)
        set_otherInformationString(&req->otherInfo, yaz_oid_userinfo_cookie,
                                   1, m_cookie);

    return send_Z_PDU(apdu, 0);
}


/*
 * Local variables:
 * c-basic-offset: 4
 * c-file-style: "Stroustrup"
 * indent-tabs-mode: nil
 * End:
 * vim: shiftwidth=4 tabstop=8 expandtab
 */

