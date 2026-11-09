/* This file is part of the yazpp toolkit.
 * Copyright (C) Index Data
 * See the file LICENSE for details.
 */

#if HAVE_CONFIG_H
#include <config.h>
#endif
#include <yazpp/z-server.h>
#include <yaz/test.h>
#include <yaz/diagbib1.h>
#include <yaz/oid_db.h>
#include <limits>
#include <climits>
#include <cstdio>
#include <cstring>

using namespace yazpp_1;

namespace {

// Exercise BER decoding and encoding without opening a socket.
class Transport : public IPDU_Observable {
    ODR m_odr;
public:
    Z_APDU *response;
    Transport() : m_odr(odr_createmem(ODR_DECODE)), response(0) {}
    ~Transport() { odr_destroy(m_odr); }
    void reset() { odr_reset(m_odr); response = 0; }
    int send_PDU(const char *buf, int len)
    {
        odr_setbuf(m_odr, const_cast<char *>(buf), len, 0);
        int ok = z_APDU(m_odr, &response, 0, 0);
        YAZ_CHECK(ok);
        return ok ? 0 : -1;
    }
    int connect(IPDU_Observer *, const char *) { return 0; }
    int listen(IPDU_Observer *, const char *) { return 0; }
    void shutdown() {}
    IPDU_Observable *clone() { return new Transport; }
    void destroy() {}
    void idleTime(int) {}
    const char *getpeername() { return "test"; }
    void close_session() {}
};

class Server : public Z_Server {
public:
    Server(Transport *transport) : Z_Server(transport) {}
    void connectNotify() {}
    void failNotify() { YAZ_CHECK(false); }
    void timeoutNotify() { YAZ_CHECK(false); }
    IPDU_Observer *sessionNotify(IPDU_Observable *, int) { return 0; }
};

class Retrieval : public Yaz_Facility_Retrieval {
public:
    Odr_int hits;
    int calls;
    Retrieval() : hits(0), calls(0) {}
    int sr_init(Z_InitRequest *, Z_InitResponse *) { return 1; }
    void sr_search(Z_SearchRequest *req, Z_SearchResponse *res)
    {
        YAZ_CHECK(!strcmp(req->resultSetName, "test"));
        *res->resultCount = hits;
    }
    void sr_present(Z_PresentRequest *, Z_PresentResponse *) {}
    void sr_record(const char *set, int position, Odr_oid *,
                   Z_RecordComposition *, Z_NamePlusRecord *record,
                   Z_Records *)
    {
        calls++;
        YAZ_CHECK(!strcmp(set, "test"));
        char buf[40];
        sprintf(buf, "record %d", position);
        create_databaseRecord(odr_encode(), record, "test",
                              yaz_oid_recsyn_sutrs, buf, strlen(buf));
    }
};

class Fixture {
public:
    Retrieval retrieval;
    Transport *transport;
    Server server;
    ODR out;
    Fixture() : transport(new Transport), server(transport),
                out(odr_createmem(ODR_ENCODE))
    {
        server.facility_add(&retrieval, "retrieval");
        Z_APDU *req = zget_APDU(out, Z_APDU_initRequest);
        *req->u.initRequest->preferredMessageSize = 16 * 1024 * 1024;
        *req->u.initRequest->maximumRecordSize = 16 * 1024 * 1024;
        Z_APDU *res = send(req);
        YAZ_CHECK(res && res->which == Z_APDU_initResponse);
        if (res && res->which == Z_APDU_initResponse)
            YAZ_CHECK(*res->u.initResponse->result);
    }
    ~Fixture() { odr_destroy(out); }
    Z_APDU *send(Z_APDU *req)
    {
        transport->reset();
        retrieval.calls = 0;
        int ok = z_APDU(out, &req, 0, 0);
        YAZ_CHECK(ok);
        if (!ok)
            return 0;
        int len;
        char *buf = odr_getbuf(out, &len, 0);
        server.recv_PDU(buf, len);
        odr_reset(out);
        YAZ_CHECK(transport->response);
        return transport->response;
    }
    Z_PresentResponse *present(Odr_int start, Odr_int count)
    {
        Z_APDU *req = zget_APDU(out, Z_APDU_presentRequest);
        req->u.presentRequest->resultSetId = odr_strdup(out, "test");
        *req->u.presentRequest->resultSetStartPoint = start;
        *req->u.presentRequest->numberOfRecordsRequested = count;
        Z_APDU *res = send(req);
        YAZ_CHECK(res && res->which == Z_APDU_presentResponse);
        return res && res->which == Z_APDU_presentResponse ?
            res->u.presentResponse : 0;
    }
    Z_SearchResponse *search(Odr_int hits, Odr_int small,
                             Odr_int large, Odr_int medium)
    {
        retrieval.hits = hits;
        Z_APDU *apdu = zget_APDU(out, Z_APDU_searchRequest);
        Z_SearchRequest *req = apdu->u.searchRequest;
        req->resultSetName = odr_strdup(out, "test");
        req->num_databaseNames = 1;
        req->databaseNames = (char **) odr_malloc(out, sizeof(char *));
        req->databaseNames[0] = odr_strdup(out, "test");
        req->query = (Z_Query *) odr_malloc(out, sizeof(Z_Query));
        req->query->which = Z_Query_type_2;
        req->query->u.type_2 = odr_create_Odr_oct(out, "test", 4);
        *req->smallSetUpperBound = small;
        *req->largeSetLowerBound = large;
        *req->mediumSetPresentNumber = medium;
        Z_APDU *res = send(apdu);
        YAZ_CHECK(res && res->which == Z_APDU_searchResponse);
        return res && res->which == Z_APDU_searchResponse ?
            res->u.searchResponse : 0;
    }
};

void check_records(Z_Records *records, int start, int count)
{
    YAZ_CHECK(records && records->which == Z_Records_DBOSD);
    if (!records || records->which != Z_Records_DBOSD)
        return;
    Z_NamePlusRecordList *list = records->u.databaseOrSurDiagnostics;
    YAZ_CHECK_EQ(list->num_records, count);
    for (int i = 0; i < list->num_records; i++)
    {
        Z_NamePlusRecord *rec = list->records[i];
        YAZ_CHECK(rec->which == Z_NamePlusRecord_databaseRecord);
        if (rec->which != Z_NamePlusRecord_databaseRecord)
            continue;
        Z_External *ext = rec->u.databaseRecord;
        YAZ_CHECK(ext && ext->which == Z_External_sutrs);
        if (ext && ext->which == Z_External_sutrs)
        {
            char buf[40];
            sprintf(buf, "record %d", start + i);
            YAZ_CHECK_EQ(ext->u.sutrs->len, strlen(buf));
            YAZ_CHECK(ext->u.sutrs->len == (int) strlen(buf) &&
                      !memcmp(ext->u.sutrs->buf, buf, strlen(buf)));
        }
    }
}

void check_diagnostic(Z_Records *records)
{
    YAZ_CHECK(records && records->which == Z_Records_NSD);
    if (records && records->which == Z_Records_NSD)
        YAZ_CHECK_EQ(*records->u.nonSurrogateDiagnostic->condition,
                     YAZ_BIB1_PRESENT_REQUEST_OUT_OF_RANGE);
}

void test_present()
{
    Fixture f;
    // Establish a result set, then retrieve records from a non-default offset.
    f.search(10000, 0, 1, 0);
    const int counts[] = {0, 1, 3, 10000};
    for (unsigned i = 0; i < sizeof(counts) / sizeof(*counts); i++)
    {
        int start = counts[i] == 10000 ? 1 : 5;
        Z_PresentResponse *res = f.present(start, counts[i]);
        if (!res)
            continue;
        YAZ_CHECK_EQ(*res->presentStatus, Z_PresentStatus_success);
        YAZ_CHECK_EQ(*res->numberOfRecordsReturned, counts[i]);
        YAZ_CHECK_EQ(*res->nextResultSetPosition,
                     counts[i] ? start + counts[i] : 0);
        YAZ_CHECK_EQ(f.retrieval.calls, counts[i]);
        check_records(res->records, start, counts[i]);
    }
    // The next position and final loop increment may safely reach INT_MAX.
    f.search(INT_MAX, 0, 1, 0);
    Z_PresentResponse *boundary = f.present(INT_MAX - 1, 1);
    if (boundary)
    {
        YAZ_CHECK_EQ(*boundary->presentStatus, Z_PresentStatus_success);
        YAZ_CHECK_EQ(*boundary->numberOfRecordsReturned, 1);
        YAZ_CHECK_EQ(*boundary->nextResultSetPosition, INT_MAX);
        YAZ_CHECK_EQ(f.retrieval.calls, 1);
        check_records(boundary->records, INT_MAX - 1, 1);
    }
    const Odr_int max = std::numeric_limits<Odr_int>::max();
    const Odr_int min = std::numeric_limits<Odr_int>::min();
    const Odr_int invalid[][2] = {
        {1, -1}, {1, min}, {1, 10001}, {1, max},
        {0, 1}, {-1, 1}, {min, 1}, {INT_MAX, 1},
        {max, 1}, {max, 10000}, {INT_MAX - 1, 2}
    };
    for (unsigned i = 0; i < sizeof(invalid) / sizeof(*invalid); i++)
    {
        Z_PresentResponse *res = f.present(invalid[i][0], invalid[i][1]);
        if (!res)
            continue;
        YAZ_CHECK_EQ(*res->presentStatus, Z_PresentStatus_failure);
        YAZ_CHECK_EQ(*res->numberOfRecordsReturned, 0);
        YAZ_CHECK_EQ(*res->nextResultSetPosition, 0);
        YAZ_CHECK_EQ(f.retrieval.calls, 0);
        check_diagnostic(res->records);
    }
}

void test_search()
{
    Fixture f;
    // hits, small bound, large bound, medium count, expected returned count.
    const Odr_int cases[][5] = {
        {0, 5, 10, 3, 0}, {5, 5, 10, 3, 5},
        {6, 5, 10, 3, 3}, {6, 5, 10, 9, 6},
        {10, 5, 10, 3, 0}, {6, 5, 10, 0, 0},
        {10000, 10000, 10001, 0, 10000},
        {10001, 0, 10002, 10000, 10000}
    };
    for (unsigned i = 0; i < sizeof(cases) / sizeof(*cases); i++)
    {
        const Odr_int *c = cases[i];
        Z_SearchResponse *res = f.search(c[0], c[1], c[2], c[3]);
        if (!res)
            continue;
        YAZ_CHECK(*res->searchStatus);
        YAZ_CHECK_EQ(*res->resultCount, c[0]);
        YAZ_CHECK_EQ(*res->numberOfRecordsReturned, c[4]);
        YAZ_CHECK_EQ(f.retrieval.calls, c[4]);
        YAZ_CHECK_EQ(*res->nextResultSetPosition,
                     c[4] ? c[4] + 1 : (c[0] ? 1 : 0));
        if (c[4])
        {
            YAZ_CHECK(res->presentStatus &&
                      *res->presentStatus == Z_PresentStatus_success);
            check_records(res->records, 1, (int) c[4]);
        }
        else
        {
            YAZ_CHECK(!res->presentStatus);
            YAZ_CHECK(!res->records);
        }
    }
    const Odr_int max = std::numeric_limits<Odr_int>::max();
    const Odr_int invalid[][4] = {
        // hits, smallSetUpperBound, largeSetLowerBound, mediumSetPresentNumber.
        {6, -1, 10, 3}, {6, 5, -1, 3}, {6, 5, 10, -1},
        {10001, 10001, 10002, 0}, {10001, 0, 10002, 10001},
        {max, max, max, 0}, {max - 1, 0, max, max}
    };
    for (unsigned i = 0; i < sizeof(invalid) / sizeof(*invalid); i++)
    {
        const Odr_int *c = invalid[i];
        Z_SearchResponse *res = f.search(c[0], c[1], c[2], c[3]);
        if (!res)
            continue;
        YAZ_CHECK(*res->searchStatus);
        YAZ_CHECK_EQ(*res->resultCount, c[0]);
        YAZ_CHECK(res->presentStatus &&
                  *res->presentStatus == Z_PresentStatus_failure);
        YAZ_CHECK_EQ(*res->numberOfRecordsReturned, 0);
        YAZ_CHECK_EQ(f.retrieval.calls, 0);
        check_diagnostic(res->records);
    }
}

}

int main(int argc, char **argv)
{
    YAZ_CHECK_INIT(argc, argv);
    test_present();
    test_search();
    YAZ_CHECK_TERM;
}

/*
 * Local variables:
 * c-basic-offset: 4
 * c-file-style: "Stroustrup"
 * indent-tabs-mode: nil
 * End:
 * vim: shiftwidth=4 tabstop=8 expandtab
 */
