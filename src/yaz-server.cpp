/*
 * Copyright (c) 1998-1999, Index Data.
 * See the file LICENSE for details.
 * Sebastian Hammer, Adam Dickmeiss
 * 
 * $Log: yaz-server.cpp,v $
 * Revision 1.1.1.1  1999-01-28 09:41:07  adam
 * First implementation of YAZ++.
 *
 */

#include <log.h>
#include <yaz-ir-assoc.h>
#include <yaz-pdu-assoc.h>
#include <yaz-socket-manager.h>

class MyServer : public Yaz_IR_Assoc {
public:
    MyServer(IYaz_PDU_Observable *the_PDU_Observable);
    void recv_Z_PDU(Z_APDU *apdu);
    IYaz_PDU_Observer* clone(IYaz_PDU_Observable *the_PDU_Observable);
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
	stop = 1;
        break;
    }
}

IYaz_PDU_Observer *MyServer::clone(IYaz_PDU_Observable *the_PDU_Observable)
{
    return new MyServer(the_PDU_Observable);
}

MyServer::MyServer(IYaz_PDU_Observable *the_PDU_Observable) :
    Yaz_IR_Assoc (the_PDU_Observable)
{

}

int main(int argc, char **argv)
{
    Yaz_SocketManager mySocketManager;

    Yaz_PDU_Assoc my_PDU_Assoc(&mySocketManager, 0);
    MyServer z(&my_PDU_Assoc);

    z.server("@:9999");

    while (!stop && mySocketManager.processEvent() > 0)
	;
}
