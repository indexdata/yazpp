/*
 * Copyright (c) 1998-1999, Index Data.
 * See the file LICENSE for details.
 * Sebastian Hammer, Adam Dickmeiss
 * 
 * $Log: yaz-client.cpp,v $
 * Revision 1.5  1999-04-09 11:46:57  adam
 * Added object Yaz_Z_Assoc. Much more functional client.
 *
 * Revision 1.4  1999/03/23 14:17:57  adam
 * More work on timeout handling. Work on yaz-client.
 *
 * Revision 1.3  1999/02/02 14:01:18  adam
 * First WIN32 port of YAZ++.
 *
 * Revision 1.2  1999/01/28 13:08:42  adam
 * Yaz_PDU_Assoc better encapsulated. Memory leak fix in
 * yaz-socket-manager.cc.
 *
 * Revision 1.1.1.1  1999/01/28 09:41:07  adam
 * First implementation of YAZ++.
 *
 */

#include <log.h>
#include <options.h>
#include <yaz-ir-assoc.h>
#include <yaz-pdu-assoc.h>
#include <yaz-socket-manager.h>

extern "C" {
#if HAVE_READLINE_READLINE_H
#include <readline/readline.h>
#endif
#if HAVE_READLINE_HISTORY_H
#include <readline/history.h>
#endif
}

class YAZ_EXPORT MyClient : public Yaz_IR_Assoc {
private:
    int m_interactive_flag;
    char m_thisCommand[1024];
    char m_lastCommand[1024];
    Yaz_SocketManager *m_socketManager;
public:
    MyClient(IYaz_PDU_Observable *the_PDU_Observable,
	     Yaz_SocketManager *the_SocketManager);
    IYaz_PDU_Observer *clone(IYaz_PDU_Observable *the_PDU_Observable);
    int args(Yaz_SocketManager *socketManager, int argc, char **argv);
    int interactive(Yaz_SocketManager *socketManager);
    int wait();
    void recv_initResponse(Z_InitResponse *initResponse);
    void recv_searchResponse(Z_SearchResponse *searchResponse);
    int processCommand(const char *cmd);
    const char *MyClient::getCommand();
    int cmd_open(char *host);
    int cmd_quit(char *args);
    int cmd_close(char *args);
    int cmd_find(char *args);
};

IYaz_PDU_Observer *MyClient::clone(IYaz_PDU_Observable *the_PDU_Observable)
{
    return new MyClient(the_PDU_Observable, m_socketManager);
}

MyClient::MyClient(IYaz_PDU_Observable *the_PDU_Observable,
		   Yaz_SocketManager *the_socketManager) :
    Yaz_IR_Assoc (the_PDU_Observable)
{
    m_interactive_flag = 1;
    m_thisCommand[0] = '\0';
    m_lastCommand[0] = '\0';
    m_socketManager = the_socketManager;
}

void usage(char *prog)
{
    fprintf (stderr, "%s: [-v log] [-p proxy] [zurl]\n", prog);
    exit (1);
}

void MyClient::recv_initResponse(Z_InitResponse *initResponse)
{
    printf ("Got InitResponse. Status ");
    if (*initResponse->result)
	printf ("Ok\n");
    else
	printf ("Fail\n");
}

void MyClient::recv_searchResponse(Z_SearchResponse *searchResponse)
{
    printf ("Got SearchResponse. Status ");
    if (!*searchResponse->searchStatus)
    {
	printf ("Fail\n");
	return;
    }
    printf ("Ok\n");
    printf ("Hits: %d\n", *searchResponse->resultCount);
}

int MyClient::wait()
{
    set_lastReceived(0);
    while (m_socketManager->processEvent() > 0)
    {
	if (get_lastReceived())
	    return 1;
    }
    return 0;
}

#define C_PROMPT "Z>"

int MyClient::cmd_open(char *host)
{
    client (host);
    if (send_initRequest() >= 0)
	wait();
    else
	close();
    return 1;
}

int MyClient::cmd_quit(char *args)
{
    return 0;
}

int MyClient::cmd_close(char *args)
{
    close();
    return 1;
}

int MyClient::cmd_find(char *args)
{
    Yaz_Z_Query query;

    if (query.set_rpn(args) <= 0)
    {
	printf ("Bad RPN query\n");
	return 1;
    }
    if (send_searchRequest(&query) >= 0)
	wait();
    return 1;
}

int MyClient::processCommand(const char *commandLine)
{
    char cmdStr[1024], cmdArgs[1024];
    cmdArgs[0] = '\0';
    cmdStr[0] = '\0';
    static struct {
        char *cmd;
        int (MyClient::*fun)(char *arg);
        char *ad;
    } cmd[] = {
	{"open", &cmd_open, "('tcp'|'osi')':'[<tsel>'/']<host>[':'<port>]"},
        {"quit", &cmd_quit, ""},
	{"close", &cmd_close, ""},
	{"find", &cmd_find, ""},
	{0,0,0}
    };
    
    if (sscanf(commandLine, "%s %[^;]", cmdStr, cmdArgs) < 1)
	return 1;
    int i;
    for (i = 0; cmd[i].cmd; i++)
	if (!strncmp(cmd[i].cmd, cmdStr, strlen(cmdStr)))
	    break;
    
    int res = 1;
    if (cmd[i].cmd) // Invoke command handler
	res = (this->*cmd[i].fun)(cmdArgs);
    else            // Dump help screen
    {
	printf("Unknown command: %s.\n", cmdStr);
	printf("Currently recognized commands:\n");
	for (i = 0; cmd[i].cmd; i++)
	    printf("   %s %s\n", cmd[i].cmd, cmd[i].ad);
    }
    return res;
}

const char *MyClient::getCommand()
{
#if HAVE_READLINE_READLINE_H
    // Read using GNU readline
    char *line_in;
    line_in=readline(C_PROMPT);
    if (!line_in)
	return 0;
#if HAVE_READLINE_HISTORY_H
    if (*line_in)
	add_history(line_in);
#endif
    strncpy(m_thisCommand,line_in, 1023);
    m_thisCommand[1023] = '\0';
    free (line_in);
#else    
    // Read using fgets(3)
    printf (C_PROMPT);
    fflush(stdout);
    if (!fgets(m_thisCommand, 1023, stdin))
	return 0;
#endif
    // Remove trailing whitespace
    char *cp = m_thisCommand + strlen(m_thisCommand);
    while (cp != m_thisCommand && strchr("\t \n", cp[-1]))
	cp--;
    *cp = '\0';
    cp = m_thisCommand;
    // Remove leading spaces...
    while (strchr ("\t \n", *cp))
	cp++;
    // Save command if non-empty
    if (*cp != '\0')
	strcpy (m_lastCommand, cp);
    return m_lastCommand;
}

int MyClient::interactive(Yaz_SocketManager *socketManager)
{
    const char *cmd;
    if (!m_interactive_flag)
	return 0;
    while ((cmd = getCommand()))
    {
	if (!processCommand(cmd))
	    break;
    }
    return 0;
}

int MyClient::args(Yaz_SocketManager *socketManager, int argc, char **argv)
{
    char *host = 0;
    char *proxy = 0;
    char *arg;
    char *prog = argv[0];
    int ret;

    while ((ret = options("p:v:q", argv, argc, &arg)) != -2)
    {
        switch (ret)
        {
        case 0:
            if (host)
	    {
		usage(prog);
		return 1;
	    }
	    host = arg;
            break;
        case 'p':
	    if (proxy)
	    {
		usage(prog);
		return 1;
	    }
	    set_proxy(arg);
	    break;
	case 'v':
	    log_init_level (log_mask_str(arg));
	    break;
	case 'q':
	    m_interactive_flag = 0;
	    break;
        default:
	    usage(prog);
	    return 1;
        }
    }
    if (host)
    {
	client (host);
	send_initRequest();
	wait ();
    }
    return 0;
}

int main(int argc, char **argv)
{
    Yaz_SocketManager mySocketManager;
    Yaz_PDU_Assoc *some = new Yaz_PDU_Assoc(&mySocketManager, 0);

    MyClient z(some, &mySocketManager);

    if (z.args(&mySocketManager, argc, argv))
	exit (1);
    if (z.interactive(&mySocketManager))
	exit (1);
}
