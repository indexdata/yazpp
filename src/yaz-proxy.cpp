/*
 * Copyright (c) 1998-2003, Index Data.
 * See the file LICENSE for details.
 * 
 * $Id: yaz-proxy.cpp,v 1.50 2003-10-04 06:44:16 adam Exp $
 */

#include <assert.h>
#include <time.h>

#include <yaz/log.h>
#include <yaz/diagbib1.h>
#include <yaz++/proxy.h>

static const char *apdu_name(Z_APDU *apdu)
{
    switch (apdu->which)
    {
    case Z_APDU_initRequest:
        return "initRequest";
    case Z_APDU_initResponse:
        return "initResponse";
    case Z_APDU_searchRequest:
	return "searchRequest";
    case Z_APDU_searchResponse:
	return "searchResponse";
    case Z_APDU_presentRequest:
	return "presentRequest";
    case Z_APDU_presentResponse:
	return "presentResponse";
    case Z_APDU_deleteResultSetRequest:
	return "deleteResultSetRequest";
    case Z_APDU_deleteResultSetResponse:
	return "deleteResultSetResponse";
    case Z_APDU_scanRequest:
	return "scanRequest";
    case Z_APDU_scanResponse:
	return "scanResponse";
    case Z_APDU_sortRequest:
	return "sortRequest";
    case Z_APDU_sortResponse:
	return "sortResponse";
    case Z_APDU_extendedServicesRequest:
	return "extendedServicesRequest";
    case Z_APDU_extendedServicesResponse:
	return "extendedServicesResponse";
    case Z_APDU_close:
	return "close";
    }
    return "other";
}

Yaz_Proxy::Yaz_Proxy(IYaz_PDU_Observable *the_PDU_Observable) :
    Yaz_Z_Assoc(the_PDU_Observable), m_bw_stat(60), m_pdu_stat(60)
{
    m_PDU_Observable = the_PDU_Observable;
    m_client = 0;
    m_parent = 0;
    m_clientPool = 0;
    m_seqno = 1;
    m_keepalive = 0;
    m_proxyTarget = 0;
    m_default_target = 0;
    m_proxy_authentication = 0;
    m_max_clients = 150;
    m_seed = time(0);
    m_client_idletime = 600;
    m_target_idletime = 600;
    m_optimize = xstrdup ("1");
    strcpy(m_session_str, "x");
    m_session_no=0;
    m_bytes_sent = m_bytes_recv = 0;
    m_bw_hold_PDU = 0;
    m_bw_max = 0;
    m_pdu_max = 0;
    m_max_record_retrieve = 0;
}

Yaz_Proxy::~Yaz_Proxy()
{
    yaz_log(LOG_LOG, "%s Closed %d/%d sent/recv bytes total", m_session_str,
	    m_bytes_sent, m_bytes_recv);
    xfree (m_proxyTarget);
    xfree (m_default_target);
    xfree (m_proxy_authentication);
    xfree (m_optimize);
}

int Yaz_Proxy::set_config(const char *config)
{
    int r = m_config.read_xml(config);
    return r;
}

void Yaz_Proxy::set_default_target(const char *target)
{
    xfree (m_default_target);
    m_default_target = 0;
    if (target)
	m_default_target = (char *) xstrdup (target);
}

void Yaz_Proxy::set_proxy_authentication (const char *auth)
{
    xfree (m_proxy_authentication);
    m_proxy_authentication = 0;
    if (auth)
	m_proxy_authentication = (char *) xstrdup (auth);
}

IYaz_PDU_Observer *Yaz_Proxy::sessionNotify(IYaz_PDU_Observable
					    *the_PDU_Observable, int fd)
{
    Yaz_Proxy *new_proxy = new Yaz_Proxy(the_PDU_Observable);
    new_proxy->m_parent = this;
    new_proxy->m_config = m_config;
    new_proxy->timeout(m_client_idletime);
    new_proxy->m_target_idletime = m_target_idletime;
    new_proxy->set_default_target(m_default_target);
    new_proxy->set_APDU_log(get_APDU_log());
    new_proxy->set_proxy_authentication(m_proxy_authentication);
    sprintf(new_proxy->m_session_str, "%ld:%d", (long) time(0), m_session_no);
    m_session_no++;
    yaz_log (LOG_LOG, "%s New session %s", new_proxy->m_session_str,
	     the_PDU_Observable->getpeername());
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

    if (!m_proxyTarget)
    {
	const char *proxy_host = get_proxy(oi);
	if (proxy_host)
	{
	    xfree(m_default_target);
	    m_default_target = xstrdup(proxy_host);
	    proxy_host = m_default_target;
	}
	
	const char *url = 0;
	int client_idletime = -1;
	m_config.get_target_info(proxy_host, &url, &m_keepalive, &m_bw_max,
				 &m_pdu_max, &m_max_record_retrieve,
				 &m_target_idletime, &client_idletime,
				 &parent->m_max_clients);
	if (client_idletime != -1)
	{
	    m_client_idletime = client_idletime;
	    timeout(m_client_idletime);
	}
	if (!url)
	{
	    yaz_log(LOG_LOG, "%s No default target", m_session_str);
	    return 0;
	}
	m_proxyTarget = (char*) xstrdup(url);
    }
    if (cookie && *cookie)
    {
	Yaz_ProxyClient *cc = 0;
	
	for (c = parent->m_clientPool; c; c = c->m_next)
	{
	    assert (c->m_prev);
	    assert (*c->m_prev == c);
	    if (c->m_cookie && !strcmp(cookie,c->m_cookie) &&
		!strcmp(m_proxyTarget, c->get_hostname()))
	    {
		cc = c;
	    }
	}
	if (cc)
	{
	    // found it in cache
	    c = cc;
	    // The following handles "cancel"
	    // If connection is busy (waiting for PDU) and
	    // we have an initRequest we can safely do re-open
	    if (c->m_waiting && apdu->which == Z_APDU_initRequest)
	    {
		yaz_log (LOG_LOG, "%s REOPEN target=%s", m_session_str,
			 c->get_hostname());
		c->close();
		c->client(m_proxyTarget);
		c->m_init_flag = 0;

		c->m_last_ok = 0;
		c->m_cache.clear();
		c->m_last_resultCount = 0;
		c->m_sr_transform = 0;
		c->m_waiting = 0;
		c->m_resultSetStartPoint = 0;
		c->timeout(m_target_idletime); 
	    }
	    c->m_seqno = parent->m_seqno;
	    if (c->m_server && c->m_server != this)
		c->m_server->m_client = 0;
	    c->m_server = this;
	    (parent->m_seqno)++;
	    yaz_log (LOG_DEBUG, "get_client 1 %p %p", this, c);
	    return c;
	}
    }
    else if (!c)
    {
	Yaz_ProxyClient *cc = 0;
	
	for (c = parent->m_clientPool; c; c = c->m_next)
	{
	    assert (c->m_prev);
	    assert (*c->m_prev == c);
	    if (c->m_server == 0 && c->m_cookie == 0 && 
		!strcmp(m_proxyTarget, c->get_hostname()))
	    {
		cc = c;
	    }
	}
	if (cc)
	{
	    // found it in cache
	    c = cc;

	    yaz_log (LOG_LOG, "%s REUSE %d %d %s",
		     m_session_str,
		     c->m_seqno, parent->m_seqno, c->get_hostname());

	    c->m_seqno = parent->m_seqno;
	    assert(c->m_server == 0);
	    c->m_server = this;
	    
	    (parent->m_seqno)++;
	    return c;
	}
    }
    if (!m_client)
    {
	if (apdu->which != Z_APDU_initRequest)
	{
	    yaz_log (LOG_LOG, "no first INIT!");
	    return 0;
	}
        Z_InitRequest *initRequest = apdu->u.initRequest;

        if (!initRequest->idAuthentication)
        {
            if (m_proxy_authentication)
            {
                initRequest->idAuthentication =
                    (Z_IdAuthentication *)
                    odr_malloc (odr_encode(),
                                sizeof(*initRequest->idAuthentication));
                initRequest->idAuthentication->which =
                    Z_IdAuthentication_open;
                initRequest->idAuthentication->u.open =
                    odr_strdup (odr_encode(), m_proxy_authentication);
            }
        }

	// go through list of clients - and find the lowest/oldest one.
	Yaz_ProxyClient *c_min = 0;
	int min_seq = -1;
	int no_of_clients = 0;
	if (parent->m_clientPool)
	    yaz_log (LOG_LOG, "Existing sessions");
	for (c = parent->m_clientPool; c; c = c->m_next)
	{
	    yaz_log (LOG_LOG, " Session %-3d wait=%d %s cookie=%s", c->m_seqno,
			       c->m_waiting, c->get_hostname(),
			       c->m_cookie ? c->m_cookie : "");
	    no_of_clients++;
	    if (min_seq < 0 || c->m_seqno < min_seq)
	    {
		min_seq = c->m_seqno;
		c_min = c;
	    }
	}
	if (no_of_clients >= parent->m_max_clients)
	{
	    c = c_min;
	    if (c->m_waiting || strcmp(m_proxyTarget, c->get_hostname()))
	    {
		yaz_log (LOG_LOG, "%s MAXCLIENTS Destroy %d",
			 m_session_str, c->m_seqno);
		if (c->m_server && c->m_server != this)
		    delete c->m_server;
		c->m_server = 0;
	    }
	    else
	    {
		yaz_log (LOG_LOG, "%s MAXCLIENTS Reuse %d %d %s",
			 m_session_str,
			 c->m_seqno, parent->m_seqno, c->get_hostname());
		xfree (c->m_cookie);
		c->m_cookie = 0;
		if (cookie)
		    c->m_cookie = xstrdup(cookie);
		c->m_seqno = parent->m_seqno;
		if (c->m_server && c->m_server != this)
		{
		    c->m_server->m_client = 0;
		    delete c->m_server;
		}
		(parent->m_seqno)++;
		return c;
	    }
	}
	else
	{
	    yaz_log (LOG_LOG, "%s NEW %d %s",
		     m_session_str, parent->m_seqno, m_proxyTarget);
	    c = new Yaz_ProxyClient(m_PDU_Observable->clone());
	    c->m_next = parent->m_clientPool;
	    if (c->m_next)
		c->m_next->m_prev = &c->m_next;
	    parent->m_clientPool = c;
	    c->m_prev = &parent->m_clientPool;
	}

	xfree (c->m_cookie);
	c->m_cookie = 0;
	if (cookie)
	    c->m_cookie = xstrdup(cookie);

	c->m_seqno = parent->m_seqno;
	c->client(m_proxyTarget);
	c->m_init_flag = 0;
	c->m_last_resultCount = 0;
        c->m_last_ok = 0;
	c->m_cache.clear();
	c->m_sr_transform = 0;
	c->m_waiting = 0;
	c->m_resultSetStartPoint = 0;
	c->timeout(30);

	(parent->m_seqno)++;
    }
    yaz_log (LOG_DEBUG, "get_client 3 %p %p", this, c);
    return c;
}

void Yaz_Proxy::display_diagrecs(Z_DiagRec **pp, int num)
{
    int i;
    for (i = 0; i<num; i++)
    {
	oident *ent;
	Z_DefaultDiagFormat *r;
        Z_DiagRec *p = pp[i];
        if (p->which != Z_DiagRec_defaultFormat)
        {
	    yaz_log(LOG_LOG, "%s Error no diagnostics", m_session_str);
            return;
        }
        else
            r = p->u.defaultFormat;
        if (!(ent = oid_getentbyoid(r->diagnosticSetId)) ||
            ent->oclass != CLASS_DIAGSET || ent->value != VAL_BIB1)
	    yaz_log(LOG_LOG, "%s Error unknown diagnostic set", m_session_str);
        switch (r->which)
        {
        case Z_DefaultDiagFormat_v2Addinfo:
	    yaz_log(LOG_LOG, "%s Error %d %s:%s",
		    m_session_str,
		    *r->condition, diagbib1_str(*r->condition),
		    r->u.v2Addinfo);
            break;
        case Z_DefaultDiagFormat_v3Addinfo:
	    yaz_log(LOG_LOG, "%s Error %d %s:%s",
		    m_session_str,
		    *r->condition, diagbib1_str(*r->condition),
		    r->u.v3Addinfo);
            break;
        }
    }
}

int Yaz_Proxy::send_to_client(Z_APDU *apdu)
{
    int len = 0;
    if (apdu->which == Z_APDU_searchResponse)
    {
	Z_SearchResponse *sr = apdu->u.searchResponse;
	Z_Records *p = sr->records;
	if (p && p->which == Z_Records_NSD)
	{
	    Z_DiagRec dr, *dr_p = &dr;
	    dr.which = Z_DiagRec_defaultFormat;
	    dr.u.defaultFormat = p->u.nonSurrogateDiagnostic;

	    display_diagrecs(&dr_p, 1);
	}
	else
	{
	    if (sr->resultCount)
		yaz_log(LOG_LOG, "%s %d hits", m_session_str,
			*sr->resultCount);
	}
    }
    else if (apdu->which == Z_APDU_presentResponse)
    {
	Z_PresentResponse *sr = apdu->u.presentResponse;
	Z_Records *p = sr->records;
	if (p && p->which == Z_Records_NSD)
	{
	    Z_DiagRec dr, *dr_p = &dr;
	    dr.which = Z_DiagRec_defaultFormat;
	    dr.u.defaultFormat = p->u.nonSurrogateDiagnostic;

	    display_diagrecs(&dr_p, 1);
	}
    }
    int r = send_Z_PDU(apdu, &len);
    yaz_log (LOG_LOG, "%s Sending %s to client %d bytes", m_session_str,
	     apdu_name(apdu), len);
    m_bytes_sent += len;
    m_bw_stat.add_bytes(len);
    return r;
}

int Yaz_ProxyClient::send_to_target(Z_APDU *apdu)
{
    int len = 0;
    int r = send_Z_PDU(apdu, &len);
    yaz_log (LOG_LOG, "%s Sending %s to %s %d bytes",
	     get_session_str(),
	     apdu_name(apdu), get_hostname(), len);
    m_bytes_sent += len;
    return r;
}

Z_APDU *Yaz_Proxy::result_set_optimize(Z_APDU *apdu)
{
    if (*m_parent->m_optimize == '0')
        return apdu;      // don't optimize result sets..
    if (apdu->which == Z_APDU_presentRequest)
    {
	Z_PresentRequest *pr = apdu->u.presentRequest;
	Z_NamePlusRecordList *npr;
	int toget = *pr->numberOfRecordsRequested;
	int start = *pr->resultSetStartPoint;

	if (m_client->m_last_resultSetId &&
	    !strcmp(m_client->m_last_resultSetId, pr->resultSetId))
	{
	    if (m_client->m_cache.lookup (odr_encode(), &npr, start, toget,
					  pr->preferredRecordSyntax,
					  pr->recordComposition))
	    {
		yaz_log (LOG_LOG, "%s Returned cache records for present request", 
			 m_session_str);
		Z_APDU *new_apdu = create_Z_PDU(Z_APDU_presentResponse);
		new_apdu->u.presentResponse->referenceId = pr->referenceId;
		
		new_apdu->u.presentResponse->numberOfRecordsReturned
		    = odr_intdup(odr_encode(), toget);
								 
		new_apdu->u.presentResponse->records = (Z_Records*)
		    odr_malloc(odr_encode(), sizeof(Z_Records));
		new_apdu->u.presentResponse->records->which = Z_Records_DBOSD;
		new_apdu->u.presentResponse->records->u.databaseOrSurDiagnostics = npr;
		new_apdu->u.presentResponse->nextResultSetPosition =
		    odr_intdup(odr_encode(), start+toget);

		send_to_client(new_apdu);
		return 0;
	    }
	}
    }

    if (apdu->which != Z_APDU_searchRequest)
	return apdu;
    Z_SearchRequest *sr = apdu->u.searchRequest;
    Yaz_Z_Query *this_query = new Yaz_Z_Query;
    Yaz_Z_Databases this_databases;

    this_databases.set(sr->num_databaseNames, (const char **)
                       sr->databaseNames);
    
    this_query->set_Z_Query(sr->query);

    char query_str[80];
    this_query->print(query_str, sizeof(query_str)-1);
    yaz_log(LOG_LOG, "%s Query %s", m_session_str, query_str);

    if (m_client->m_last_ok && m_client->m_last_query &&
	m_client->m_last_query->match(this_query) &&
        !strcmp(m_client->m_last_resultSetId, sr->resultSetName) &&
        m_client->m_last_databases.match(this_databases))
    {
	delete this_query;
	if (m_client->m_last_resultCount > *sr->smallSetUpperBound &&
	    m_client->m_last_resultCount < *sr->largeSetLowerBound)
	{
	    Z_NamePlusRecordList *npr;
	    int toget = *sr->mediumSetPresentNumber;
	    Z_RecordComposition *comp = 0;

	    if (toget > m_client->m_last_resultCount)
		toget = m_client->m_last_resultCount;
	    
	    if (sr->mediumSetElementSetNames)
	    {
		comp = (Z_RecordComposition *)
		    odr_malloc(odr_encode(), sizeof(Z_RecordComposition));
		comp->which = Z_RecordComp_simple;
		comp->u.simple = sr->mediumSetElementSetNames;
	    }
 
	    if (m_client->m_cache.lookup (odr_encode(), &npr, 1, toget,
					  sr->preferredRecordSyntax, comp))
	    {
		yaz_log (LOG_LOG, "%s Returned cache records for medium set",
			 m_session_str);
		Z_APDU *new_apdu = create_Z_PDU(Z_APDU_searchResponse);
		new_apdu->u.searchResponse->referenceId = sr->referenceId;
		new_apdu->u.searchResponse->resultCount =
		    &m_client->m_last_resultCount;
		
		new_apdu->u.searchResponse->numberOfRecordsReturned
		    = odr_intdup(odr_encode(), toget);
							
		new_apdu->u.searchResponse->presentStatus =
		    odr_intdup(odr_encode(), Z_PresentStatus_success);
		new_apdu->u.searchResponse->records = (Z_Records*)
		    odr_malloc(odr_encode(), sizeof(Z_Records));
		new_apdu->u.searchResponse->records->which = Z_Records_DBOSD;
		new_apdu->u.searchResponse->records->u.databaseOrSurDiagnostics = npr;
		new_apdu->u.searchResponse->nextResultSetPosition =
		    odr_intdup(odr_encode(), toget+1);
		send_to_client(new_apdu);
		return 0;
	    }
	    else
	    {
		// medium Set
		// send present request (medium size)
		yaz_log (LOG_LOG, "%s Optimizing search for medium set",
			 m_session_str);

		Z_APDU *new_apdu = create_Z_PDU(Z_APDU_presentRequest);
		Z_PresentRequest *pr = new_apdu->u.presentRequest;
		pr->referenceId = sr->referenceId;
		pr->resultSetId = sr->resultSetName;
		pr->preferredRecordSyntax = sr->preferredRecordSyntax;
		*pr->numberOfRecordsRequested = toget;
		pr->recordComposition = comp;
		m_client->m_sr_transform = 1;
		return new_apdu;
	    }
	}
	else if (m_client->m_last_resultCount >= *sr->largeSetLowerBound ||
	    m_client->m_last_resultCount <= 0)
	{
            // large set. Return pseudo-search response immediately
	    yaz_log (LOG_LOG, "%s Optimizing search for large set",
		     m_session_str);
	    Z_APDU *new_apdu = create_Z_PDU(Z_APDU_searchResponse);
	    new_apdu->u.searchResponse->referenceId = sr->referenceId;
	    new_apdu->u.searchResponse->resultCount =
		&m_client->m_last_resultCount;
	    send_to_client(new_apdu);
	    return 0;
	}
	else
	{
	    Z_NamePlusRecordList *npr;
	    int toget = m_client->m_last_resultCount;
	    Z_RecordComposition *comp = 0;
	    // small set
            // send a present request (small set)
	    
	    if (sr->smallSetElementSetNames)
	    {
		comp = (Z_RecordComposition *)
		    odr_malloc(odr_encode(), sizeof(Z_RecordComposition));
		comp->which = Z_RecordComp_simple;
		comp->u.simple = sr->smallSetElementSetNames;
	    }

	    if (m_client->m_cache.lookup (odr_encode(), &npr, 1, toget,
					  sr->preferredRecordSyntax, comp))
	    {
		yaz_log (LOG_LOG, "%s Returned cache records for small set",
			 m_session_str);
		Z_APDU *new_apdu = create_Z_PDU(Z_APDU_searchResponse);
		new_apdu->u.searchResponse->referenceId = sr->referenceId;
		new_apdu->u.searchResponse->resultCount =
		    &m_client->m_last_resultCount;
		
		new_apdu->u.searchResponse->numberOfRecordsReturned
		    = odr_intdup(odr_encode(), toget);
								 
		new_apdu->u.searchResponse->presentStatus =
		    odr_intdup(odr_encode(), Z_PresentStatus_success);
		new_apdu->u.searchResponse->records = (Z_Records*)
		    odr_malloc(odr_encode(), sizeof(Z_Records));
		new_apdu->u.searchResponse->records->which = Z_Records_DBOSD;
		new_apdu->u.searchResponse->records->u.databaseOrSurDiagnostics = npr;
		new_apdu->u.searchResponse->nextResultSetPosition =
		    odr_intdup(odr_encode(), toget+1);
		send_to_client(new_apdu);
		return 0;
	    }
	    else
	    {
		yaz_log (LOG_LOG, "%s Optimizing search for small set",
			 m_session_str);
		Z_APDU *new_apdu = create_Z_PDU(Z_APDU_presentRequest);
		Z_PresentRequest *pr = new_apdu->u.presentRequest;
		pr->referenceId = sr->referenceId;
		pr->resultSetId = sr->resultSetName;
		pr->preferredRecordSyntax = sr->preferredRecordSyntax;
		*pr->numberOfRecordsRequested = toget;
		pr->recordComposition = comp;
		m_client->m_sr_transform = 1;
		return new_apdu;
	    }
	}
    }
    else
    {
	delete m_client->m_last_query;
	m_client->m_last_query = this_query;
        m_client->m_last_ok = 0;
	m_client->m_cache.clear();
	m_client->m_resultSetStartPoint = 0;

        xfree (m_client->m_last_resultSetId);
        m_client->m_last_resultSetId = xstrdup (sr->resultSetName);

        m_client->m_last_databases.set(sr->num_databaseNames,
                                       (const char **) sr->databaseNames);
    }
    return apdu;
}


void Yaz_Proxy::recv_Z_PDU(Z_APDU *apdu, int len)
{
    int reduce = 0;
    m_bytes_recv += len;
    
    yaz_log (LOG_LOG, "%s Receiving %s from client %d bytes", m_session_str,
	     apdu_name(apdu), len);

    if (m_bw_hold_PDU)     // double incoming PDU. shutdown now.
	shutdown();

    m_bw_stat.add_bytes(len);
    m_pdu_stat.add_bytes(1);

    int bw_total = m_bw_stat.get_total();
    int pdu_total = m_pdu_stat.get_total();

    yaz_log(LOG_LOG, "%s stat bw=%d pdu=%d limit-bw=%d limit-pdu=%d",
	    m_session_str, bw_total, pdu_total, m_bw_max, m_pdu_max);
    if (m_bw_max)
    {
	if (bw_total > m_bw_max)
	{
	    reduce = (bw_total/m_bw_max);
	}
    }
    if (m_pdu_max)
    {
	if (pdu_total > m_pdu_max)
	{
	    int nreduce = (60/m_pdu_max);
	    reduce = (reduce > nreduce) ? reduce : nreduce;
	}
    }
    if (reduce)  
    {
	yaz_log(LOG_LOG, "%s Limit delay=%d", m_session_str, reduce);
	m_bw_hold_PDU = apdu;  // save PDU and signal "on hold"
	timeout(reduce);       // call us reduce seconds later
    }
    else
	recv_Z_PDU_0(apdu);    // all fine. Proceed receive PDU as usual
}

void Yaz_Proxy::handle_max_record_retrieve(Z_APDU *apdu)
{
    if (m_max_record_retrieve)
    {
	if (apdu->which == Z_APDU_presentRequest)
	{
	    Z_PresentRequest *pr = apdu->u.presentRequest;
	    if (pr->numberOfRecordsRequested && 
		*pr->numberOfRecordsRequested > m_max_record_retrieve)
		*pr->numberOfRecordsRequested = m_max_record_retrieve;
	}
    }
}

Z_Records *Yaz_Proxy::create_nonSurrogateDiagnostics(ODR odr,
						     int error,
						     const char *addinfo)
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

Z_APDU *Yaz_Proxy::handle_query_validation(Z_APDU *apdu)
{
    if (apdu->which == Z_APDU_searchRequest)
    {
	Z_SearchRequest *sr = apdu->u.searchRequest;
	int err;
	char *addinfo = 0;
	err = m_config.check_query(odr_encode(), m_default_target, sr->query,
				   &addinfo);
	if (err)
	{
	    Z_APDU *new_apdu = create_Z_PDU(Z_APDU_searchResponse);
	    int *nulint = odr_intdup (odr_encode(), 0);

	    new_apdu->u.searchResponse->referenceId = sr->referenceId;
	    new_apdu->u.searchResponse->records =
		create_nonSurrogateDiagnostics(odr_encode(), err, addinfo);
	    new_apdu->u.searchResponse->searchStatus = nulint;
	    new_apdu->u.searchResponse->resultCount = nulint;

	    send_to_client(new_apdu);

	    return 0;
	}
    }
    return apdu;
}

void Yaz_Proxy::recv_Z_PDU_0(Z_APDU *apdu)
{
    // Determine our client.
    m_client = get_client(apdu);
    if (!m_client)
    {
	delete this;
	return;
    }
    m_client->m_server = this;

    if (apdu->which == Z_APDU_initRequest)
    {
	if (m_client->m_init_flag)
	{
	    Z_APDU *apdu = m_client->m_initResponse;
	    apdu->u.initResponse->otherInfo = 0;
	    if (m_client->m_cookie && *m_client->m_cookie)
		set_otherInformationString(apdu, VAL_COOKIE, 1,
					   m_client->m_cookie);
	    send_to_client(apdu);
	    return;
	}
	m_client->m_init_flag = 1;
    }
    handle_max_record_retrieve(apdu);

    if (apdu)
	apdu = handle_query_validation(apdu);

    if (apdu)
	apdu = result_set_optimize(apdu);
    if (!apdu)
    {
	m_client->timeout(m_target_idletime);  // mark it active even 
	// though we didn't use it
	return;
    }

    // delete other info part from PDU before sending to target
    Z_OtherInformation **oi;
    get_otherInfoAPDU(apdu, &oi);
    if (oi)
        *oi = 0;

    if (apdu->which == Z_APDU_presentRequest &&
	m_client->m_resultSetStartPoint == 0)
    {
	Z_PresentRequest *pr = apdu->u.presentRequest;
	m_client->m_resultSetStartPoint = *pr->resultSetStartPoint;
	m_client->m_cache.copy_presentRequest(apdu->u.presentRequest);
    } else {
	m_client->m_resultSetStartPoint = 0;
    }
    if (m_client->send_to_target(apdu) < 0)
    {
	delete m_client;
	m_client = 0;
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
    // only keep if keep_alive flag is set...
    if (m_keepalive && m_client && m_client->m_waiting == 0)
    {
        yaz_log (LOG_LOG, "%s Shutdown (client to proxy) keepalive %s",
		 m_session_str,
                 m_client->get_hostname());
        assert (m_client->m_waiting != 2);
	// Tell client (if any) that no server connection is there..
	m_client->m_server = 0;
    }
    else if (m_client)
    {
        yaz_log (LOG_LOG, "%s Shutdown (client to proxy) close %s",
		 m_session_str,
                 m_client->get_hostname());
        assert (m_client->m_waiting != 2);
	delete m_client;
    }
    else if (!m_parent)
    {
        yaz_log (LOG_LOG, "%s shutdown (client to proxy) bad state",
		 m_session_str);
        assert (m_parent);
    }
    else 
    {
        yaz_log (LOG_LOG, "%s Shutdown (client to proxy)",
		 m_session_str);
    }
    delete this;
}

const char *Yaz_ProxyClient::get_session_str() 
{
    if (!m_server)
	return "0";
    return m_server->get_session_str();
}

void Yaz_ProxyClient::shutdown()
{
    yaz_log (LOG_LOG, "%s Shutdown (proxy to target) %s", get_session_str(),
	     get_hostname());
    delete m_server;
    delete this;
}

void Yaz_Proxy::failNotify()
{
    yaz_log (LOG_LOG, "%s Connection closed by client",
	     get_session_str());
    shutdown();
}

void Yaz_ProxyClient::failNotify()
{
    yaz_log (LOG_LOG, "%s Connection closed by target %s", 
	     get_session_str(), get_hostname());
    shutdown();
}

void Yaz_ProxyClient::connectNotify()
{
    yaz_log (LOG_LOG, "%s Connection accepted by %s", get_session_str(),
	     get_hostname());
    int to;
    if (m_server)
	to = m_server->get_target_idletime();
    else
	to = 600;
    timeout(to);
}

IYaz_PDU_Observer *Yaz_ProxyClient::sessionNotify(IYaz_PDU_Observable
						  *the_PDU_Observable, int fd)
{
    return new Yaz_ProxyClient(the_PDU_Observable);
}

Yaz_ProxyClient::~Yaz_ProxyClient()
{
    if (m_prev)
	*m_prev = m_next;
    if (m_next)
	m_next->m_prev = m_prev;
    m_waiting = 2;     // for debugging purposes only.
    odr_destroy(m_init_odr);
    delete m_last_query;
    xfree (m_last_resultSetId);
    xfree (m_cookie);
}

void Yaz_Proxy::timeoutNotify()
{
    if (m_bw_hold_PDU)
    {
	timeout(m_client_idletime);
	Z_APDU *apdu = m_bw_hold_PDU;
	m_bw_hold_PDU = 0;
	recv_Z_PDU_0(apdu);
    }
    else
    {
	yaz_log (LOG_LOG, "%s Timeout (client to proxy)", m_session_str);
	shutdown();
    }
}

void Yaz_ProxyClient::timeoutNotify()
{
    yaz_log (LOG_LOG, "%s Timeout (proxy to target) %s", get_session_str(),
	     get_hostname());
    shutdown();
}

Yaz_ProxyClient::Yaz_ProxyClient(IYaz_PDU_Observable *the_PDU_Observable) :
    Yaz_Z_Assoc (the_PDU_Observable)
{
    m_cookie = 0;
    m_next = 0;
    m_prev = 0;
    m_init_flag = 0;
    m_last_query = 0;
    m_last_resultSetId = 0;
    m_last_resultCount = 0;
    m_last_ok = 0;
    m_sr_transform = 0;
    m_waiting = 0;
    m_init_odr = odr_createmem (ODR_DECODE);
    m_initResponse = 0;
    m_resultSetStartPoint = 0;
    m_bytes_sent = m_bytes_recv = 0;
}

const char *Yaz_Proxy::option(const char *name, const char *value)
{
    if (!strcmp (name, "optimize")) {
	if (value) {
            xfree (m_optimize);	
	    m_optimize = xstrdup (value);
        }
	return m_optimize;
    }
    return 0;
}

void Yaz_ProxyClient::recv_Z_PDU(Z_APDU *apdu, int len)
{
    m_bytes_recv += len;
    m_waiting = 0;
    yaz_log (LOG_LOG, "%s Receiving %s from %s %d bytes", get_session_str(),
	     apdu_name(apdu), get_hostname(), len);
    if (apdu->which == Z_APDU_initResponse)
    {
        NMEM nmem = odr_extract_mem (odr_decode());
	odr_reset (m_init_odr);
        nmem_transfer (m_init_odr->mem, nmem);
        m_initResponse = apdu;

	Z_InitResponse *ir = apdu->u.initResponse;
	char *im0 = ir->implementationName;
	
	char *im1 = (char*) 
	    odr_malloc(m_init_odr, 20 + (im0 ? strlen(im0) : 0));
	*im1 = '\0';
	if (im0)
	{
	    strcat(im1, im0);
	    strcat(im1, " ");
	}
	strcat(im1, "(YAZ Proxy)");
	ir->implementationName = im1;

        nmem_destroy (nmem);
    }
    if (apdu->which == Z_APDU_searchResponse)
    {
	Z_SearchResponse *sr = apdu->u.searchResponse;
	m_last_resultCount = *sr->resultCount;
	int status = *sr->searchStatus;
	if (status && (!sr->records || sr->records->which == Z_Records_DBOSD))
	{
            m_last_ok = 1;
	    
	    if (sr->records && sr->records->which == Z_Records_DBOSD)
	    {
		m_cache.add(odr_decode(),
			    sr->records->u.databaseOrSurDiagnostics, 1,
			    *sr->resultCount);
	    }
	}
    }
    if (apdu->which == Z_APDU_presentResponse)
    {
	Z_PresentResponse *pr = apdu->u.presentResponse;
	if (m_sr_transform)
	{
	    m_sr_transform = 0;
	    Z_APDU *new_apdu = create_Z_PDU(Z_APDU_searchResponse);
	    Z_SearchResponse *sr = new_apdu->u.searchResponse;
	    sr->referenceId = pr->referenceId;
	    *sr->resultCount = m_last_resultCount;
	    sr->records = pr->records;
	    sr->nextResultSetPosition = pr->nextResultSetPosition;
	    sr->numberOfRecordsReturned = pr->numberOfRecordsReturned;
	    apdu = new_apdu;
	}
	if (pr->records && 
	    pr->records->which == Z_Records_DBOSD && m_resultSetStartPoint)
	{
	    m_cache.add(odr_decode(),
			pr->records->u.databaseOrSurDiagnostics,
			m_resultSetStartPoint, -1);
	    m_resultSetStartPoint = 0;
	}
    }
    if (m_cookie)
	set_otherInformationString (apdu, VAL_COOKIE, 1, m_cookie);
    if (m_server)
    {
	m_server->send_to_client(apdu);
    }
    if (apdu->which == Z_APDU_close)
    {
	shutdown();
    }
}
