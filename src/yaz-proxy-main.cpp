/*
 * Copyright (c) 1998-1999, Index Data.
 * See the file LICENSE for details.
 * Sebastian Hammer, Adam Dickmeiss
 * 
 * $Log: yaz-proxy-main.cpp,v $
 * Revision 1.1  1999-01-28 09:41:07  adam
 * Initial revision
 *
 */

#include <yaz-socket-manager.h>
#include <yaz-pdu-assoc.h>
#include <yaz-proxy.h>

int main(int argc, char **argv)
{
    Yaz_SocketManager mySocketManager;
    Yaz_PDU_Assoc *my_PDU_Assoc = new Yaz_PDU_Assoc(&mySocketManager, 0);
    Yaz_Proxy proxy(my_PDU_Assoc);
    
    proxy.server("@:9999");
    while (mySocketManager.processEvent() > 0)
	;
    return 0;
}
