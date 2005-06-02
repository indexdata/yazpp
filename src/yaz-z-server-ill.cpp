/*
 * Copyright (c) 2000-2004, Index Data.
 * See the file LICENSE for details.
 * 
 * $Id: yaz-z-server-ill.cpp,v 1.12 2005-06-02 06:40:21 adam Exp $
 */

#include <yaz/log.h>
#include <yaz++/z-server.h>

using namespace yazpp_1;

int Yaz_Facility_ILL::init(Yaz_Z_Server *s, Z_InitRequest *initRequest,
			   Z_InitResponse *initResponse)
{
    Z_Options *req = initRequest->options;
    Z_Options *res = initResponse->options;
    
    if (ODR_MASK_GET(req, Z_Options_extendedServices))
	ODR_MASK_SET(res, Z_Options_extendedServices);
    return 1;
}

int Yaz_Facility_ILL::recv(Yaz_Z_Server *s, Z_APDU *apdu_request)
{   
    Z_APDU *apdu_response;

    if (apdu_request->which != Z_APDU_extendedServicesRequest)
	return 0;
    Z_ExtendedServicesRequest *req = apdu_request->u.extendedServicesRequest;
    if (!req->taskSpecificParameters || req->taskSpecificParameters->which !=
        Z_External_itemOrder)
        return 0;
    yaz_log (YLOG_LOG, "got ill p=%p", this);
    apdu_response = s->create_Z_PDU(Z_APDU_extendedServicesResponse);
    ill_service(req, req->taskSpecificParameters->u.itemOrder,
        apdu_response->u.extendedServicesResponse);
    s->transfer_referenceId(apdu_request, apdu_response);
    s->send_Z_PDU(apdu_response, 0);
    return 1;
}
