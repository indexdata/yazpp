/*
 * Copyright (c) 2004, Index Data.
 * See the file LICENSE for details.
 * 
 * $Id: test-init-08.cpp,v 1.1 2004-03-25 23:14:07 adam Exp $
 */

#include <yaz/log.h>
#include <yaz/charneg.h>
#include <yaz/otherinfo.h>

#include <zlint.h>

Zlint_test_init_08::Zlint_test_init_08()
{
    m_no = 0;
}

Zlint_test_init_08::~Zlint_test_init_08()
{
}

Zlint_code Zlint_test_init_08::init(Zlint *z)
{
    int len;
    Z_APDU *apdu = z->create_Z_PDU(Z_APDU_initRequest);
    Z_InitRequest *init = apdu->u.initRequest;

    z->msg_check_for("for init message sizes %d", m_no);
    
    /* set all options.. see what target really supports .. */
    ODR_MASK_SET(init->protocolVersion, Z_ProtocolVersion_3);

    *init->maximumRecordSize = m_no * m_no * 100000 + 2000;
    *init->preferredMessageSize = m_no * m_no *100000 + 2000;

    int r = z->send_Z_PDU(apdu, &len);
    if (r < 0)
    {
	z->msg_check_fail("unable to send init request");
	return TEST_FINISHED;
    }
    return TEST_CONTINUE;
}

Zlint_code Zlint_test_init_08::recv_gdu(Zlint *z, Z_GDU *gdu)
{
    if (gdu->which == Z_GDU_Z3950 &&
	gdu->u.z3950 && gdu->u.z3950->which == Z_APDU_initResponse)
    {
	Z_InitResponse *init = gdu->u.z3950->u.initResponse;
	int ver = z->initResponseGetVersion(init);
	int result = init->result ? *init->result : 0;
	
	if (m_no * m_no * 100000 + 2000 < *init->maximumRecordSize)
	    z->msg_check_fail("maximumRecordSize bigger than proposed size");
	if (m_no * m_no * 100000 + 2000 < *init->preferredMessageSize)
	    z->msg_check_fail("preferredMessage bigger than proposed size");
	z->msg_check_ok();
	if (m_no < 2)
	{
	    m_no++;
	    return TEST_REOPEN;
	}
    }
    else
	z->msg_check_fail("did not receive init response as expected");
    return TEST_FINISHED;
}

Zlint_code Zlint_test_init_08::recv_fail(Zlint *z, int reason)
{
    z->msg_check_fail("target closed connection");
    if (m_no < 2)
    {
	m_no++;
	return TEST_REOPEN;
    }
    return TEST_FINISHED;
}

