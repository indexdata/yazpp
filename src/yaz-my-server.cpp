/*
 * Copyright (c) 1998-2001, Index Data.
 * See the file LICENSE for details.
 * 
 * $Log: yaz-my-server.cpp,v $
 * Revision 1.5  2001-05-03 12:39:39  adam
 * Added Update server service.
 *
 * Revision 1.4  2001/04/05 13:09:44  adam
 * Removed ursula dependancy.
 *
 * Revision 1.3  2001/04/04 14:02:49  adam
 * URSULA / Z-ruth service.
 *
 * Revision 1.2  2001/03/29 15:14:26  adam
 * Minor updates.
 *
 * Revision 1.1  2001/03/27 14:47:45  adam
 * New server facility scheme.
 *
 * Revision 1.15  2001/03/26 14:43:49  adam
 * New threaded PDU association.
 *
 * Revision 1.14  2000/11/01 14:22:59  adam
 * Added fd parameter for method IYaz_PDU_Observer::clone.
 *
 * Revision 1.13  2000/10/11 11:58:16  adam
 * Moved header files to include/yaz++. Switched to libtool and automake.
 * Configure script creates yaz++-config script.
 *
 * Revision 1.12  2000/09/21 21:43:20  adam
 * Better high-level server API.
 *
 * Revision 1.11  2000/09/12 16:23:49  adam
 * Updated server example.
 *
 * Revision 1.10  2000/09/12 16:04:17  adam
 * Added comstack method for Yaz_PDU_Assoc..
 *
 * Revision 1.9  2000/09/12 12:09:53  adam
 * More work on high-level server.
 *
 * Revision 1.8  2000/09/08 10:23:42  adam
 * Added skeleton of yaz-z-server.
 *
 * Revision 1.7  1999/12/06 13:52:45  adam
 * Modified for new location of YAZ header files. Experimental threaded
 * operation.
 *
 * Revision 1.6  1999/04/21 12:09:01  adam
 * Many improvements. Modified to proxy server to work with "sessions"
 * based on cookies.
 *
 * Revision 1.5  1999/04/09 11:46:57  adam
 * Added object Yaz_Z_Assoc. Much more functional client.
 *
 * Revision 1.4  1999/03/23 14:17:57  adam
 * More work on timeout handling. Work on yaz-client.
 *
 * Revision 1.3  1999/02/02 14:01:22  adam
 * First WIN32 port of YAZ++.
 *
 * Revision 1.2  1999/01/28 13:08:47  adam
 * Yaz_PDU_Assoc better encapsulated. Memory leak fix in
 * yaz-socket-manager.cc.
 *
 * Revision 1.1.1.1  1999/01/28 09:41:07  adam
 * First implementation of YAZ++.
 *
 */

#include <yaz/log.h>
#include <yaz/options.h>
#include <yaz++/yaz-z-server.h>
#include <yaz++/yaz-pdu-assoc.h>
#include <yaz++/yaz-socket-manager.h>

class MyILL : public Yaz_Facility_ILL {
public:
    void ill_service (Z_ExtendedServicesRequest *req,
		      Z_ItemOrder *io,
		      Z_ExtendedServicesResponse *res);
};

class MyUpdate : public Yaz_Facility_Update {
public:
    void update_service (Z_ExtendedServicesRequest *req,
			 Z_IUUpdate *io,
			 Z_ExtendedServicesResponse *res);
};


class MyRetrieval : public Yaz_Facility_Retrieval, Yaz_USMARC {
public:
    int sr_init (Z_InitRequest *initRequest,
		 Z_InitResponse *initResponse);
    void sr_search (Z_SearchRequest *searchRequest,
			Z_SearchResponse *searchResponse);
    void sr_present (Z_PresentRequest *presentRequest,
			 Z_PresentResponse *presentResponse);
    void sr_record (const char *resultSetName,
		    int position,
		    int *format,
		    Z_RecordComposition *comp,
		    Z_NamePlusRecord *namePlusRecord,
		    Z_Records *records);
};

#if HAVE_YAZ_URSULA_H
class MyUrsula : public Yaz_Facility_Ursula {
public:
    void ursula_service (Z_ExtendedServicesRequest *req,
			 Z_UrsPDU *u,
			 Z_ExtendedServicesResponse *res);
};
#endif

class MyServer : public Yaz_Z_Server {
public:
    ~MyServer();
    MyServer(IYaz_PDU_Observable *the_PDU_Observable);
    IYaz_PDU_Observer* sessionNotify(IYaz_PDU_Observable *the_PDU_Observable,
				     int fd);
    void failNotify();
    void timeoutNotify();
    void connectNotify();

private:
    MyRetrieval m_retrieval;
    MyILL       m_ill;
    MyUpdate    m_update;
#if HAVE_YAZ_URSULA_H
    MyUrsula    m_ursula;
#endif
    int m_no;
};

void MyILL::ill_service (Z_ExtendedServicesRequest *req,
			 Z_ItemOrder *io,
			 Z_ExtendedServicesResponse *res)
{
    yaz_log (LOG_LOG, "MyServer::ill_service");
}

void MyUpdate::update_service (Z_ExtendedServicesRequest *req,
			   Z_IUUpdate *io,
			   Z_ExtendedServicesResponse *res)
{
    yaz_log (LOG_LOG, "MyServer::update_service");
}


#if HAVE_YAZ_URSULA_H
void MyUrsula::ursula_service (Z_ExtendedServicesRequest *req,
			       Z_UrsPDU *u,
			       Z_ExtendedServicesResponse *res)
{
    yaz_log (LOG_LOG, "MyServer::ursula_service");
    switch (u->which)
    {
    case  Z_UrsPDU_request:
	yaz_log(LOG_LOG, "request");
	if (u->u.request->libraryNo)
	    yaz_log (LOG_LOG, "libraryNo: %s", u->u.request->libraryNo);
	break;
    case  Z_UrsPDU_update:
	yaz_log(LOG_LOG, "request");
	break;
    case  Z_UrsPDU_reservation:
	yaz_log(LOG_LOG, "request");
	break;
    case  Z_UrsPDU_renewal:
	yaz_log(LOG_LOG, "request");
	break;
    default:
	yaz_log(LOG_LOG, "unknown");
	break;
    }
}
#endif

int MyRetrieval::sr_init (Z_InitRequest *initRequest,
		       Z_InitResponse *initResponse)
{
    yaz_log (LOG_LOG, "MyServer::sr_init");
    return 1;
}

void MyRetrieval::sr_search (Z_SearchRequest *searchRequest,
			     Z_SearchResponse *searchResponse)
{
    yaz_log (LOG_LOG, "MyServer::recv_Z_search");
    if (searchRequest->query->which == Z_Query_type_1)
    {
	Z_RPNStructure *s = searchRequest->query->u.type_1->RPNStructure;
	if (s->which == Z_RPNStructure_simple &&
	    s->u.simple->which == Z_Operand_APT &&
	    s->u.simple->u.attributesPlusTerm->term->which == Z_Term_general)
	{
	    Odr_oct *term = s->u.simple->u.attributesPlusTerm->term->u.general;
	    char *str = (char *) odr_malloc (odr_encode(), term->len+1);
	    if (term->len)
		memcpy (str, term->buf, term->len);
	    str[term->len] = '\0';
	    *searchResponse->resultCount = atoi(str);
	}
    }
}

void MyRetrieval::sr_present (Z_PresentRequest *presentRequest,
			       Z_PresentResponse *presentResponse)
{
    yaz_log (LOG_LOG, "MyServer::recv_Z_present");
}

void MyRetrieval::sr_record (const char *resultSetName,
			     int position,
			     int *format,
			     Z_RecordComposition *comp,
			     Z_NamePlusRecord *namePlusRecord,
			     Z_Records *records)
{
    yaz_log (LOG_LOG, "MyServer::recv_Z_record");
    const char *rec = get_record(position);
    create_databaseRecord (odr_encode(), namePlusRecord, 0, VAL_USMARC, rec,
			   strlen(rec));
}

MyServer::~MyServer()
{
}

IYaz_PDU_Observer *MyServer::sessionNotify(
    IYaz_PDU_Observable *the_PDU_Observable, int fd)
{
    MyServer *new_server;
    m_no++;
    new_server = new MyServer(the_PDU_Observable);
    new_server->timeout(900);
    new_server->facility_add(&new_server->m_retrieval, "my sr");
    new_server->facility_add(&new_server->m_ill, "my ill");
    new_server->facility_add(&new_server->m_update, "my update");
#if HAVE_YAZ_URSULA_H
    new_server->facility_add(&new_server->m_ursula, "my ursula");
#endif

    new_server->set_APDU_log(get_APDU_log());

    return new_server;
}

MyServer::MyServer(IYaz_PDU_Observable *the_PDU_Observable) :
    Yaz_Z_Server (the_PDU_Observable)
{
    m_no = 0;
}

void MyServer::timeoutNotify()
{
    yaz_log (LOG_LOG, "connection timed out");
    delete this;
}

void MyServer::failNotify()
{
    yaz_log (LOG_LOG, "connection closed by client");
    delete this;
}

void MyServer::connectNotify()
{
}


void usage(char *prog)
{
    fprintf (stderr, "%s: [-a log] [-v level] [-T] @:port\n", prog);
    exit (1);
}

int main(int argc, char **argv)
{
    int thread_flag = 0;
    char *arg;
    char *prog = *argv;
    char *addr = "tcp:@:9999";
    char *apdu_log = 0;
    
    Yaz_SocketManager mySocketManager;
    
    Yaz_PDU_Assoc *my_PDU_Assoc = 0;
    
    MyServer *z = 0;
    int ret;
    
    while ((ret = options("a:v:T", argv, argc, &arg)) != -2)
    {
	switch (ret)
	{
	case 0:
	    addr = xstrdup(arg);
	    break;
	case 'a':
	    apdu_log = xstrdup(arg);
	    break;
	case 'v':
	    yaz_log_init_level (yaz_log_mask_str(arg));
	    break;
	case 'T':
	    thread_flag = 1;
	    break;
	default:
	    usage(prog);
	    return 1;
	}
    }
    if (thread_flag)
	my_PDU_Assoc = new Yaz_PDU_AssocThread(&mySocketManager);
    else
	my_PDU_Assoc = new Yaz_PDU_Assoc(&mySocketManager);
    
    z = new MyServer(my_PDU_Assoc);
    z->server(addr);
    if (apdu_log)
    {
	yaz_log (LOG_LOG, "set_APDU_log %s", apdu_log);
	z->set_APDU_log(apdu_log);
    }

    while (mySocketManager.processEvent() > 0)
	;
    delete z;
    return 0;
}
