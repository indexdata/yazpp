/*
 * Copyright (c) 1998-1999, Index Data.
 * See the file LICENSE for details.
 * Sebastian Hammer, Adam Dickmeiss
 * 
 * $Log: yaz-ir-assoc.cpp,v $
 * Revision 1.7  1999-04-21 12:09:01  adam
 * Many improvements. Modified to proxy server to work with "sessions"
 * based on cookies.
 *
 * Revision 1.6  1999/04/20 10:30:05  adam
 * Implemented various stuff for client and proxy. Updated calls
 * to ODR to reflect new name parameter.
 *
 * Revision 1.5  1999/04/09 11:46:57  adam
 * Added object Yaz_Z_Assoc. Much more functional client.
 */

#include <assert.h>

#include <log.h>
#include <yaz-ir-assoc.h>

Yaz_IR_Assoc::Yaz_IR_Assoc(IYaz_PDU_Observable *the_PDU_Observable)
    : Yaz_Z_Assoc(the_PDU_Observable)
{
    m_num_databaseNames = 0;
    m_databaseNames = 0;
    m_preferredRecordSyntax = VAL_NONE;
    m_elementSetNames = 0;
    m_lastReceived = 0;
    m_host = 0;
    m_proxy = 0;
    m_cookie = 0;
    const char *db = "Default";
    set_databaseNames(1, &db);
}

Yaz_IR_Assoc::~Yaz_IR_Assoc()
{
    if (m_elementSetNames)
	delete [] m_elementSetNames->u.generic;
    delete [] m_elementSetNames;
    delete [] m_host;
    delete [] m_proxy;
    delete [] m_cookie;
}

void Yaz_IR_Assoc::get_databaseNames (int *num, char ***list)
{
    *num = m_num_databaseNames;
    *list = m_databaseNames;
}

void Yaz_IR_Assoc::set_databaseNames (int num, const char **list)
{
    int i;
    logf (LOG_LOG, "Yaz_IR_Assoc::set_databaseNames num=%d", num);
    for (i = 0; i<m_num_databaseNames; i++)
	delete [] m_databaseNames[i];
    delete [] m_databaseNames;
    m_num_databaseNames = num;
    m_databaseNames = new (char*) [num];
    for (i = 0; i<m_num_databaseNames; i++)
    {
	m_databaseNames[i] = new char[strlen(list[i])+1];
	strcpy(m_databaseNames[i], list[i]);
    }
}

void Yaz_IR_Assoc::set_databaseNames(const char *dblist, const char *sep)
{
    const char **list = new (const char*) [strlen(dblist)];
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

void Yaz_IR_Assoc::set_preferredRecordSyntax (int value)
{
    m_preferredRecordSyntax = value;
}

void Yaz_IR_Assoc::set_preferredRecordSyntax (const char *syntax)
{
    m_preferredRecordSyntax = VAL_NONE;
    if (syntax && *syntax)
	m_preferredRecordSyntax = oid_getvalbyname (syntax);
}

void Yaz_IR_Assoc::get_preferredRecordSyntax (int *value)
{
    *value = m_preferredRecordSyntax;
}

void Yaz_IR_Assoc::get_preferredRecordSyntax (const char **dst)
{
    struct oident ent;
    ent.proto = PROTO_Z3950;
    ent.oclass = CLASS_RECSYN;
    ent.value = (enum oid_value) m_preferredRecordSyntax;

    int oid[OID_SIZE];
    oid_ent_to_oid (&ent, oid);
    struct oident *entp = oid_getentbyoid (oid);
    
    *dst = entp ? entp->desc : "";
}

void Yaz_IR_Assoc::set_elementSetName (const char *elementSetName)
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

void Yaz_IR_Assoc::get_elementSetName (Z_ElementSetNames **elementSetNames)
{
    *elementSetNames = m_elementSetNames;
}

void Yaz_IR_Assoc::get_elementSetName (const char **elementSetName)
{
    if (!m_elementSetNames ||
	m_elementSetNames->which != Z_ElementSetNames_generic)
    {
	*elementSetName = 0;
	return;
    }
    *elementSetName = m_elementSetNames->u.generic;
}

void Yaz_IR_Assoc::recv_Z_PDU(Z_APDU *apdu)
{
    logf (LOG_LOG, "recv_Z_PDU");
    m_lastReceived = apdu->which;
    switch (apdu->which)
    {
    case Z_APDU_initResponse:
	logf (LOG_LOG, "recv InitResponse");
	recv_initResponse(apdu->u.initResponse);
	break;
    case Z_APDU_initRequest:
        logf (LOG_LOG, "recv InitRequest");
	recv_initRequest(apdu->u.initRequest);
        break;
    case Z_APDU_searchRequest:
        logf (LOG_LOG, "recv searchRequest");
	recv_searchRequest(apdu->u.searchRequest);
        break;
    case Z_APDU_searchResponse:
	logf (LOG_LOG, "recv searchResponse"); 
	recv_searchResponse(apdu->u.searchResponse);
	break;
    case Z_APDU_presentRequest:
        logf (LOG_LOG, "recv presentRequest");
	recv_presentRequest(apdu->u.presentRequest);
        break;
    case Z_APDU_presentResponse:
        logf (LOG_LOG, "recv presentResponse");
	recv_presentResponse(apdu->u.presentResponse);
        break;
    }
}

int Yaz_IR_Assoc::send_searchRequest(Yaz_Z_Query *query)
{
    Z_APDU *apdu = create_Z_PDU(Z_APDU_searchRequest);
    Z_SearchRequest *req = apdu->u.searchRequest;
    int recordSyntax;

    req->query = query->get_Z_Query();
    if (!req->query)
	return -1;
    get_databaseNames (&req->num_databaseNames, &req->databaseNames);
    int oid_syntax[OID_SIZE];
    oident prefsyn;
    get_preferredRecordSyntax(&recordSyntax);
    if (recordSyntax != VAL_NONE)
    {
	prefsyn.proto = PROTO_Z3950;
	prefsyn.oclass = CLASS_RECSYN;
	prefsyn.value = (enum oid_value) recordSyntax;
	oid_ent_to_oid(&prefsyn, oid_syntax);
	req->preferredRecordSyntax = oid_syntax;
    }
    logf (LOG_LOG, "send_searchRequest");
    assert (req->otherInfo == 0);
    if (m_cookie)
    {
	set_otherInformationString(&req->otherInfo, VAL_COOKIE, 1, m_cookie);
	assert (req->otherInfo);
    }
    return send_Z_PDU(apdu);
}

int Yaz_IR_Assoc::send_presentRequest(int start, int number)
{
    Z_APDU *apdu = create_Z_PDU(Z_APDU_presentRequest);
    Z_PresentRequest *req = apdu->u.presentRequest;

    req->resultSetStartPoint = &start;
    req->numberOfRecordsRequested = &number;

    int oid_syntax[OID_SIZE];
    oident prefsyn;
    int recordSyntax;
    get_preferredRecordSyntax (&recordSyntax);
    if (recordSyntax != VAL_NONE)
    {
	prefsyn.proto = PROTO_Z3950;
	prefsyn.oclass = CLASS_RECSYN;
	prefsyn.value = (enum oid_value) recordSyntax;
	oid_ent_to_oid(&prefsyn, oid_syntax);
	req->preferredRecordSyntax = oid_syntax;
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
	set_otherInformationString(&req->otherInfo, VAL_COOKIE, 1, m_cookie);
    return send_Z_PDU(apdu);
}

void Yaz_IR_Assoc::set_proxy(const char *str)
{
    delete [] m_proxy;
    m_proxy = 0;
    if (str)
    {
	m_proxy = new char[strlen(str)+1];
	strcpy (m_proxy, str);
    }
}

void Yaz_IR_Assoc::set_cookie(const char *str)
{
    delete [] m_cookie;
    m_cookie = 0;
    if (str)
    {
	m_cookie = new char[strlen(str)+1];
	strcpy(m_cookie, str);
    }
}

const char *Yaz_IR_Assoc::get_cookie()
{
    return m_cookie;
}

void Yaz_IR_Assoc::client(const char *addr)
{
    delete [] m_host;
    m_host = new char[strlen(addr)+1];
    strcpy(m_host, addr);
    const char *zurl_p = (m_proxy ? m_proxy : m_host);
    char *zurl = new char[strlen(zurl_p)+1];
    strcpy(zurl, zurl_p);
    char *dbpart = strchr(zurl, '/');
    if (dbpart)
    {
	set_databaseNames (dbpart+1, "+ ");
	*dbpart = '\0';
    }
    Yaz_Z_Assoc::client(zurl);
    delete [] zurl;
}

const char *Yaz_IR_Assoc::get_proxy()
{
    return m_proxy;
}

const char *Yaz_IR_Assoc::get_host()
{
    return m_host;
}

void Yaz_IR_Assoc::recv_searchRequest(Z_SearchRequest *searchRequest)
{
    Z_APDU *apdu = create_Z_PDU(Z_APDU_searchResponse);
    send_Z_PDU(apdu);
}

void Yaz_IR_Assoc::recv_presentRequest(Z_PresentRequest *presentRequest)
{
    Z_APDU *apdu = create_Z_PDU(Z_APDU_presentResponse);
    send_Z_PDU(apdu);
}

void Yaz_IR_Assoc::recv_initRequest(Z_InitRequest *initRequest)
{
    Z_APDU *apdu = create_Z_PDU(Z_APDU_initResponse);
    send_Z_PDU(apdu);
}

void Yaz_IR_Assoc::recv_searchResponse (Z_SearchResponse *searchResponse)
{
}

void Yaz_IR_Assoc::recv_presentResponse (Z_PresentResponse *presentResponse)
{
}

void Yaz_IR_Assoc::recv_initResponse(Z_InitResponse *initResponse)
{
}

int Yaz_IR_Assoc::get_lastReceived()
{
    return m_lastReceived;
}

void Yaz_IR_Assoc::set_lastReceived(int lastReceived)
{
    m_lastReceived = lastReceived;
}

int Yaz_IR_Assoc::send_initRequest()
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

    if (m_proxy && m_host)
	set_otherInformationString(&req->otherInfo, VAL_PROXY, 1, m_host);
    if (m_cookie)
	set_otherInformationString(&req->otherInfo, VAL_COOKIE, 1, m_cookie);
    return send_Z_PDU(apdu);
}

