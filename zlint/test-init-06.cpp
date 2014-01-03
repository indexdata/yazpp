/* This file is part of the yazpp toolkit.
 * Copyright (C) Index Data 
 * See the file LICENSE for details.
 */

#if HAVE_CONFIG_H
#include <config.h>
#endif
#include <yaz/log.h>

#include <zlint.h>

Zlint_test_init_06::Zlint_test_init_06()
{
}

Zlint_test_init_06::~Zlint_test_init_06()
{
}

Zlint_code Zlint_test_init_06::init(Zlint *z)
{
    int len;
    Z_APDU *apdu = z->create_Z_PDU(Z_APDU_initRequest);
    Z_InitRequest *init = apdu->u.initRequest;

    z->msg_check_for("for init options");

    /* set all options.. see what target really supports .. */
    ODR_MASK_SET(init->protocolVersion, Z_ProtocolVersion_3);
    ODR_MASK_ZERO(init->options);
    int i;
    for (i = 0; i <= 24; i++)
        ODR_MASK_SET(init->options, i);

    int r = z->send_Z_PDU(apdu, &len);
    if (r < 0)
    {
        z->msg_check_fail("unable to send init request");
        return TEST_FINISHED;
    }
    return TEST_CONTINUE;
}

Zlint_code Zlint_test_init_06::recv_gdu(Zlint *z, Z_GDU *gdu)
{
    if (gdu->which == Z_GDU_Z3950 &&
        gdu->u.z3950 && gdu->u.z3950->which == Z_APDU_initResponse)
    {
        Z_InitResponse *init = gdu->u.z3950->u.initResponse;

        if (init->options)
        {
            int i;
            int no_set = 0;
            int no_reset = 0;
            for (i = 0; i <= 24; i++)
                if (ODR_MASK_GET(init->options, i))
                    no_set++;
                else
                    no_reset++;
            if (no_set < 2)
            {
                z->msg_check_fail("suspicuously few option bits set");
                return TEST_FINISHED;
            }
            if (no_reset == 0)
            {
                z->msg_check_fail("suspicuously many option bits set");
                return TEST_FINISHED;
            }
        }
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

