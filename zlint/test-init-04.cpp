/*
 * Copyright (c) 2004, Index Data.
 * See the file LICENSE for details.
 * 
 * $Id: test-init-04.cpp,v 1.3 2004-12-13 20:50:54 adam Exp $
 */

#include <yaz/log.h>

#include <zlint.h>

#define REFID_BUF1 "zlint\000check1"
#define REFID_LEN1 12

Zlint_test_init_04::Zlint_test_init_04()
{
}

Zlint_test_init_04::~Zlint_test_init_04()
{
}

Zlint_code Zlint_test_init_04::init(Zlint *z)
{
    int len;
    Z_APDU *apdu = z->create_Z_PDU(Z_APDU_initRequest);
    Z_InitRequest *init = apdu->u.initRequest;

    z->msg_check_for("for referenceID for init");

    ODR_MASK_SET(init->protocolVersion, Z_ProtocolVersion_3);
    init->referenceId = z->mk_refid(REFID_BUF1, REFID_LEN1);

    int r = z->send_Z_PDU(apdu, &len);
    if (r < 0)
    {
	z->msg_check_fail("unable to send init request");
	return TEST_FINISHED;
    }
    return TEST_CONTINUE;
}

Zlint_code Zlint_test_init_04::recv_gdu(Zlint *z, Z_GDU *gdu)
{
    if (gdu->which == Z_GDU_Z3950 &&
	gdu->u.z3950 && gdu->u.z3950->which == Z_APDU_initResponse)
    {
	Z_InitResponse *init = gdu->u.z3950->u.initResponse;
	int ver = z->initResponseGetVersion(init);
	int result = init->result ? *init->result : 0;
	
	if (!init->referenceId)
	    z->msg_check_fail("missing referenceID from init response");
	else if (init->referenceId->len != REFID_LEN1
		 || memcmp(init->referenceId->buf, REFID_BUF1, REFID_LEN1))
	    z->msg_check_fail("reference ID does not match");
	z->msg_check_ok();
    }
    else
	z->msg_check_fail("did not receive init response as expected");
    return TEST_FINISHED;
}

