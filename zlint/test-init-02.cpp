/*
 * Copyright (c) 2004, Index Data.
 * See the file LICENSE for details.
 * 
 * $Id: test-init-02.cpp,v 1.1 2004-03-25 23:14:07 adam Exp $
 */

#include <yaz/log.h>

#include <zlint.h>

Zlint_test_init_02::Zlint_test_init_02()
{
}

Zlint_test_init_02::~Zlint_test_init_02()
{
}

Zlint_code Zlint_test_init_02::init(Zlint *z)
{
    int len;
    Z_APDU *apdu = z->create_Z_PDU(Z_APDU_initRequest);
    Z_InitRequest *init = apdu->u.initRequest;

    z->msg_check_for("for V2 init");

    ODR_MASK_ZERO(init->protocolVersion);
    ODR_MASK_SET(init->protocolVersion, Z_ProtocolVersion_1);
    ODR_MASK_SET(init->protocolVersion, Z_ProtocolVersion_2);
    
    int r = z->send_Z_PDU(apdu, &len);
    if (r < 0)
    {
	z->msg_check_fail("unable to send init request");
	return TEST_FINISHED;
    }
    return TEST_CONTINUE;
}

Zlint_code Zlint_test_init_02::recv_gdu(Zlint *z, Z_GDU *gdu)
{
    if (gdu->which == Z_GDU_Z3950 &&
	gdu->u.z3950 && gdu->u.z3950->which == Z_APDU_initResponse)
    {
	Z_InitResponse *init = gdu->u.z3950->u.initResponse;
	int ver = z->initResponseGetVersion(init);
	int result = init->result ? *init->result : 0;
	if (ver != 2)
	    z->msg_check_fail("got version %d, expected 2", ver);
	z->msg_check_ok();
    }
    else
	z->msg_check_fail("did not receive init response as expected");
    return TEST_FINISHED;
}



