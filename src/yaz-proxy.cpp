/*
 * Copyright (c) 1998-1999, Index Data.
 * See the file LICENSE for details.
 * Sebastian Hammer, Adam Dickmeiss
 * 
 * $Log: yaz-proxy.cpp,v $
 * Revision 1.5  1999-04-21 12:09:01  adam
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
	(oi = update_otherInformation(otherInfo, 0, oid, 1)) &&
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
	(oi = update_otherInformation(otherInfo, 0, oid, 1)) &&
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
	  "<null>");
    if (cookie)
    {
	for (c = parent->m_clientPool; c; c = c->m_next)
	{
	    assert (c->m_prev);
	    assert (*c->m_prev == c);
	    if (!strcmp(cookie,c->m_cookie))
	    {
		logf (LOG_LOG, "Yaz_Proxy::get_client cached");
		break;
	    }
	}
    }
    else if (!m_client)
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

    Z_OtherInformation **oi;
    get_otherInfoAPDU(apdu, &oi);
    *oi = 0;
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
	// Tell client (if any) that not server connection is there..
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
}

void Yaz_ProxyClient::recv_Z_PDU(Z_APDU *apdu)
{
    logf (LOG_LOG, "Yaz_ProxyClient::recv_Z_PDU");
    if (m_cookie)
	set_otherInformationString (apdu, VAL_COOKIE, 1, m_cookie);
    if (m_server)
    {
	logf (LOG_LOG, "Yaz_Proxy::send_Z_PDU");
	m_server->send_Z_PDU(apdu);
    }
}
