/*
 * Copyright (c) 2000-2001, Index Data.
 * See the file LICENSE for details.
 * 
 * $Log: yaz-z-server.cpp,v $
 * Revision 1.9  2001-03-29 15:14:26  adam
 * Minor updates.
 *
 * Revision 1.8  2001/03/27 14:47:45  adam
 * New server facility scheme.
 *
 * Revision 1.7  2001/03/26 14:43:49  adam
 * New threaded PDU association.
 *
 * Revision 1.6  2001/01/29 11:18:24  adam
 * Server sets OPTIONS search and present.
 *
 * Revision 1.5  2000/10/24 12:29:57  adam
 * Fixed bug in proxy where a Yaz_ProxyClient could be owned by
 * two Yaz_Proxy's (fatal).
 *
 * Revision 1.4  2000/10/11 11:58:17  adam
 * Moved header files to include/yaz++. Switched to libtool and automake.
 * Configure script creates yaz++-config script.
 *
 * Revision 1.3  2000/09/21 21:43:20  adam
 * Better high-level server API.
 *
 * Revision 1.2  2000/09/12 12:09:53  adam
 * More work on high-level server.
 *
 * Revision 1.1  2000/09/08 10:23:42  adam
 * Added skeleton of yaz-z-server.
 *
 */

#include <yaz/log.h>
#include <yaz++/yaz-z-server.h>

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

void Yaz_Z_Server::recv_Z_PDU (Z_APDU *apdu_request)
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
	send_Z_PDU(apdu_response);
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
	    yaz_log (LOG_LOG, "got request = %d", apdu_request->which);
	    delete this;
	}
    }
}
