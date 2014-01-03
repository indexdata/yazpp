/* This file is part of the yazpp toolkit.
 * Copyright (C) Index Data 
 * See the file LICENSE for details.
 */

#if HAVE_CONFIG_H
#include <config.h>
#endif
#include <stdlib.h>
#include <yaz/log.h>
#include <yaz/options.h>
#include <yaz/diagbib1.h>
#include <yaz/marcdisp.h>
#include <yazpp/ir-assoc.h>
#include <yazpp/pdu-assoc.h>
#include <yazpp/socket-manager.h>
#include <yaz/oid_db.h>

extern "C" {
#if HAVE_READLINE_READLINE_H
#include <readline/readline.h>
#endif
#if HAVE_READLINE_HISTORY_H
#include <readline/history.h>
#endif
}

using namespace yazpp_1;

class YAZ_EXPORT MyClient : public IR_Assoc {
private:
    int m_interactive_flag;
    char m_thisCommand[1024];
    char m_lastCommand[1024];
    int m_setOffset;
    SocketManager *m_socketManager;
public:
    MyClient(IPDU_Observable *the_PDU_Observable,
             SocketManager *the_SocketManager);
    IPDU_Observer *sessionNotify(
        IPDU_Observable *the_PDU_Observable, int fd);
    int args(SocketManager *socketManager, int argc, char **argv);
    int interactive(SocketManager *socketManager);
    int wait();
    void recv_initResponse(Z_InitResponse *initResponse);
    void recv_searchResponse(Z_SearchResponse *searchResponse);
    void recv_presentResponse(Z_PresentResponse *presentResponse);
    void recv_records (Z_Records *records);
    void recv_diagrecs(Z_DiagRec **pp, int num);
    void recv_namePlusRecord (Z_NamePlusRecord *zpr, int offset);
    void recv_record(Z_DatabaseRecord *record, int offset,
                     const char *databaseName);
    void recv_textRecord(const char *buf, size_t len);
    void recv_genericRecord(Z_GenericRecord *r);
    void connectNotify();
    void failNotify();
    void timeoutNotify();
    char *get_cookie (Z_OtherInformation **oi);
    int processCommand(const char *cmd);
    const char *getCommand();
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

IPDU_Observer *MyClient::sessionNotify(IPDU_Observable *the_PDU_Observable,
                                       int fd)
{
    return new MyClient(the_PDU_Observable, m_socketManager);
}

MyClient::MyClient(IPDU_Observable *the_PDU_Observable,
                   SocketManager *the_socketManager) :
    IR_Assoc (the_PDU_Observable)
{
    m_setOffset = 1;
    m_interactive_flag = 1;
    m_thisCommand[0] = '\0';
    m_lastCommand[0] = '\0';
    m_socketManager = the_socketManager;
}

void usage(char *prog)
{
    fprintf (stderr, "%s: [-v log] [-c cookie] [-p proxy] [zurl]\n", prog);
    exit (1);
}

char *MyClient::get_cookie(Z_OtherInformation **otherInfo)
{
    Z_OtherInformationUnit *oi =
        update_otherInformation(otherInfo, 0, yaz_oid_userinfo_cookie, 1, 1);

    if (oi && oi->which == Z_OtherInfo_characterInfo)
        return oi->information.characterInfo;
    return 0;
}

void MyClient::recv_initResponse(Z_InitResponse *initResponse)
{
    printf ("Got InitResponse. Status ");
    if (*initResponse->result)
    {
        printf ("Ok\n");

        const char *p = get_cookie (&initResponse->otherInfo);
        if (p)
        {
            printf ("cookie = %s\n", p);
            set_cookie(p);
        }
    }
    else
        printf ("Fail\n");
}

void MyClient::recv_diagrecs(Z_DiagRec **pp, int num)
{
    int i;
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
             printf("    [" ODR_INT_PRINTF "] %s", *r->condition, diagbib1_str(*r->condition));
        switch (r->which)
        {
        case Z_DefaultDiagFormat_v2Addinfo:
            printf (" -- v2 addinfo '%s'\n", r->u.v2Addinfo);
            break;
        case Z_DefaultDiagFormat_v3Addinfo:
            printf (" -- v3 addinfo '%s'\n", r->u.v3Addinfo);
            break;
        }
    }
}

void MyClient::recv_textRecord(const char *buf, size_t len)
{
    fwrite (buf, 1, len, stdout);
    fputc ('\n', stdout);
}

void MyClient::recv_genericRecord(Z_GenericRecord *r)
{
    WRBUF w = wrbuf_alloc();
    yaz_display_grs1(w, r, 0);
    fwrite(wrbuf_buf(w), 1, wrbuf_len(w), stdout);
    wrbuf_destroy(w);
}

void MyClient::recv_record(Z_DatabaseRecord *record, int offset,
                           const char *databaseName)
{
    Z_External *r = (Z_External*) record;
    /*
     * Tell the user what we got.
     */
    if (r->direct_reference)
    {
        char name_oid_str[OID_STR_MAX];
        const char *name_oid = yaz_oid_to_string_buf(r->direct_reference, 0,
                                                     name_oid_str);
        printf("Record type: %s\n", name_oid ? name_oid : "unknown");
    }
    if (r->which == Z_External_octet && record->u.octet_aligned->len)
    {
        if (yaz_oid_is_iso2709(r->direct_reference))
        {
            yaz_marc_t mt = yaz_marc_create();

            const char *result_buf;
            size_t result_size;
            yaz_marc_decode_buf(mt, (const char *)
                                record->u.octet_aligned->buf,
                                record->u.octet_aligned->len,
                                &result_buf, &result_size);
            fwrite(result_buf, 1, result_size, stdout);
            yaz_marc_destroy(mt);
        }
        else
        {
            recv_textRecord((const char *) record->u.octet_aligned->buf,
                            (size_t) record->u.octet_aligned->len);
        }
    }
    else if (r->which == Z_External_sutrs)
        recv_textRecord((const char *) r->u.sutrs->buf,
                        (size_t) r->u.sutrs->len);
    else if (r->which == Z_External_grs1)
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
    Z_DiagRec dr, *dr_p = &dr;
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
        dr.which = Z_DiagRec_defaultFormat;
        dr.u.defaultFormat = records->u.nonSurrogateDiagnostic;
        recv_diagrecs (&dr_p, 1);
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
    }
    else
    {
        printf ("Ok\n");
        printf ("Hits: " ODR_INT_PRINTF "\n", *searchResponse->resultCount);
    }
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
    timeout (-1);
    return 1;
}

int MyClient::cmd_open(char *host)
{
    client (host);
    timeout (10);
    wait ();
    timeout (-1);
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
        const char *cmd;
        int (MyClient::*fun)(char *arg);
        const char *ad;
    } cmd[] = {
        {"open", &MyClient::cmd_open, "<host>[':'<port>][/<database>]"},
        {"connect", &MyClient::cmd_connect, "<host>[':'<port>][/<database>]"},
        {"quit", &MyClient::cmd_quit, ""},
        {"close", &MyClient::cmd_close, ""},
        {"find", &MyClient::cmd_find, "<query>"},
        {"show", &MyClient::cmd_show, "[<start> [<number>]]"},
        {"cookie", &MyClient::cmd_cookie, "<cookie>"},
        {"init", &MyClient::cmd_init, ""},
        {"format", &MyClient::cmd_format, "<record-syntax>"},
        {"proxy", &MyClient::cmd_proxy, "<host>:[':'<port>]"},
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

int MyClient::interactive(SocketManager *socketManager)
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

int MyClient::args(SocketManager *socketManager, int argc, char **argv)
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
            yaz_log_init_level (yaz_log_mask_str(arg));
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
        timeout (-1);
        send_initRequest();
        wait ();
    }
    return 0;
}

int main(int argc, char **argv)
{
    SocketManager mySocketManager;
    PDU_Assoc *some = new PDU_Assoc(&mySocketManager);

    MyClient z(some, &mySocketManager);

    if (z.args(&mySocketManager, argc, argv))
        exit (1);
    if (z.interactive(&mySocketManager))
        exit (1);
    return 0;
}
/*
 * Local variables:
 * c-basic-offset: 4
 * c-file-style: "Stroustrup"
 * indent-tabs-mode: nil
 * End:
 * vim: shiftwidth=4 tabstop=8 expandtab
 */

