/*
 * Copyright (c) 1998-2000, Index Data.
 * See the file LICENSE for details.
 * Sebastian Hammer, Adam Dickmeiss
 * 
 * $Log: yaz-proxy.cpp,v $
 * Revision 1.15  2000-08-31 14:41:55  adam
 * Proxy no longer generates cookies (it's up to the client). Proxy
 * re-opens if target new op is started before previous operation finishes.
 *
 * Revision 1.14  2000/08/10 08:42:42  adam
 * Fixes for {set,get}_APDU_log.
 *
 * Revision 1.13  2000/08/07 14:19:59  adam
 * Fixed serious bug regarding timeouts. Improved logging for proxy.
 *
 * Revision 1.12  2000/07/04 13:48:49  adam
 * Implemented upper-limit on proxy-to-target sessions.
 *
 * Revision 1.11  1999/12/06 13:52:45  adam
 * Modified for new location of YAZ header files. Experimental threaded
 * operation.
 *
 * Revision 1.10  1999/11/10 10:02:34  adam
 * Work on proxy.
 *
 * Revision 1.9  1999/09/13 12:53:44  adam
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
#include <time.h>

#include <yaz/log.h>
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
    m_proxyTarget = 0;
    m_max_clients = 50;
    m_seed = time(0);
}

Yaz_Proxy::~Yaz_Proxy()
{
    xfree (m_proxyTarget);
}

void Yaz_Proxy::set_proxyTarget(const char *target)
{
    xfree (m_proxyTarget);
    m_proxyTarget = 0;
    if (target)
	m_proxyTarget = (char *) xstrdup (target);
}

IYaz_PDU_Observer *Yaz_Proxy::clone(IYaz_PDU_Observable
				    *the_PDU_Observable)
{
    Yaz_Proxy *new_proxy = new Yaz_Proxy(the_PDU_Observable);
    new_proxy->m_parent = this;
    new_proxy->timeout(120);
    new_proxy->set_proxyTarget(m_proxyTarget);
    new_proxy->set_APDU_log(get_APDU_log());
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

    const char *proxy_host = get_proxy(oi);
    if (proxy_host)
	set_proxyTarget(proxy_host);
    logf (LOG_LOG, "proxy_host = %s", m_proxyTarget ? m_proxyTarget:"none");
    
    // no target specified at all?
    if (!m_proxyTarget)
	return 0;

    if (cookie)
    {
	logf (LOG_LOG, "lookup of clients cookie=%s target=%s",
	      cookie, m_proxyTarget);
	Yaz_ProxyClient *cc = 0;
	
	for (c = parent->m_clientPool; c; c = c->m_next)
	{
	    logf (LOG_LOG, " found client cookie = %s target=%s",
		  c->m_cookie, c->get_hostname());
	    assert (c->m_prev);
	    assert (*c->m_prev == c);
	    if (!strcmp(cookie,c->m_cookie) &&
		!strcmp(m_proxyTarget, c->get_hostname()))
	    {
		logf (LOG_LOG, "found!");
		cc = c;
	    }
	}
	if (cc)
	{
	    c = cc;
	    if (c->m_waiting)
	    {
		logf (LOG_LOG, "reopen target=%s", c->get_hostname());
		c->close();
		c->client(m_proxyTarget);
		c->m_init_flag = 0;
		
		delete c->m_last_query;
		c->m_last_query = 0;
		c->m_last_resultCount = 0;
		c->m_sr_transform = 0;
		c->m_waiting = 0;
		c->timeout(600);
	    }
	    c->m_seqno = parent->m_seqno;
	    (parent->m_seqno)++;
	    return c;
	}
    }
    if (!m_client)
    {
	if (apdu->which != Z_APDU_initRequest)
	{
	    logf (LOG_LOG, "no first INIT!");
	    return 0;
	}
	logf (LOG_LOG, "got InitRequest");
	    
	// go through list of clients - and find the lowest/oldest one.
	Yaz_ProxyClient *c_min = 0;
	int min_seq = -1;
	int no_of_clients = 0;
	for (c = parent->m_clientPool; c; c = c->m_next)
	{
	    no_of_clients++;
	    logf (LOG_LOG, "found seqno = %d", c->m_seqno);
	    if (min_seq < 0 || c->m_seqno < min_seq)
	    {
		min_seq = c->m_seqno;
		c_min = c;
	    }
	}
	if (no_of_clients >= parent->m_max_clients)
	{
	    c = c_min;
	    logf (LOG_LOG, "Yaz_Proxy::get_client re-using session %d",
		  c->m_seqno);
	    if (c->m_server)
		delete c->m_server;
	    c->m_server = 0;
	}
	else
	{
	    logf (LOG_LOG, "Yaz_Proxy::get_client making session %d",
		  parent->m_seqno);
	    c = new Yaz_ProxyClient(m_PDU_Observable->clone());
	    c->m_next = parent->m_clientPool;
	    if (c->m_next)
		c->m_next->m_prev = &c->m_next;
	    parent->m_clientPool = c;
	    c->m_prev = &parent->m_clientPool;
	}
	if (cookie)
	    strcpy (c->m_cookie, cookie);
	else
	    c->m_cookie[0] = '\0';
	logf (LOG_LOG, "Yaz_Proxy::get_client connect to %s", m_proxyTarget);
	c->m_seqno = parent->m_seqno;
	c->client(m_proxyTarget);
	c->m_init_flag = 0;

	delete c->m_last_query;
	c->m_last_query = 0;
	c->m_last_resultCount = 0;
	c->m_sr_transform = 0;
	c->m_waiting = 0;
	c->timeout(600);

	(parent->m_seqno)++;
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
	    logf (LOG_LOG, "Yaz_Proxy::result_set_optimize medium set");
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
	else if (m_client->m_last_resultCount > *sr->largeSetLowerBound ||
	    m_client->m_last_resultCount == 0)
	{
	    // large set
	    logf (LOG_LOG, "Yaz_Proxy::result_set_optimize large set");
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
	    logf (LOG_LOG, "Yaz_Proxy::result_set_optimize small set");
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
	logf (LOG_LOG, "Yaz_Proxy::result_set_optimize new set");
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

    logf (LOG_LOG, "Yaz_ProxyClient::send_Z_PDU %s", m_client->get_hostname());
    if (m_client->send_Z_PDU(apdu) < 0)
    {
	delete m_client;
	delete this;
    }
    else
	m_client->m_waiting = 1;
}

void Yaz_Proxy::connectNotify()
{
}

void Yaz_Proxy::shutdown()
{
    logf (LOG_LOG, "shutdown (client to proxy)");
    // only keep if keep_alive flag and cookie is set...
    if (m_keepalive && m_client && m_client->m_cookie[0])
    {
	// Tell client (if any) that no server connection is there..
	m_client->m_server = 0;
    }
    else
    {
	delete m_client;
    }
    delete this;
}

void Yaz_ProxyClient::shutdown()
{
    logf (LOG_LOG, "shutdown (proxy to server) %s", get_hostname());
    delete m_server;
    delete this;
}

void Yaz_Proxy::failNotify()
{
    logf (LOG_LOG, "connection closed by client");
    shutdown();
}

void Yaz_ProxyClient::failNotify()
{
    logf (LOG_LOG, "Yaz_ProxyClient connection closed by %s", get_hostname());
    shutdown();
}

void Yaz_ProxyClient::connectNotify()
{
    logf (LOG_LOG, "Yaz_ProxyClient connection accept by %s", get_hostname());
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
    logf (LOG_LOG, "timeout (client to proxy)");
    shutdown();
}

void Yaz_ProxyClient::timeoutNotify()
{
    logf (LOG_LOG, "timeout (proxy to target) %s", get_hostname());
    shutdown();
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
    m_waiting = 0;
}

void Yaz_ProxyClient::recv_Z_PDU(Z_APDU *apdu)
{
    m_waiting = 0;
    logf (LOG_LOG, "Yaz_ProxyClient::recv_Z_PDU %s", get_hostname());
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
