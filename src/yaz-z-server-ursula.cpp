/*
 * Copyright (c) 2001, Index Data.
 * See the file LICENSE for details.
 * 
 * $Log: yaz-z-server-ursula.cpp,v $
 * Revision 1.1  2001-04-04 14:02:49  adam
 * URSULA / Z-ruth service.
 *
 */

#include <yaz/log.h>
#include <yaz++/yaz-z-server.h>

int Yaz_Facility_Ursula::init(Yaz_Z_Server *s, Z_InitRequest *initRequest,
			      Z_InitResponse *initResponse)
{
    Z_Options *req = initRequest->options;
    Z_Options *res = initResponse->options;
    
    if (ODR_MASK_GET(req, Z_Options_extendedServices))
	ODR_MASK_SET(res, Z_Options_extendedServices);
    return 1;
}

int Yaz_Facility_Ursula::recv(Yaz_Z_Server *s, Z_APDU *apdu_request)
{   
    Z_APDU *apdu_response;

    if (apdu_request->which != Z_APDU_extendedServicesRequest)
	return 0;
    Z_ExtendedServicesRequest *req = apdu_request->u.extendedServicesRequest;

    Z_External *r = req->taskSpecificParameters;

    if (!r)
	return 0;

    if (r->which != ODR_EXTERNAL_octet)
    {
	logf (LOG_LOG, "ursula::recv not octet alighed");
        return 0;
    }
    odr_setbuf (s->odr_decode(), (char*) r->u.octet_aligned->buf,
		r->u.octet_aligned->len, 0);
    Z_UrsPDU *pdu = 0;
    if (!z_UrsPDU (s->odr_decode(), &pdu, 0, ""))
    {
	logf (LOG_LOG, "ursula::decode failed");
	return 0;
    }
    yaz_log (LOG_LOG, "got ursula packet");
    apdu_response = s->create_Z_PDU(Z_APDU_extendedServicesResponse);
    ursula_service(req, pdu, apdu_response->u.extendedServicesResponse);
    s->send_Z_PDU(apdu_response);
    return 1;
}
