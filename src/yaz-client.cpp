/*
 * Copyright (c) 1998-1999, Index Data.
 * See the file LICENSE for details.
 * Sebastian Hammer, Adam Dickmeiss
 * 
 * $Log: yaz-client.cpp,v $
 * Revision 1.1.1.1  1999-01-28 09:41:07  adam
 * First implementation of YAZ++.
 *
 */

#include <log.h>
#include <yaz-ir-assoc.h>
#include <yaz-pdu-assoc.h>
#include <yaz-socket-manager.h>

class MyClient : public Yaz_IR_Assoc {
public:
    MyClient(IYaz_PDU_Observable *the_PDU_Observable);
    void recv_Z_PDU(Z_APDU *apdu);
    IYaz_PDU_Observer *clone(IYaz_PDU_Observable *the_PDU_Observable);
    void sendInit();
};

void MyClient::recv_Z_PDU(Z_APDU *apdu)
{
    logf (LOG_LOG, "recv_APDU");
    switch (apdu->which)
    {
    case Z_APDU_initResponse:
        logf (LOG_LOG, "got InitResponse");
        break;
    case Z_APDU_searchResponse:
        logf (LOG_LOG, "got searchResponse");
        break;
    }
}

IYaz_PDU_Observer *MyClient::clone(IYaz_PDU_Observable *the_PDU_Observable)
{
    return new MyClient(the_PDU_Observable);
}

MyClient::MyClient(IYaz_PDU_Observable *the_PDU_Observable) :
    Yaz_IR_Assoc (the_PDU_Observable)
{

}

void MyClient::sendInit()
{
    Z_APDU *apdu = create_Z_PDU(Z_APDU_initRequest);
    Z_InitRequest *req = apdu->u.initRequest;
    
    ODR_MASK_SET(req->options, Z_Options_search);
    ODR_MASK_SET(req->options, Z_Options_present);
    ODR_MASK_SET(req->options, Z_Options_namedResultSets);
    ODR_MASK_SET(req->options, Z_Options_triggerResourceCtrl);
    ODR_MASK_SET(req->options, Z_Options_scan);
    ODR_MASK_SET(req->options, Z_Options_sort);
    ODR_MASK_SET(req->options, Z_Options_extendedServices);
    ODR_MASK_SET(req->options, Z_Options_delSet);

    ODR_MASK_SET(req->protocolVersion, Z_ProtocolVersion_1);
    ODR_MASK_SET(req->protocolVersion, Z_ProtocolVersion_2);
    ODR_MASK_SET(req->protocolVersion, Z_ProtocolVersion_3);

    send_Z_PDU(apdu);
}

int main(int argc, char **argv)
{
    Yaz_SocketManager mySocketManager;

    Yaz_PDU_Assoc my_PDU_Assoc(&mySocketManager, 0);
    MyClient z(&my_PDU_Assoc);

    z.client("localhost:9999");
    z.sendInit();
    
    while (mySocketManager.processEvent() > 0)
	;
}
