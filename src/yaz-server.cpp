/*
 * Copyright (c) 1998-1999, Index Data.
 * See the file LICENSE for details.
 * Sebastian Hammer, Adam Dickmeiss
 * 
 * $Log: yaz-server.cpp,v $
 * Revision 1.6  1999-04-21 12:09:01  adam
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

#include <log.h>
#include <yaz-z-assoc.h>
#include <yaz-pdu-assoc.h>
#include <yaz-socket-manager.h>

class MyServer : public Yaz_Z_Assoc {
public:
    MyServer(IYaz_PDU_Observable *the_PDU_Observable);
    void recv_Z_PDU(Z_APDU *apdu);
    IYaz_PDU_Observer* clone(IYaz_PDU_Observable *the_PDU_Observable);
    void failNotify();
    void timeoutNotify();
private:
    int m_no;
};

static int stop = 0;

void MyServer::recv_Z_PDU(Z_APDU *apdu)
{
    logf (LOG_LOG, "recv_Z_PDU");
    switch (apdu->which)
    {
    case Z_APDU_initRequest:
        logf (LOG_LOG, "got InitRequest");
	apdu = create_Z_PDU(Z_APDU_initResponse);
	send_Z_PDU(apdu);
        break;
    case Z_APDU_searchRequest:
        logf (LOG_LOG, "got searchRequest");
	apdu = create_Z_PDU(Z_APDU_searchResponse);
	send_Z_PDU(apdu);
        break;
    case Z_APDU_presentRequest:
        logf (LOG_LOG, "got presentRequest");
	apdu = create_Z_PDU(Z_APDU_presentResponse);
	send_Z_PDU(apdu);
	// stop = 1;
        break;
    }
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
    Yaz_Z_Assoc (the_PDU_Observable)
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

int main(int argc, char **argv)
{
    Yaz_SocketManager mySocketManager;
    Yaz_PDU_Assoc *my_PDU_Assoc = new Yaz_PDU_Assoc(&mySocketManager, 0);

    MyServer z(my_PDU_Assoc);

    if (argc <= 1)
	z.server("@:9999");
    else
    {
	for (int i = 1; i < argc; i++)
	    z.server(argv[i]);
    }
    while (!stop && mySocketManager.processEvent() > 0)
	;
    return 0;
}
