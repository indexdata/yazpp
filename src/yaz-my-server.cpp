/* This file is part of the yazpp toolkit.
 * Copyright (C) Index Data 
 * See the file LICENSE for details.
 */

#if HAVE_CONFIG_H
#include <config.h>
#endif
#include <stdlib.h>
#include <yaz/log.h>
#include <yaz/diagbib1.h>
#include <yaz/options.h>
#include <yazpp/z-server.h>
#include <yazpp/pdu-assoc.h>
#include <yazpp/socket-manager.h>
#include <yaz/oid_db.h>

using namespace yazpp_1;

class MyILL : public Yaz_Facility_ILL {
public:
    void ill_service (Z_ExtendedServicesRequest *req,
                      Z_ItemOrder *io,
                      Z_ExtendedServicesResponse *res);
};

class MyUpdate : public Yaz_Facility_Update {
public:
    void update_service (Z_ExtendedServicesRequest *req,
                         Z_IUUpdate *io,
                         Z_ExtendedServicesResponse *res);
    void update_service0 (Z_ExtendedServicesRequest *req,
                         Z_IU0Update *io,
                         Z_ExtendedServicesResponse *res);
};


class MyRetrieval : public Yaz_Facility_Retrieval, Yaz_USMARC {
public:
    int sr_init (Z_InitRequest *initRequest,
                 Z_InitResponse *initResponse);
    void sr_search (Z_SearchRequest *searchRequest,
                        Z_SearchResponse *searchResponse);
    void sr_present (Z_PresentRequest *presentRequest,
                         Z_PresentResponse *presentResponse);
    void sr_record (const char *resultSetName,
                    int position,
                    Odr_oid *format,
                    Z_RecordComposition *comp,
                    Z_NamePlusRecord *namePlusRecord,
                    Z_Records *records);
};

class MyServer : public Z_Server {
public:
    ~MyServer();
    MyServer(IPDU_Observable *the_PDU_Observable);
    IPDU_Observer* sessionNotify(IPDU_Observable *the_PDU_Observable,
                                 int fd);
    void failNotify();
    void timeoutNotify();
    void connectNotify();

private:
    MyRetrieval m_retrieval;
    MyILL       m_ill;
    MyUpdate    m_update;
    int m_no;
};

void MyILL::ill_service (Z_ExtendedServicesRequest *req,
                         Z_ItemOrder *io,
                         Z_ExtendedServicesResponse *res)
{
    yaz_log (YLOG_LOG, "MyServer::ill_service");
}

void MyUpdate::update_service (Z_ExtendedServicesRequest *req,
                           Z_IUUpdate *io,
                           Z_ExtendedServicesResponse *res)
{
    yaz_log (YLOG_LOG, "MyServer::update_service (v1.1)");
}

void MyUpdate::update_service0 (Z_ExtendedServicesRequest *req,
                           Z_IU0Update *io,
                                Z_ExtendedServicesResponse *res)
{
    yaz_log (YLOG_LOG, "MyServer::update_service (v1.0)");
}

int MyRetrieval::sr_init (Z_InitRequest *initRequest,
                       Z_InitResponse *initResponse)
{
    yaz_log (YLOG_LOG, "MyServer::sr_init");
    return 1;
}

void MyRetrieval::sr_search (Z_SearchRequest *searchRequest,
                             Z_SearchResponse *searchResponse)
{
    yaz_log (YLOG_LOG, "MyServer::recv_Z_search");
    if (searchRequest->query->which == Z_Query_type_1)
    {
        Z_RPNStructure *s = searchRequest->query->u.type_1->RPNStructure;
        if (s->which == Z_RPNStructure_simple &&
            s->u.simple->which == Z_Operand_APT &&
            s->u.simple->u.attributesPlusTerm->term->which == Z_Term_general)
        {
            Odr_oct *term = s->u.simple->u.attributesPlusTerm->term->u.general;
            char *str = (char *) odr_malloc (odr_encode(), term->len+1);
            if (term->len)
                memcpy (str, term->buf, term->len);
            str[term->len] = '\0';
            *searchResponse->resultCount = atoi(str);
        }
    }
}

void MyRetrieval::sr_present (Z_PresentRequest *presentRequest,
                               Z_PresentResponse *presentResponse)
{
    yaz_log (YLOG_LOG, "MyServer::recv_Z_present");
}

void MyRetrieval::sr_record (const char *resultSetName,
                             int position,
                             Odr_oid *format,
                             Z_RecordComposition *comp,
                             Z_NamePlusRecord *namePlusRecord,
                             Z_Records *records)
{
    yaz_log (YLOG_LOG, "MyServer::recv_Z_record");
    const char *rec = get_record(position);
    if (rec)
        create_databaseRecord(odr_encode(), namePlusRecord, 0,
                              yaz_oid_recsyn_usmarc, rec, strlen(rec));
    else
        create_surrogateDiagnostics(odr_encode(), namePlusRecord, 0,
                                    YAZ_BIB1_PRESENT_REQUEST_OUT_OF_RANGE, 0);
}

MyServer::~MyServer()
{
}

IPDU_Observer *MyServer::sessionNotify(
    IPDU_Observable *the_PDU_Observable, int fd)
{
    MyServer *new_server;
    m_no++;
    new_server = new MyServer(the_PDU_Observable);
    new_server->timeout(900);
    new_server->facility_add(&new_server->m_retrieval, "my sr");
    new_server->facility_add(&new_server->m_ill, "my ill");
    new_server->facility_add(&new_server->m_update, "my update");
    new_server->set_APDU_log(get_APDU_log());

    return new_server;
}

MyServer::MyServer(IPDU_Observable *the_PDU_Observable) :
    Z_Server (the_PDU_Observable)
{
    m_no = 0;
}

void MyServer::timeoutNotify()
{
    yaz_log (YLOG_LOG, "connection timed out");
    delete this;
}

void MyServer::failNotify()
{
    yaz_log (YLOG_LOG, "connection closed by client");
    delete this;
}

void MyServer::connectNotify()
{
}

void usage(const char *prog)
{
    fprintf (stderr, "%s: [-a log] [-v level] [-T] @:port\n", prog);
    exit (1);
}

int main(int argc, char **argv)
{
    int thread_flag = 0;
    char *arg;
    char *prog = *argv;
    const char *addr = "tcp:@:9999";
    const char *cert_fname = 0;
    char *apdu_log = 0;

    SocketManager mySocketManager;

    PDU_Assoc *my_PDU_Assoc = 0;

    MyServer *z = 0;
    int ret;

    while ((ret = options("a:C:v:T", argv, argc, &arg)) != -2)
    {
        switch (ret)
        {
        case 0:
            addr = xstrdup(arg);
            break;
        case 'a':
            apdu_log = xstrdup(arg);
            break;
        case 'C':
            cert_fname = xstrdup(arg);
            break;
        case 'v':
            yaz_log_init_level (yaz_log_mask_str(arg));
            break;
        case 'T':
            thread_flag = 1;
            break;
        default:
            usage(prog);
            return 1;
        }
    }
#if YAZ_POSIX_THREADS
    if (thread_flag)
        my_PDU_Assoc = new PDU_AssocThread(&mySocketManager);
    else
        my_PDU_Assoc = new PDU_Assoc(&mySocketManager);
#else
    my_PDU_Assoc = new PDU_Assoc(&mySocketManager);
#endif

    my_PDU_Assoc->set_cert_fname(cert_fname);

    z = new MyServer(my_PDU_Assoc);
    z->server(addr);
    if (apdu_log)
    {
        yaz_log (YLOG_LOG, "set_APDU_log %s", apdu_log);
        z->set_APDU_log(apdu_log);
    }

    while (mySocketManager.processEvent() > 0)
        ;
    delete z;
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

