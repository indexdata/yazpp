/* This file is part of the yazpp toolkit.
 * Copyright (C) Index Data 
 * See the file LICENSE for details.
 */

#if HAVE_CONFIG_H
#include <config.h>
#endif
#include <yaz/log.h>

#include <zlint.h>

Zlint_test_init_03::Zlint_test_init_03()
{
}

Zlint_test_init_03::~Zlint_test_init_03()
{
}

Zlint_code Zlint_test_init_03::init(Zlint *z)
{
    int len;
    Z_APDU *apdu = z->create_Z_PDU(Z_APDU_initRequest);
    Z_InitRequest *init = apdu->u.initRequest;

    z->msg_check_for("for V9 init");

    ODR_MASK_ZERO(init->protocolVersion);
    int i;
    for (i = 0; i< 9; i++)
        ODR_MASK_SET(init->protocolVersion, i);

    int r = z->send_Z_PDU(apdu, &len);
    if (r < 0)
    {
        z->msg_check_fail("unable to send init request");
        return TEST_FINISHED;
    }
    return TEST_CONTINUE;
}

Zlint_code Zlint_test_init_03::recv_gdu(Zlint *z, Z_GDU *gdu)
{
    if (gdu->which == Z_GDU_Z3950 &&
        gdu->u.z3950 && gdu->u.z3950->which == Z_APDU_initResponse)
    {
        Z_InitResponse *init = gdu->u.z3950->u.initResponse;
        int ver = z->initResponseGetVersion(init);

        if (ver < 2 || ver > 5)
            z->msg_check_fail("%sgot version %d, expected 2-5", ver);
        z->msg_check_ok();
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

