/*
 * Copyright (c) 1998-1999, Index Data.
 * See the file LICENSE for details.
 * Sebastian Hammer, Adam Dickmeiss
 * 
 * $Log: yaz-proxy-main.cpp,v $
 * Revision 1.8  2000-07-04 13:48:49  adam
 * Implemented upper-limit on proxy-to-target sessions.
 *
 * Revision 1.7  1999/12/06 13:52:45  adam
 * Modified for new location of YAZ header files. Experimental threaded
 * operation.
 *
 * Revision 1.6  1999/11/10 10:02:34  adam
 * Work on proxy.
 *
 * Revision 1.5  1999/04/21 12:09:01  adam
 * Many improvements. Modified to proxy server to work with "sessions"
 * based on cookies.
 *
 * Revision 1.4  1999/04/09 11:46:57  adam
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

#include <yaz/log.h>
#include <yaz/options.h>

#include <yaz-socket-manager.h>
#include <yaz-pdu-assoc.h>
#include <yaz-proxy.h>

void usage(char *prog)
{
    fprintf (stderr, "%s: [-c num] [-v log] [-t target] @:port\n", prog);
    exit (1);
}


int args(Yaz_Proxy *proxy, int argc, char **argv)
{
    char *addr = 0;
    char *arg;
    char *prog = argv[0];
    int ret;

    while ((ret = options("t:v:c:", argv, argc, &arg)) != -2)
    {
        switch (ret)
        {
        case 0:
            if (addr)
	    {
		usage(prog);
		return 1;
	    }
	    addr = arg;
            break;
        case 't':
	    proxy->set_proxyTarget(arg);
	    break;
	case 'v':
	    log_init_level (log_mask_str(arg));
	    break;
	case 'c':
	    proxy->set_max_clients(atoi(arg));
	    break;
        default:
	    usage(prog);
	    return 1;
        }
    }
    if (addr)
    {
	proxy->server(addr);
    }
    else
    {
	usage(prog);
	return 1;
    }
    return 0;
}

int main(int argc, char **argv)
{
    Yaz_SocketManager mySocketManager;
    Yaz_Proxy proxy(new Yaz_PDU_Assoc(&mySocketManager));

    args(&proxy, argc, argv);
    while (mySocketManager.processEvent() > 0)
	;
    return 0;
}
