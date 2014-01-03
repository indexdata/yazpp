/* This file is part of the yazpp toolkit.
 * Copyright (C) Index Data 
 * See the file LICENSE for details.
 */

#if HAVE_CONFIG_H
#include <config.h>
#endif
#include <yaz/log.h>
#include <yaz/pquery.h>

#include <zlint.h>

static const char *try_scan [] = {
    "@attr 1=4 ab",
    "@attr 1=1003 ab",
    "@attr 1=1016 ab",
    0
};

Zlint_test_scan_01::Zlint_test_scan_01()
{
    m_scan_no = 0;
}

Zlint_test_scan_01::~Zlint_test_scan_01()
{
}

Zlint_code Zlint_test_scan_01::init(Zlint *z)
{
    int len;
    Z_APDU *apdu = z->create_Z_PDU(Z_APDU_initRequest);
    Z_InitRequest *init = apdu->u.initRequest;


    ODR_MASK_SET(init->protocolVersion, Z_ProtocolVersion_3);
    ODR_MASK_SET(init->options, Z_Options_namedResultSets);
    ODR_MASK_SET(init->options, Z_Options_scan);

    int r = z->send_Z_PDU(apdu, &len);
    if (r < 0)
    {
        z->msg_check_fail("unable to send init request");
        return TEST_FINISHED;
    }
    return TEST_CONTINUE;
}

Zlint_code Zlint_test_scan_01::sendTest(Zlint *z)
{
    if (try_scan[m_scan_no])
    {
        int len;
        z->msg_check_for("scan support %s", try_scan[m_scan_no]);

        Z_APDU *apdu = zget_APDU(z->odr_encode(), Z_APDU_scanRequest);
        YAZ_PQF_Parser pqf_parser = yaz_pqf_create ();
        Z_ScanRequest *sr = apdu->u.scanRequest;
        sr->termListAndStartPoint = yaz_pqf_scan(pqf_parser,
                                                 z->odr_encode(),
                                                 &sr->attributeSet,
                                                 try_scan[m_scan_no]);

        z->getDatabase(&sr->databaseNames, &sr->num_databaseNames);

        yaz_pqf_destroy (pqf_parser);
        z->send_Z_PDU(apdu, &len);
        return TEST_CONTINUE;
    }
    else
        return TEST_FINISHED;
}

Zlint_code Zlint_test_scan_01::recv_gdu(Zlint *z, Z_GDU *gdu)
{
    if (gdu->which == Z_GDU_Z3950 &&
        gdu->u.z3950 && gdu->u.z3950->which == Z_APDU_initResponse)
    {
        Z_InitResponse *init = gdu->u.z3950->u.initResponse;
        int ver = z->initResponseGetVersion(init);
        int result = init->result ? *init->result : 0;
        if (ver > 3 || ver < 2)
            z->msg_check_fail("got version %d, expected 2 or 3", ver);
        if (!result)
        {
            z->msg_check_fail("init rejected (result false)");
            return TEST_FINISHED;
        }
        else if (!ODR_MASK_GET(init->options, Z_Options_scan))
        {
            z->msg_check_notapp();
            z->msg_check_info("scan unsupported");
            return TEST_FINISHED;
        }
        else
        {
            sendTest(z);
            return TEST_CONTINUE;
        }
    }
    else if (gdu->u.z3950 && gdu->u.z3950->which == Z_APDU_scanResponse)
    {
        Z_ScanResponse *sr =  gdu->u.z3950->u.scanResponse;
        if (sr->entries->nonsurrogateDiagnostics)
        {
            z->msg_check_ok();
            z->msg_check_info("scan NSD for %s", try_scan[m_scan_no]);
        }
        else if (sr->entries->entries && sr->entries->num_entries > 0)
        {
            z->msg_check_ok();
        }
        else
        {
            z->msg_check_fail("scan no entries/diagnostics for %s",
                              try_scan[m_scan_no]);
        }
        m_scan_no++;
        return sendTest(z);
    }
    else
        z->msg_check_fail("did not receive init response as expected");
    return TEST_FINISHED;
}

Zlint_code Zlint_test_scan_01::recv_fail(Zlint *z, int reason)
{
    m_scan_no++;
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

