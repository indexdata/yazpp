/*
 * Copyright (c) 1998-1999, Index Data.
 * See the file LICENSE for details.
 * Sebastian Hammer, Adam Dickmeiss
 * 
 * $Log: yaz-client.cpp,v $
 * Revision 1.9  1999-12-06 13:52:45  adam
 * Modified for new location of YAZ header files. Experimental threaded
 * operation.
 *
 * Revision 1.8  1999/11/10 10:02:34  adam
 * Work on proxy.
 *
 * Revision 1.7  1999/04/21 12:09:01  adam
 * Many improvements. Modified to proxy server to work with "sessions"
 * based on cookies.
 *
 * Revision 1.6  1999/04/20 10:30:05  adam
 * Implemented various stuff for client and proxy. Updated calls
 * to ODR to reflect new name parameter.
 *
 * Revision 1.5  1999/04/09 11:46:57  adam
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

#include <yaz/log.h>
#include <yaz/options.h>
#include <yaz/diagbib1.h>
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
    int m_setOffset;
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
    void recv_presentResponse(Z_PresentResponse *presentResponse);
    void recv_records (Z_Records *records);
    void recv_diagrecs(Z_DiagRec **pp, int num);
    void recv_namePlusRecord (Z_NamePlusRecord *zpr, int offset);
    void recv_record(Z_DatabaseRecord *record, int offset,
		     const char *databaseName);
    void recv_textRecord(int type, const char *buf, size_t len);
    void recv_genericRecord(Z_GenericRecord *r);
    void display_genericRecord(Z_GenericRecord *r, int level);
    void display_variant(Z_Variant *v, int level);
    void connectNotify();
    void failNotify();
    void timeoutNotify();
    int processCommand(const char *cmd);
    const char *MyClient::getCommand();
    int cmd_open(char *host);
    int cmd_connect(char *host);
    int cmd_quit(char *args);
    int cmd_close(char *args);
    int cmd_find(char *args);
    int cmd_show(char *args);
    int cmd_cookie(char *args);
    int cmd_init(char *args);
    int cmd_format(char *args);
    int cmd_proxy(char *args);
};


void MyClient::connectNotify()
{
    printf ("Connection accepted by target\n");
    set_lastReceived(-1);
}

void MyClient::timeoutNotify()
{
    printf ("Connection timeout\n");
    close();
}

void MyClient::failNotify()
{
    printf ("Connection closed by target\n");
    set_lastReceived(-1);
}

IYaz_PDU_Observer *MyClient::clone(IYaz_PDU_Observable *the_PDU_Observable)
{ 
    return new MyClient(the_PDU_Observable, m_socketManager);
}

MyClient::MyClient(IYaz_PDU_Observable *the_PDU_Observable,
		   Yaz_SocketManager *the_socketManager) :
    Yaz_IR_Assoc (the_PDU_Observable)
{
    m_setOffset = 1;
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

void MyClient::recv_diagrecs(Z_DiagRec **pp, int num)
{
    int i;
    oident *ent;
    Z_DefaultDiagFormat *r;

    printf("Diagnostic message(s) from database:\n");
    for (i = 0; i<num; i++)
    {
	Z_DiagRec *p = pp[i];
	if (p->which != Z_DiagRec_defaultFormat)
	{
	    printf("Diagnostic record not in default format.\n");
	    return;
	}
	else
	    r = p->u.defaultFormat;
	if (!(ent = oid_getentbyoid(r->diagnosticSetId)) ||
	    ent->oclass != CLASS_DIAGSET || ent->value != VAL_BIB1)
	    printf("Missing or unknown diagset\n");
	printf("    [%d] %s", *r->condition, diagbib1_str(*r->condition));
#ifdef ASN_COMPILED
	switch (r->which)
	{
	case Z_DefaultDiagFormat_v2Addinfo:
	    printf (" -- v2 addinfo '%s'\n", r->u.v2Addinfo);
	    break;
	case Z_DefaultDiagFormat_v3Addinfo:
	    printf (" -- v3 addinfo '%s'\n", r->u.v3Addinfo);
	    break;
	}
#else
	if (r->addinfo && *r->addinfo)
	    printf(" -- '%s'\n", r->addinfo);
	else
	    printf("\n");
#endif
    }
}

void MyClient::recv_textRecord(int type, const char *buf, size_t len)
{
    fwrite (buf, 1, len, stdout);
    fputc ('\n', stdout);
}

void MyClient::display_variant(Z_Variant *v, int level)
{
    int i;

    for (i = 0; i < v->num_triples; i++)
    {
	printf("%*sclass=%d,type=%d", level * 4, "", *v->triples[i]->zclass,
	    *v->triples[i]->type);
	if (v->triples[i]->which == Z_Triple_internationalString)
	    printf(",value=%s\n", v->triples[i]->value.internationalString);
	else
	    printf("\n");
    }
}

void MyClient::display_genericRecord(Z_GenericRecord *r, int level)
{
    int i;

    if (!r)
        return;
    for (i = 0; i < r->num_elements; i++)
    {
        Z_TaggedElement *t;

        printf("%*s", level * 4, "");
        t = r->elements[i];
        printf("(");
        if (t->tagType)
            printf("%d,", *t->tagType);
        else
            printf("?,");
        if (t->tagValue->which == Z_StringOrNumeric_numeric)
            printf("%d) ", *t->tagValue->u.numeric);
        else
            printf("%s) ", t->tagValue->u.string);
        if (t->content->which == Z_ElementData_subtree)
        {
            printf("\n");
            display_genericRecord(t->content->u.subtree, level+1);
        }
        else if (t->content->which == Z_ElementData_string)
            printf("%s\n", t->content->u.string);
        else if (t->content->which == Z_ElementData_numeric)
	    printf("%d\n", *t->content->u.numeric);
	else if (t->content->which == Z_ElementData_oid)
	{
	    int *ip = t->content->u.oid;
	    oident *oent;

	    if ((oent = oid_getentbyoid(t->content->u.oid)))
		printf("OID: %s\n", oent->desc);
	    else
	    {
		printf("{");
		while (ip && *ip >= 0)
		    printf(" %d", *(ip++));
		printf(" }\n");
	    }
	}
	else if (t->content->which == Z_ElementData_noDataRequested)
	    printf("[No data requested]\n");
	else if (t->content->which == Z_ElementData_elementEmpty)
	    printf("[Element empty]\n");
	else if (t->content->which == Z_ElementData_elementNotThere)
	    printf("[Element not there]\n");
	else
            printf("??????\n");
	if (t->appliedVariant)
	    display_variant(t->appliedVariant, level+1);
	if (t->metaData && t->metaData->supportedVariants)
	{
	    int c;

	    printf("%*s---- variant list\n", (level+1)*4, "");
	    for (c = 0; c < t->metaData->num_supportedVariants; c++)
	    {
		printf("%*svariant #%d\n", (level+1)*4, "", c);
		display_variant(t->metaData->supportedVariants[c], level + 2);
	    }
	}
    }
}

void MyClient::recv_genericRecord(Z_GenericRecord *r)
{
    display_genericRecord(r, 0);
}

void MyClient::recv_record(Z_DatabaseRecord *record, int offset,
			   const char *databaseName)
{
    Z_External *r = (Z_External*) record;
    oident *ent = oid_getentbyoid(r->direct_reference);

    /*
     * Tell the user what we got.
     */
    if (r->direct_reference)
    {
        printf("Record type: ");
        if (ent)
            printf("%s\n", ent->desc);
    }
    /* Check if this is a known, ASN.1 type tucked away in an octet string */
    Z_ext_typeent *etype = z_ext_getentbyref(ent->value);
    if (ent && (r->which == Z_External_octet || r->which == Z_External_single)
	&& (etype = z_ext_getentbyref(ent->value)))

    {
	void *rr;
	/*
	 * Call the given decoder to process the record.
	 */
	odr_setbuf(odr_decode(), (char*)record->u.octet_aligned->buf,
		   record->u.octet_aligned->len, 0);
	if (!(*etype->fun)(odr_decode(), (char **)&rr, 0, 0))
	{
	    odr_perror(odr_decode(), "Decoding constructed record.");
	    fprintf(stderr, "[Near %d]\n", odr_offset(odr_decode()));
	    fprintf(stderr, "Packet dump:\n---------\n");
	    odr_dumpBER(stderr, (char*)record->u.octet_aligned->buf,
			record->u.octet_aligned->len);
	    fprintf(stderr, "---------\n");
	}
	if (etype->what == Z_External_sutrs)
	{
	    Z_SUTRS *sutrs = (Z_SUTRS *) rr;
	    recv_textRecord ((int) VAL_SUTRS, (const char *) sutrs->buf,
			     (size_t) sutrs->len);
	}
	return;
    }
    if (r->which == Z_External_octet && record->u.octet_aligned->len)
    {
	recv_textRecord((int) ent->value,
			(const char *) record->u.octet_aligned->buf,
			(size_t) record->u.octet_aligned->len);
    }
    else if (ent && ent->value == VAL_SUTRS && r->which == Z_External_sutrs)
	recv_textRecord((int) VAL_SUTRS, (const char *) r->u.sutrs->buf,
			(size_t) r->u.sutrs->len);
    else if (ent && ent->value == VAL_GRS1 && r->which == Z_External_grs1)
        recv_genericRecord(r->u.grs1);
    else 
    {
        printf("Unknown record representation.\n");
        if (!z_External(odr_print(), &r, 0, 0))
        {
            odr_perror(odr_print(), "Printing external");
            odr_reset(odr_print());
        }
    }    
}

void MyClient::recv_namePlusRecord (Z_NamePlusRecord *zpr, int offset)
{
    if (zpr->databaseName)
	printf("[%s]", zpr->databaseName);
    if (zpr->which == Z_NamePlusRecord_surrogateDiagnostic)
        recv_diagrecs(&zpr->u.surrogateDiagnostic, 1);
    else
        recv_record(zpr->u.databaseRecord, offset, zpr->databaseName);
}

void MyClient::recv_records (Z_Records *records)
{
#ifdef ASN_COMPILED
    Z_DiagRec dr, *dr_p = &dr;
#endif
    if (!records)
	return;
    int i;
    switch (records->which)
    {
    case Z_Records_DBOSD:
        for (i = 0; i < records->u.databaseOrSurDiagnostics->num_records; i++)
            recv_namePlusRecord(records->u.databaseOrSurDiagnostics->
				records[i], i + m_setOffset);
	m_setOffset += records->u.databaseOrSurDiagnostics->num_records;
	break;
    case Z_Records_NSD:
#ifdef ASN_COMPILED
	dr.which = Z_DiagRec_defaultFormat;
	dr.u.defaultFormat = records->u.nonSurrogateDiagnostic;
	recv_diagrecs (&dr_p, 1);
#else
	recv_diagrecs (&records->u.nonSurrogateDiagnostic, 1);
#endif
	break;
    case Z_Records_multipleNSD:
	recv_diagrecs (records->u.multipleNonSurDiagnostics->diagRecs,
		       records->u.multipleNonSurDiagnostics->num_diagRecs);
	break;
    }
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
    recv_records (searchResponse->records);
}

void MyClient::recv_presentResponse(Z_PresentResponse *presentResponse)
{
    printf ("Got PresentResponse\n");
    recv_records (presentResponse->records);
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

int MyClient::cmd_connect(char *host)
{
    client (host);
    timeout (10);
    wait ();
    timeout (0);
    return 1;
}

int MyClient::cmd_open(char *host)
{
    client (host);
    timeout (10);
    wait ();
    timeout (0);
    send_initRequest();
    wait ();
    return 1;
}

int MyClient::cmd_init(char *args)
{
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
    else
	printf ("Not connected\n");
    return 1;
}

int MyClient::cmd_show(char *args)
{
    int start = m_setOffset, number = 1;

    sscanf (args, "%d %d", &start, &number);
    m_setOffset = start;
    if (send_presentRequest(start, number) >= 0)
	wait();
    else
	printf ("Not connected\n");
    return 1;
}

int MyClient::cmd_cookie(char *args)
{
    set_cookie(*args ? args : 0);
    return 1;
}

int MyClient::cmd_format(char *args)
{
    set_preferredRecordSyntax(args);
    return 1;
}

int MyClient::cmd_proxy(char *args)
{
    set_proxy(args);
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
	{"open", &cmd_open, "<host>[':'<port>][/<database>]"},
	{"connect", &cmd_connect, "<host>[':'<port>][/<database>]"},
	{"quit", &cmd_quit, ""},
	{"close", &cmd_close, ""},
	{"find", &cmd_find, "<query>"},
	{"show", &cmd_show, "[<start> [<number>]]"},
	{"cookie", &cmd_cookie, "<cookie>"},
	{"init", &cmd_init, ""},
	{"format", &cmd_format, "<record-syntax>"},
	{"proxy", &cmd_proxy, "<host>:[':'<port>]"},
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
    while (*cp && strchr ("\t \n", *cp))
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

    while ((ret = options("c:p:v:q", argv, argc, &arg)) != -2)
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
	case 'c':
	    set_cookie(arg);
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
        timeout (10);
	wait ();
        timeout (0);
	send_initRequest();
	wait ();
    }
    return 0;
}

int main(int argc, char **argv)
{
    Yaz_SocketManager mySocketManager;
    Yaz_PDU_Assoc *some = new Yaz_PDU_Assoc(&mySocketManager);

    MyClient z(some, &mySocketManager);

    if (z.args(&mySocketManager, argc, argv))
	exit (1);
    if (z.interactive(&mySocketManager))
	exit (1);
}
