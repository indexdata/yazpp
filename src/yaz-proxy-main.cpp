/*
 * Copyright (c) 1998-2003, Index Data.
 * See the file LICENSE for details.
 * 
 * $Id: yaz-proxy-main.cpp,v 1.25 2003-10-23 13:49:58 adam Exp $
 */

#include <signal.h>
#include <unistd.h>
#include <pwd.h>
#include <sys/types.h>

#include <yaz/log.h>
#include <yaz/options.h>

#include <yaz++/socket-manager.h>
#include <yaz++/pdu-assoc.h>
#include <yaz++/proxy.h>

void usage(char *prog)
{
    fprintf (stderr, "%s: [-c config] [-a log] [-m num] [-v level] [-t target] [-i sec] "
             "[-u uid] [-p pidfile] [-o optlevel] @:port\n", prog);
    exit (1);
}

static char *pid_fname = 0;
static char *uid = 0;
static char *log_file = 0;

int args(Yaz_Proxy *proxy, int argc, char **argv)
{
    char *addr = 0;
    char *arg;
    char *prog = argv[0];
    int ret;

    while ((ret = options("o:a:t:v:c:u:i:m:l:T:p:U:", argv, argc, &arg)) != -2)
    {
	int err;
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
	case 'c':
	    err = proxy->set_config(arg);
	    if (err == -2)
	    {
		fprintf(stderr, "Config file support not enabled (proxy not compiled with libxml2 support)\n");
		exit(1);
	    }
	    else if (err == -1)
	    {
		fprintf(stderr, "Bad or missing file %s\n", arg);
		exit(1);
	    }
	    break;
	case 'a':
	    proxy->set_APDU_log(arg);
	    break;
        case 't':
	    proxy->set_default_target(arg);
	    break;
        case 'U':
            proxy->set_proxy_authentication(arg);
            break;
        case 'o':
	    proxy->option("optimize", arg);
	    break;
	case 'v':
	    yaz_log_init_level (yaz_log_mask_str(arg));
	    break;
	case 'l':
	    yaz_log_init_file (arg);
	    log_file = xstrdup(arg);
	    break;
	case 'm':
	    proxy->set_max_clients(atoi(arg));
	    break;
        case 'i':
	    proxy->set_client_idletime(atoi(arg));
	    break;
        case 'T':
	    proxy->set_target_idletime(atoi(arg));
	    break;
	case 'p':
	    if (!pid_fname)
		pid_fname = xstrdup(arg);
	    break;
	case 'u':
	    if (!uid)
		uid = xstrdup(arg);
	    break;
        default:
	    usage(prog);
	    return 1;
        }
    }
    if (addr)
    {
	yaz_log(LOG_LOG, "Starting proxy pid=%ld", (long) getpid());
	if (proxy->server(addr))
	{
	    yaz_log(LOG_FATAL|LOG_ERRNO, "listen %s", addr);
	    exit(1);
	}
    }
    else
    {
	usage(prog);
	return 1;
    }
    return 0;
}

static Yaz_Proxy *static_yaz_proxy = 0;
static void sighup_handler(int num)
{
    signal(SIGHUP, sighup_handler);
    if (static_yaz_proxy)
	static_yaz_proxy->reconfig();
}

int main(int argc, char **argv)
{
    static int mk_pid = 0;
    Yaz_SocketManager mySocketManager;
    Yaz_Proxy proxy(new Yaz_PDU_Assoc(&mySocketManager));

    static_yaz_proxy = &proxy;

    signal(SIGHUP, sighup_handler);

    args(&proxy, argc, argv);

    if (pid_fname)
    {
	FILE *f = fopen(pid_fname, "w");
	if (!f)
	{
	    yaz_log(LOG_ERRNO|LOG_FATAL, "Couldn't create %s", pid_fname);
	    exit(0);
	}
	fprintf(f, "%ld", (long) getpid());
	fclose(f);
	xfree(pid_fname);
    }
    if (uid)
    {
    	struct passwd *pw;

	if (!(pw = getpwnam(uid)))
	{
	    yaz_log(LOG_FATAL, "%s: Unknown user", uid);
	    exit(3);
	}
	if (log_file)
	{
	    chown(log_file, pw->pw_uid,  pw->pw_gid);
	    xfree(log_file);
	}

	if (setuid(pw->pw_uid) < 0)
	{
	    yaz_log(LOG_FATAL|LOG_ERRNO, "setuid");
	    exit(4);
	}
	xfree(uid);
    }

    while (mySocketManager.processEvent() > 0)
	;

    exit (0);
    return 0;
}
