/* This file is part of the yazpp toolkit.
 * Copyright (C) Index Data 
 * See the file LICENSE for details.
 */

#if HAVE_CONFIG_H
#include <config.h>
#endif
#include <yaz/log.h>
#include <yaz/charneg.h>
#include <yaz/otherinfo.h>

#include <zlint.h>

Zlint_test_init_07::Zlint_test_init_07()
{
}

Zlint_test_init_07::~Zlint_test_init_07()
{
}

Zlint_code Zlint_test_init_07::init(Zlint *z)
{
    int len;
    Z_APDU *apdu = z->create_Z_PDU(Z_APDU_initRequest);
    Z_InitRequest *init = apdu->u.initRequest;
    Z_OtherInformation **oi;

    z->msg_check_for("for character set negotiation");

    /* set all options.. see what target really supports .. */
    ODR_MASK_SET(init->protocolVersion, Z_ProtocolVersion_3);
    yaz_oi_APDU(apdu, &oi);
    if (!oi)
    {
        z->msg_check_fail("encoding failure");
        return TEST_FINISHED;
    }
    else
    {
        Z_OtherInformationUnit *p0;
        const char *negotiationCharset[] = {
            "UTF-8",
            "UTF-16",
            "UCS-2",
            "UCS-4",
            "ISO-8859-1"
        };
        char *yazLang = 0;

        if ((p0=yaz_oi_update(oi, z->odr_encode(), NULL, 0, 0))) {
            ODR_MASK_SET(init->options, Z_Options_negotiationModel);

            p0->which = Z_OtherInfo_externallyDefinedInfo;
            p0->information.externallyDefinedInfo =

                yaz_set_proposal_charneg(
                    z->odr_encode(),
                    negotiationCharset, 5,
                    (const char**)&yazLang, yazLang ? 1 : 0, 1);
        }
    }
    int r = z->send_Z_PDU(apdu, &len);
    if (r < 0)
    {
        z->msg_check_fail("unable to send init request");
        return TEST_FINISHED;
    }
    return TEST_CONTINUE;
}

Zlint_code Zlint_test_init_07::recv_gdu(Zlint *z, Z_GDU *gdu)
{
    if (gdu->which == Z_GDU_Z3950 &&
        gdu->u.z3950 && gdu->u.z3950->which == Z_APDU_initResponse)
    {
        Z_InitResponse *init = gdu->u.z3950->u.initResponse;

        if (ODR_MASK_GET(init->options, Z_Options_negotiationModel))
        {
            Z_CharSetandLanguageNegotiation *p =
                yaz_get_charneg_record(init->otherInfo);

            if (p) {

                char *charset=NULL, *lang=NULL;
                int selected;
                NMEM m = nmem_create();

                yaz_get_response_charneg(m, p, &charset, &lang,
                                         &selected);
                z->msg_check_ok();
                z->msg_check_info("Accepted character set : %s", charset);
                z->msg_check_info("Accepted code language : %s", lang ? lang : "none");
                z->msg_check_info("Accepted records in ...: %d", selected );
                nmem_destroy(m);
                return TEST_FINISHED;
            }
        }
        z->msg_check_notapp();
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

