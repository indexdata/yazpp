/*
 * Copyright (c) 1998-1999, Index Data.
 * See the file LICENSE for details.
 * Sebastian Hammer, Adam Dickmeiss
 * 
 * $Log: yaz-proxy.cpp,v $
 * Revision 1.9  1999-09-13 12:53:44  adam
 * Proxy removes OtherInfo Proxy Address and Session ID. Other
 * Otherinfo remains untouched.
 *
 * Revision 1.8  1999/05/04 10:53:00  adam
 * Changed the way the PROXY behaves when lost cookie is received.
 *
 * Revision 1.7  1999/04/28 13:31:17  adam
 * Better result set optimisation for proxy.
 *
 * Revision 1.6  1999/04/27 07:52:13  adam
 * Improved proxy; added query match for result set re-use.
 *
 * Revision 1.5  1999/04/21 12:09:01  adam
 * Many improvements. Modified to proxy server to work with "sessions"
 * based on cookies.
 *
 * Revision 1.4  1999/04/20 10:30:05  adam
 * Implemented various stuff for client and proxy. Updated calls
 * to ODR to reflect new name parameter.
 *
 * Revision 1.3  1999/04/09 11:46:57  adam
 * Added object Yaz_Z_Assoc. Much more functional client.
 *
 * Revision 1.2  1999/01/28 13:08:46  adam
 * Yaz_PDU_Assoc better encapsulated. Memory leak fix in
 * yaz-socket-manager.cc.
 *
 * Revision 1.1.1.1  1999/01/28 09:41:07  adam
 * First implementation of YAZ++.
 *
 */

#include <assert.h>

#include <log.h>

#include <yaz-proxy.h>

Yaz_Proxy::Yaz_Proxy(IYaz_PDU_Observable *the_PDU_Observable) :
    Yaz_Z_Assoc(the_PDU_Observable)
{
    m_PDU_Observable = the_PDU_Observable;
    m_client = 0;
    m_parent = 0;
    m_clientPool = 0;
    m_seqno = 1;
    m_keepalive = 1;
}

Yaz_Proxy::~Yaz_Proxy()
{
}

IYaz_PDU_Observer *Yaz_Proxy::clone(IYaz_PDU_Observable
				    *the_PDU_Observable)
{
    Yaz_Proxy *new_proxy = new Yaz_Proxy(the_PDU_Observable);
    new_proxy->m_parent = this;
    new_proxy->timeout(120);
    return new_proxy;
}

char *Yaz_Proxy::get_cookie(Z_OtherInformation **otherInfo)
{
    int oid[OID_SIZE];
    Z_OtherInformationUnit *oi;
    struct oident ent;
    ent.proto = PROTO_Z3950;
    ent.oclass = CLASS_USERINFO;
    ent.value = (oid_value) VAL_COOKIE;
    assert (oid_ent_to_oid (&ent, oid));

    if (oid_ent_to_oid (&ent, oid) && 
	(oi = update_otherInformation(otherInfo, 0, oid, 1, 1)) &&
	oi->which == Z_OtherInfo_characterInfo)
	return oi->information.characterInfo;
    return 0;
}

char *Yaz_Proxy::get_proxy(Z_OtherInformation **otherInfo)
{
    int oid[OID_SIZE];
    Z_OtherInformationUnit *oi;
    struct oident ent;
    ent.proto = PROTO_Z3950;
    ent.oclass = CLASS_USERINFO;
    ent.value = (oid_value) VAL_PROXY;
    if (oid_ent_to_oid (&ent, oid) &&
	(oi = update_otherInformation(otherInfo, 0, oid, 1, 1)) &&
	oi->which == Z_OtherInfo_characterInfo)
	return oi->information.characterInfo;
    return 0;
}

Yaz_ProxyClient *Yaz_Proxy::get_client(Z_APDU *apdu)
{
    assert (m_parent);
    Yaz_Proxy *parent = m_parent;
    Z_OtherInformation **oi;
    Yaz_ProxyClient *c = m_client;
    
    get_otherInfoAPDU(apdu, &oi);
    char *cookie = get_cookie(oi);
    logf (LOG_LOG, "Yaz_Proxy::get_client cookie=%s", cookie ? cookie :
	  "null");
    if (cookie)
    {
	for (c = parent->m_clientPool; c; c = c->m_next)
	{
	    assert (c->m_prev);
	    assert (*c->m_prev == c);
	    if (!strcmp(cookie,c->m_cookie))
	    {
		logf (LOG_LOG, "Yaz_Proxy::get_client cached");
		return c;
	    }
	}
	
    }
    if (!m_client)
    {
	logf (LOG_LOG, "Yaz_Proxy::get_client creating new");
	c = new Yaz_ProxyClient(m_PDU_Observable->clone());
	c->m_next = parent->m_clientPool;
	if (c->m_next)
	    c->m_next->m_prev = &c->m_next;
	parent->m_clientPool = c;
	c->m_prev = &parent->m_clientPool;

	sprintf (c->m_cookie, "%d", parent->m_seqno);
	(parent->m_seqno)++;

	if (apdu->which == Z_APDU_initRequest)
	{
	    logf (LOG_LOG, "got InitRequest");
	    
	    char *proxy_host = get_proxy(&apdu->u.initRequest->otherInfo);
	    if (proxy_host)
		c->client(proxy_host);
	    else
		c->client("localhost:9999");
	}
	c->timeout(600);
    }
    return c;
}

Z_APDU *Yaz_Proxy::result_set_optimize(Z_APDU *apdu)
{
    if (apdu->which != Z_APDU_searchRequest)
	return apdu;
    Z_SearchRequest *sr = apdu->u.searchRequest;
    Yaz_Z_Query *this_query = new Yaz_Z_Query;
    
    this_query->set_Z_Query(sr->query);
    
    if (m_client->m_last_query &&
	m_client->m_last_query->match(this_query))
    {
	delete this_query;
	if (m_client->m_last_resultCount > *sr->smallSetUpperBound &&
	    m_client->m_last_resultCount < *sr->largeSetLowerBound)
	{
	    // medium Set
	    Z_APDU *new_apdu = create_Z_PDU(Z_APDU_presentRequest);
	    Z_PresentRequest *pr = new_apdu->u.presentRequest;
	    pr->referenceId = sr->referenceId;
	    pr->resultSetId = sr->resultSetName;
	    pr->preferredRecordSyntax = sr->preferredRecordSyntax;
	    *pr->numberOfRecordsRequested = *sr->mediumSetPresentNumber;
	    if (sr->mediumSetElementSetNames)
	    {
		pr->recordComposition = (Z_RecordComposition *)
		    odr_malloc(odr_encode(), sizeof(Z_RecordComposition));
		pr->recordComposition->which = Z_RecordComp_simple;
		pr->recordComposition->u.simple = sr->mediumSetElementSetNames;
	    }
	    m_client->m_sr_transform = 1;
	    return new_apdu;
	}
	else if (m_client->m_last_resultCount >= *sr->largeSetLowerBound ||
	    m_client->m_last_resultCount == 0)
	{
	    // large set
	    Z_APDU *new_apdu = create_Z_PDU(Z_APDU_searchResponse);
	    new_apdu->u.searchResponse->referenceId = sr->referenceId;
	    new_apdu->u.searchResponse->resultCount =
		&m_client->m_last_resultCount;
	    send_Z_PDU(new_apdu);
	    return 0;
	}
	else
	{
	    // small set
	    Z_APDU *new_apdu = create_Z_PDU(Z_APDU_presentRequest);
	    Z_PresentRequest *pr = new_apdu->u.presentRequest;
	    pr->referenceId = sr->referenceId;
	    pr->resultSetId = sr->resultSetName;
	    pr->preferredRecordSyntax = sr->preferredRecordSyntax;
	    *pr->numberOfRecordsRequested = m_client->m_last_resultCount;
	    if (sr->smallSetElementSetNames)
	    {
		pr->recordComposition = (Z_RecordComposition *)
		    odr_malloc(odr_encode(), sizeof(Z_RecordComposition));
		pr->recordComposition->which = Z_RecordComp_simple;
		pr->recordComposition->u.simple = sr->smallSetElementSetNames;
	    }
	    m_client->m_sr_transform = 1;
	    return new_apdu;
	}
    }
    else
    {
	logf (LOG_LOG, "query doesn't match");
	delete m_client->m_last_query;
	m_client->m_last_query = this_query;
    }
    return apdu;
}

void Yaz_Proxy::recv_Z_PDU(Z_APDU *apdu)
{
    logf (LOG_LOG, "Yaz_Proxy::recv_Z_PDU");
    // Determine our client.
    m_client = get_client(apdu);
    if (!m_client)
    {
	delete this;
	return;
    }
    m_client->m_server = this;

#if 0
    Z_OtherInformation **oi;
    get_otherInfoAPDU(apdu, &oi);
    *oi = 0;
#endif

    if (apdu->which == Z_APDU_initRequest)
    {
	if (m_client->m_init_flag)
	{
	    Z_APDU *apdu = create_Z_PDU(Z_APDU_initResponse);
	    if (m_client->m_cookie)
		set_otherInformationString(apdu, VAL_COOKIE, 1,
					   m_client->m_cookie);
	    send_Z_PDU(apdu);
	    return;
	}
	m_client->m_init_flag = 1;
    }
    apdu = result_set_optimize(apdu);
    if (!apdu)
	return;

    logf (LOG_LOG, "Yaz_ProxyClient::send_Z_PDU");
    if (m_client->send_Z_PDU(apdu) < 0)
    {
	delete m_client;
	delete this;
    }
}

void Yaz_Proxy::failNotify()
{
    logf (LOG_LOG, "failNotity server");
    if (m_keepalive)
    {
	// Tell client (if any) that no server connection is there..
	if (m_client)
	    m_client->m_server = 0;
    }
    else
    {
	delete m_client;
    }
    delete this;
}

void Yaz_ProxyClient::failNotify()
{
    logf (LOG_LOG, "failNotity client");
    delete m_server;
    delete this;
}

IYaz_PDU_Observer *Yaz_ProxyClient::clone(IYaz_PDU_Observable
					  *the_PDU_Observable)
{
    return new Yaz_ProxyClient(the_PDU_Observable);
}

Yaz_ProxyClient::~Yaz_ProxyClient()
{
    if (m_prev)
    {
	*m_prev = m_next;
	if (m_next)
	    m_next->m_prev = m_prev;
    }
    delete m_last_query;
}

void Yaz_Proxy::timeoutNotify()
{
    failNotify();
}

void Yaz_ProxyClient::timeoutNotify()
{
    failNotify();
}

Yaz_ProxyClient::Yaz_ProxyClient(IYaz_PDU_Observable *the_PDU_Observable) :
    Yaz_Z_Assoc (the_PDU_Observable)
{
    m_cookie[0] = 0;
    m_next = 0;
    m_prev = 0;
    m_init_flag = 0;
    m_last_query = 0;
    m_last_resultCount = 0;
    m_sr_transform = 0;
}

void Yaz_ProxyClient::recv_Z_PDU(Z_APDU *apdu)
{
    logf (LOG_LOG, "Yaz_ProxyClient::recv_Z_PDU");
    if (apdu->which == Z_APDU_searchResponse)
	m_last_resultCount = *apdu->u.searchResponse->resultCount;
    if (apdu->which == Z_APDU_presentResponse && m_sr_transform)
    {
	m_sr_transform = 0;
	Z_PresentResponse *pr = apdu->u.presentResponse;
	Z_APDU *new_apdu = create_Z_PDU(Z_APDU_searchResponse);
	Z_SearchResponse *sr = new_apdu->u.searchResponse;
	sr->referenceId = pr->referenceId;
	*sr->resultCount = m_last_resultCount;
	sr->records = pr->records;
	sr->nextResultSetPosition = pr->nextResultSetPosition;
	sr->numberOfRecordsReturned = pr->numberOfRecordsReturned;
	apdu = new_apdu;
    }
    if (m_cookie)
	set_otherInformationString (apdu, VAL_COOKIE, 1, m_cookie);
    if (m_server)
    {
	logf (LOG_LOG, "Yaz_Proxy::send_Z_PDU");
	m_server->send_Z_PDU(apdu);
    }
}
