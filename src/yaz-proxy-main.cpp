/*
 * Copyright (c) 1998-2001, Index Data.
 * See the file LICENSE for details.
 * 
 * $Log: yaz-proxy-main.cpp,v $
 * Revision 1.13  2001-04-10 10:48:08  adam
 * Fixed problem where proxy could cash bad result sets.
 *
 * Revision 1.12  2000/10/11 11:58:16  adam
 * Moved header files to include/yaz++. Switched to libtool and automake.
 * Configure script creates yaz++-config script.
 *
 * Revision 1.11  2000/09/08 10:23:42  adam
 * Added skeleton of yaz-z-server.
 *
 * Revision 1.10  2000/09/04 08:59:16  adam
 * Changed call to logging functions (yaz_ added).
 *
 * Revision 1.9  2000/08/07 14:19:59  adam
 * Fixed serious bug regarding timeouts. Improved logging for proxy.
 *
 * Revision 1.8  2000/07/04 13:48:49  adam
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

#include <yaz++/yaz-socket-manager.h>
#include <yaz++/yaz-pdu-assoc.h>
#include <yaz++/yaz-proxy.h>

void usage(char *prog)
{
    fprintf (stderr, "%s: [-a log] [-c num] [-v level] [-t target] @:port\n", prog);
    exit (1);
}


int args(Yaz_Proxy *proxy, int argc, char **argv)
{
    char *addr = 0;
    char *arg;
    char *prog = argv[0];
    int ret;

    while ((ret = options("a:t:v:c:", argv, argc, &arg)) != -2)
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
	case 'a':
	    proxy->set_APDU_log(arg);
	    break;
        case 't':
	    proxy->set_proxyTarget(arg);
	    break;
	case 'v':
	    yaz_log_init_level (yaz_log_mask_str(arg));
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
    exit (0);
    return 0;
}
