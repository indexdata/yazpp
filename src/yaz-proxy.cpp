/*
 * Copyright (c) 1998-1999, Index Data.
 * See the file LICENSE for details.
 * Sebastian Hammer, Adam Dickmeiss
 * 
 * $Log: yaz-proxy.cpp,v $
 * Revision 1.2  1999-01-28 13:08:46  adam
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
    Yaz_IR_Assoc(the_PDU_Observable)
{
    m_PDU_Observable = the_PDU_Observable;
    m_maps = 0;
    m_client = 0;
}

Yaz_Proxy::~Yaz_Proxy()
{
}

IYaz_PDU_Observer *Yaz_Proxy::clone(IYaz_PDU_Observable
				    *the_PDU_Observable)
{
    return new Yaz_Proxy(the_PDU_Observable);
}

void Yaz_Proxy::recv_Z_PDU(Z_APDU *apdu)
{
    if (apdu->which == Z_APDU_initRequest)
    {
	assert (m_client == 0);
        logf (LOG_LOG, "got InitRequest");
	m_client = new Yaz_ProxyClient(m_PDU_Observable->clone());
	m_client->m_server = this;
	m_client->client("localhost:8888");
    }
    assert (m_client);
    logf (LOG_LOG, "sending PDU");
    m_client->send_Z_PDU(apdu);
}

void Yaz_Proxy::failNotify()
{
    logf (LOG_LOG, "failNotity server");
    delete m_client;
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

Yaz_ProxyClient::Yaz_ProxyClient(IYaz_PDU_Observable *the_PDU_Observable) :
    Yaz_IR_Assoc (the_PDU_Observable)
{
    
}

void Yaz_ProxyClient::recv_Z_PDU(Z_APDU *apdu)
{
    m_server->send_Z_PDU(apdu);
}
