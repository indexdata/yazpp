/*
 * Copyright (c) 1998-2001, Index Data.
 * See the file LICENSE for details.
 * 
 * $Id: yaz-proxy-main.cpp,v 1.16 2002-01-14 12:01:28 adam Exp $
 */

#include <yaz/log.h>
#include <yaz/options.h>

#include <yaz++/yaz-socket-manager.h>
#include <yaz++/yaz-pdu-assoc.h>
#include <yaz++/yaz-proxy.h>

void usage(char *prog)
{
    fprintf (stderr, "%s: [-a log] [-c num] [-v level] [-t target] "
             "[-u auth] [-o optlevel] @:port\n", prog);
    exit (1);
}


int args(Yaz_Proxy *proxy, int argc, char **argv)
{
    char *addr = 0;
    char *arg;
    char *prog = argv[0];
    int ret;

    while ((ret = options("o:a:t:v:c:u:", argv, argc, &arg)) != -2)
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
	    proxy->set_proxy_target(arg);
	    break;
        case 'u':
            proxy->set_proxy_authentication(arg);
            break;
        case 'o':
	    proxy->option("optimize", arg);
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
