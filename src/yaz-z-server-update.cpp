/*
 * Copyright (c) 2000-2001, Index Data.
 * See the file LICENSE for details.
 * 
 * $Id: yaz-z-server-update.cpp,v 1.3 2001-11-04 22:36:21 adam Exp $
 */

#include <yaz/log.h>
#include <yaz++/yaz-z-server.h>

int Yaz_Facility_Update::init(Yaz_Z_Server *s, Z_InitRequest *initRequest,
			   Z_InitResponse *initResponse)
{
    Z_Options *req = initRequest->options;
    Z_Options *res = initResponse->options;
    
    if (ODR_MASK_GET(req, Z_Options_extendedServices))
	ODR_MASK_SET(res, Z_Options_extendedServices);
    return 1;
}

int Yaz_Facility_Update::recv(Yaz_Z_Server *s, Z_APDU *apdu_request)
{   
    Z_APDU *apdu_response;

    if (apdu_request->which != Z_APDU_extendedServicesRequest)
	return 0;
    Z_ExtendedServicesRequest *req = apdu_request->u.extendedServicesRequest;
    if (req->taskSpecificParameters && req->taskSpecificParameters->which ==
        Z_External_update)
    {
	yaz_log (LOG_LOG, "got update p=%p", this);
	apdu_response = s->create_Z_PDU(Z_APDU_extendedServicesResponse);
	update_service(req, req->taskSpecificParameters->u.update,
		       apdu_response->u.extendedServicesResponse);
	s->transfer_referenceId(apdu_request, apdu_response);
	s->send_Z_PDU(apdu_response);
    }
    else if (req->taskSpecificParameters &&
	     req->taskSpecificParameters->which == Z_External_update0)
    {
	yaz_log (LOG_LOG, "got update p=%p", this);
	apdu_response = s->create_Z_PDU(Z_APDU_extendedServicesResponse);
	update_service0 (req, req->taskSpecificParameters->u.update0,
			 apdu_response->u.extendedServicesResponse);
	s->transfer_referenceId(apdu_request, apdu_response);
	s->send_Z_PDU(apdu_response);
    }
    return 1;
}