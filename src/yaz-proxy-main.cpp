/*
 * Copyright (c) 1998-1999, Index Data.
 * See the file LICENSE for details.
 * Sebastian Hammer, Adam Dickmeiss
 * 
 * $Log: yaz-proxy-main.cpp,v $
 * Revision 1.4  1999-04-09 11:46:57  adam
 * Added object Yaz_Z_Assoc. Much more functional client.
 *
 * Revision 1.3  1999/02/02 14:01:21  adam
 * First WIN32 port of YAZ++.
 *
 * Revision 1.2  1999/01/28 13:08:45  adam
 * Yaz_PDU_Assoc better encapsulated. Memory leak fix in
 * yaz-socket-manager.cc.
 *
 * Revision 1.1.1.1  1999/01/28 09:41:07  adam
 * First implementation of YAZ++.
 *
 */

#include <log.h>

#include <yaz-socket-manager.h>
#include <yaz-pdu-assoc.h>
#include <yaz-proxy.h>

int main(int argc, char **argv)
{
    Yaz_SocketManager mySocketManager;
    Yaz_Proxy proxy(new Yaz_PDU_Assoc(&mySocketManager, 0));

    proxy.server(argc < 2 ? "@:9999" : argv[1]);
    while (mySocketManager.processEvent() > 0)
	;
    return 0;
}
