/* This file is part of the yazpp toolkit.
 * Copyright (C) Index Data 
 * See the file LICENSE for details.
 */

#if HAVE_CONFIG_H
#include <config.h>
#endif
#include <yaz/log.h>
#include <yaz/pquery.h>
#include <yaz/sortspec.h>

#include <zlint.h>
#include <yaz/oid_db.h>

static const char *try_query[] = {
    "@attr 1=4 petersson",
    "@attr 1=1016 petersson",
    "@attr 1=4 kingdom",
    "@attr 1=1016 kingdom",
    "@attr 1=62 sword",
    "sword"
    "seven",
    "@attr 1=4 water",
    "@attr 1=1016 water",
    "computer",
    "@attr 1=4 computer",
    "@attr 1=1016 computer",
    "water",
    "join",
    "about",
    "map",
    0,
};

static const char *try_syntax [] = {
    "usmarc",
    "unimarc",
    "danmarc",
    "sutrs",
    "grs1",
    "xml",
    "normarc",
    0
};

static const char *try_sort [] = {
    "1=4 <",
    "1=4 >",
    "1=62 >",
    "1=62 <",
    0
};

Zlint_test_search_01::Zlint_test_search_01()
{
    m_query_no = 0;
    m_record_syntax_no = 0;
    m_got_result_set = 0;
    m_sort_no = 0;
}

Zlint_test_search_01::~Zlint_test_search_01()
{
}

Zlint_code Zlint_test_search_01::init(Zlint *z)
{
    int len;
    Z_APDU *apdu = z->create_Z_PDU(Z_APDU_initRequest);
    Z_InitRequest *init = apdu->u.initRequest;

    z->msg_check_for("search and retrieve");

    ODR_MASK_SET(init->protocolVersion, Z_ProtocolVersion_3);

    ODR_MASK_SET(init->options, Z_Options_namedResultSets);
    ODR_MASK_SET(init->options, Z_Options_sort);

    int r = z->send_Z_PDU(apdu, &len);
    if (r < 0)
    {
        z->msg_check_fail("unable to send init request");
        return TEST_FINISHED;
    }
    return TEST_CONTINUE;
}

Zlint_code Zlint_test_search_01::sendTest(Zlint *z)
{
    if (!m_got_result_set)
    {
        Z_APDU *apdu = zget_APDU(z->odr_encode(), Z_APDU_searchRequest);
        Z_SearchRequest *sr;
        sr = apdu->u.searchRequest;
        sr->query = (Z_Query *) odr_malloc(z->odr_encode(), sizeof(*sr->query));
        if (try_query[m_query_no] && sr)
        {
            sr->query->which = Z_Query_type_1;
            Z_RPNQuery *rpn;
            YAZ_PQF_Parser pqf_parser = yaz_pqf_create ();

            z->getDatabase(&sr->databaseNames, &sr->num_databaseNames);

            rpn = yaz_pqf_parse(pqf_parser, z->odr_encode(),
                                try_query[m_query_no]);

            yaz_pqf_destroy (pqf_parser);

            if (!rpn)
            {
                z->msg_check_fail("Query %s invalid", try_query[m_query_no]);
                return TEST_FINISHED;
            }
            int len;
            sr->query->u.type_1 = rpn;
            z->send_Z_PDU(apdu, &len);
        }
        else
        {
            z->msg_check_notapp();
            z->msg_check_info("unable to get any hit count");
            return TEST_FINISHED;
        }
    }
    else if (m_got_result_set && try_syntax[m_record_syntax_no])
    {
        int len;
        Z_APDU *apdu = zget_APDU(z->odr_encode(), Z_APDU_presentRequest);
        Z_PresentRequest *pr = apdu->u.presentRequest;
        *pr->numberOfRecordsRequested = 1;
        *pr->resultSetStartPoint = 1;

        z->msg_check_for("record syntax %s", try_syntax[m_record_syntax_no]);
        pr->preferredRecordSyntax =
            yaz_string_to_oid_odr(yaz_oid_std(),
                                  CLASS_RECSYN, try_syntax[m_record_syntax_no],
                                  z->odr_encode());
        z->send_Z_PDU(apdu, &len);
        return TEST_CONTINUE;
    }
    else if(m_got_result_set && !try_syntax[m_record_syntax_no])
    {
        Z_APDU *apdu = zget_APDU(z->odr_encode(), Z_APDU_sortRequest);
        if (apdu && try_sort[m_sort_no])
        {
            z->msg_check_for("sort %s", try_sort[m_sort_no]);

            const char *setstring = "default";
            int len;
            Z_SortRequest *sr = apdu->u.sortRequest;

            sr->num_inputResultSetNames = 1;
            sr->num_inputResultSetNames = 1;
            sr->inputResultSetNames = (Z_InternationalString **)
                odr_malloc (z->odr_encode(), sizeof(*sr->inputResultSetNames));
            sr->inputResultSetNames[0] = odr_strdup (z->odr_encode(), setstring);
            sr->sortedResultSetName = odr_strdup(z->odr_encode(), setstring);
            sr->sortSequence = yaz_sort_spec(z->odr_encode(), try_sort[m_sort_no]);
            z->send_Z_PDU(apdu, &len);
        }
        else
            return TEST_FINISHED;
    }
    else
    {
        printf ("finished...\n");
        return TEST_FINISHED;
    }
    return TEST_CONTINUE;
}

Zlint_code Zlint_test_search_01::recv_gdu(Zlint *z, Z_GDU *gdu)
{
    if (gdu->which == Z_GDU_Z3950 &&
        gdu->u.z3950 && gdu->u.z3950->which == Z_APDU_initResponse)
    {
        Z_InitResponse *init = gdu->u.z3950->u.initResponse;
        int result = init->result ? *init->result : 0;
        if (!result)
        {
            z->msg_check_notapp();
            z->msg_check_info ("init rejected (result false)");
            return TEST_FINISHED;
        }
        return sendTest(z);
    }
    else if (gdu->which == Z_GDU_Z3950 &&
             gdu->u.z3950 && gdu->u.z3950->which == Z_APDU_searchResponse)
    {
        Z_SearchResponse *sr = gdu->u.z3950->u.searchResponse;
        if (sr->records && (sr->records->which == Z_Records_NSD
                            ||
                            sr->records->which == Z_Records_multipleNSD))
            m_query_no++;
        else if (!sr->resultCount || *sr->resultCount == 0)
            m_query_no++;
        else
        {
            z->msg_check_ok();
            z->msg_check_info("got %d result count with %s", *sr->resultCount,
                              try_query[m_query_no]);
            m_got_result_set = 1;
        }
        return sendTest(z);
    }
    else if (gdu->which == Z_GDU_Z3950 &&
             gdu->u.z3950 && gdu->u.z3950->which == Z_APDU_presentResponse)
    {
        Z_PresentResponse *sr = gdu->u.z3950->u.presentResponse;
        if (sr->records && (sr->records->which == Z_Records_NSD
                            ||
                            sr->records->which == Z_Records_multipleNSD))
        {
            z->msg_check_ok();
            z->msg_check_info("present returned NSD for %s",
                              try_syntax[m_record_syntax_no]);
        }
        else if (sr->records && sr->records->which == Z_Records_DBOSD
                 && sr->records->u.databaseOrSurDiagnostics->num_records>0
                 && sr->records->u.databaseOrSurDiagnostics->records[0])
        {
            if (sr->records->u.databaseOrSurDiagnostics->records[0]->which == Z_NamePlusRecord_databaseRecord)
            {
                Z_External *ext = sr->records->u.databaseOrSurDiagnostics->records[0]->u.databaseRecord;
                Odr_oid *expectRecordSyntax =
                    yaz_string_to_oid_odr(
                        yaz_oid_std(), CLASS_RECSYN,
                        try_syntax[m_record_syntax_no], z->odr_decode());
                if (oid_oidcmp(expectRecordSyntax,
                               ext->direct_reference))
                {
                    z->msg_check_fail("Got Record in different syntax "
                                      "from that requested %s",
                                      try_syntax[m_record_syntax_no]);
                }
                else
                    z->msg_check_ok();
            }
            else if (sr->records->u.databaseOrSurDiagnostics->records[0]->which == Z_NamePlusRecord_surrogateDiagnostic)
            {
                z->msg_check_ok();
                z->msg_check_info("present returned SD %s",
                                  try_syntax[m_record_syntax_no]);
            }
            else
            {
                z->msg_check_ok();
                z->msg_check_info("present returned fragment %s",
                                  try_syntax[m_record_syntax_no]);
            }
        }
        else
        {
            z->msg_check_fail("present returned no records or diagnostics");
        }
        m_record_syntax_no++;
        return sendTest(z);
    }
    else if (gdu->which == Z_GDU_Z3950 &&
             gdu->u.z3950 && gdu->u.z3950->which == Z_APDU_sortResponse)
    {
        Z_SortResponse *sr =  gdu->u.z3950->u.sortResponse;
        z->msg_check_ok();
        if (sr->diagnostics)
            z->msg_check_info( "sort NSD for %s", try_sort[m_sort_no]);
        m_sort_no++;
        return sendTest(z);
    }
    else
        z->msg_check_fail("did not receive init/search/present response "
                          "as expected");
    return TEST_FINISHED;
}

Zlint_code Zlint_test_search_01::recv_fail(Zlint *z, int reason)
{
    z->msg_check_fail("target closed connection");
    return TEST_FINISHED;
}
/*
 * Local variables:
 * c-basic-offset: 4
 * c-file-style: "Stroustrup"
 * indent-tabs-mode: nil
 * End:
 * vim: shiftwidth=4 tabstop=8 expandtab
 */

