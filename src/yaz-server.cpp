/*
 * Copyright (c) 1998-2000, Index Data.
 * See the file LICENSE for details.
 * 
 * $Log: yaz-server.cpp,v $
 * Revision 1.10  2000-09-12 16:04:17  adam
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
#include <yaz-z-server.h>
#include <yaz-pdu-assoc.h>
#include <yaz-socket-manager.h>

class MyServer : public Yaz_Z_Server {
public:
    MyServer(IYaz_PDU_Observable *the_PDU_Observable);
    void recv_Z_init (Z_InitRequest *initRequest,
		      Z_InitResponse *initResponse);
    void recv_Z_search (Z_SearchRequest *searchRequest,
			Z_SearchResponse *searchResponse);
    void recv_Z_present (Z_PresentRequest *presentRequest,
			 Z_PresentResponse *presentResponse);

    void recv_Z_record (const char *resultSetName,
			int position,
			int *format,
			Z_RecordComposition *comp,
			      Z_NamePlusRecord *namePlusRecord,
			Z_DefaultDiagFormat *diagnostics);
    IYaz_PDU_Observer* clone(IYaz_PDU_Observable *the_PDU_Observable);
    void failNotify();
    void timeoutNotify();
    void connectNotify();
private:
    int m_no;
};

static int stop = 0;

void MyServer::recv_Z_init (Z_InitRequest *initRequest,
			    Z_InitResponse *initResponse)
{
    logf (LOG_LOG, "MyServer::recv_Z_init");
}

void MyServer::recv_Z_search (Z_SearchRequest *searchRequest,
			      Z_SearchResponse *searchResponse)
{
    logf (LOG_LOG, "MyServer::recv_Z_search");
    delete this;
    stop = 1;
}

void MyServer::recv_Z_present (Z_PresentRequest *presentRequest,
			       Z_PresentResponse *presentResponse)
{
    logf (LOG_LOG, "MyServer::recv_Z_present");
}

void MyServer::recv_Z_record (const char *resultSetName,
			      int position,
			      int *format,
			      Z_RecordComposition *comp,
			      Z_NamePlusRecord *namePlusRecord,
			      Z_DefaultDiagFormat *diagnostics)
{

}

IYaz_PDU_Observer *MyServer::clone(IYaz_PDU_Observable *the_PDU_Observable)
{
    MyServer *new_server;
    logf (LOG_LOG, "child no %d", m_no);
    m_no++;
    new_server = new MyServer(the_PDU_Observable);
    new_server->timeout(60);
    return new_server;
}

MyServer::MyServer(IYaz_PDU_Observable *the_PDU_Observable) :
    Yaz_Z_Server (the_PDU_Observable)
{
    m_no = 0;
}

void MyServer::timeoutNotify()
{
    logf (LOG_LOG, "connection timed out");
    delete this;
}

void MyServer::failNotify()
{
    logf (LOG_LOG, "connection closed by client");
    delete this;
}

void MyServer::connectNotify()
{
}

int main(int argc, char **argv)
{
    while (1)
    {
	stop = 0;
	Yaz_SocketManager mySocketManager;
	Yaz_PDU_Assoc *my_PDU_Assoc = new Yaz_PDU_Assoc(&mySocketManager);
	
	MyServer *z = new MyServer(my_PDU_Assoc);
	
	if (argc <= 1)
	    z->server("@:9999");
	else
	{
	    for (int i = 1; i < argc; i++)
		z->server(argv[i]);
	}
	COMSTACK cs = my_PDU_Assoc->comstack();
	if (cs)
	    printf ("fd=%d\n", cs_fileno(cs));
	while (!stop && mySocketManager.processEvent() > 0)
	    ;
	logf (LOG_LOG, "bailing out");
	delete z;
    }
    return 0;
}
