/* This file is part of the yazpp toolkit.
 * Copyright (C) Index Data 
 * See the file LICENSE for details.
 */

#if HAVE_CONFIG_H
#include <config.h>
#endif
#include <yaz/log.h>

#include <zlint.h>

#define REFID_BUF1 "zlint\000check1"
#define REFID_LEN1 12
#define REFID_BUF2 "zlint\000check2"
#define REFID_LEN2 12

Zlint_test_init_05::Zlint_test_init_05()
{
    m_init_response_no = 0;
}

Zlint_test_init_05::~Zlint_test_init_05()
{
}

Zlint_code Zlint_test_init_05::init(Zlint *z)
{
    int len;
    Z_APDU *apdu = z->create_Z_PDU(Z_APDU_initRequest);
    Z_InitRequest *init = apdu->u.initRequest;

    z->msg_check_for("for double init");

    /* send double init with differnet refID's */
    ODR_MASK_SET(init->protocolVersion, Z_ProtocolVersion_3);
    ODR_MASK_SET(init->options, Z_Options_concurrentOperations);
    init->referenceId = z->mk_refid(REFID_BUF1, REFID_LEN1);

    int r = z->send_Z_PDU(apdu, &len);
    if (r < 0)
    {
        z->msg_check_fail("unable to send init request");
        return TEST_FINISHED;
    }

    apdu = z->create_Z_PDU(Z_APDU_initRequest);
    init = apdu->u.initRequest;

    ODR_MASK_SET(init->protocolVersion, Z_ProtocolVersion_3);
    ODR_MASK_SET(init->options, Z_Options_concurrentOperations);

    init->referenceId = z->mk_refid(REFID_BUF2, REFID_LEN2);

    r = z->send_Z_PDU(apdu, &len);
    if (r < 0)
    {
        z->msg_check_fail("unable to send init request");
        return TEST_FINISHED;
    }
    return TEST_CONTINUE;
}

Zlint_code Zlint_test_init_05::recv_gdu(Zlint *z, Z_GDU *gdu)
{
    if (gdu->which == Z_GDU_Z3950 &&
        gdu->u.z3950 && gdu->u.z3950->which == Z_APDU_initResponse)
    {
        Z_InitResponse *init = gdu->u.z3950->u.initResponse;
        int result = init->result ? *init->result : 0;

        if (m_init_response_no == 0)
        {
            if (!init->referenceId)
            {
                z->msg_check_fail("missing referenceID from "
                                  "first init response");
                return TEST_FINISHED;
            }
            else if (init->referenceId->len != REFID_LEN1
                     || memcmp(init->referenceId->buf, REFID_BUF1, REFID_LEN1))
            {
                z->msg_check_fail("reference ID does not match from "
                                  "first init response");
                return TEST_FINISHED;
            }
        }
        else
        {
            if (!init->referenceId)
                z->msg_check_fail("missing referenceID from "
                                  "second init response");
            else if (init->referenceId->len != REFID_LEN2
                     || memcmp(init->referenceId->buf, REFID_BUF2, REFID_LEN2))
                z->msg_check_fail("reference ID does not match from "
                                  "second init response");
        }

        if (!result)
        {
            z->msg_check_fail("init rejected (result false)");
            return TEST_FINISHED;
        }
        else
        {
            if (m_init_response_no == 0)
            {
                m_init_response_no++;
                return TEST_CONTINUE;
            }
            else
                z->msg_check_ok();
        }
    }
    else
        z->msg_check_fail("did not receive init response as expected");
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

