/*
 * Copyright (c) 2000-2001, Index Data.
 * See the file LICENSE for details.
 * 
 * $Log: yaz-z-server-ill.cpp,v $
 * Revision 1.4  2001-04-04 14:02:49  adam
 * URSULA / Z-ruth service.
 *
 * Revision 1.3  2001/04/03 14:37:19  adam
 * More work ILL-service.
 *
 * Revision 1.2  2001/03/29 15:14:26  adam
 * Minor updates.
 *
 * Revision 1.1  2001/03/27 14:47:45  adam
 * New server facility scheme.
 *
 */

#include <yaz/log.h>
#include <yaz++/yaz-z-server.h>

int Yaz_Facility_ILL::init(Yaz_Z_Server *s, Z_InitRequest *initRequest,
			   Z_InitResponse *initResponse)
{
    Z_Options *req = initRequest->options;
    Z_Options *res = initResponse->options;
    
    if (ODR_MASK_GET(req, Z_Options_extendedServices))
	ODR_MASK_SET(res, Z_Options_extendedServices);
    return ill_init (initRequest, initResponse);
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
    yaz_log (LOG_LOG, "got ill p=%p", this);
    apdu_response = s->create_Z_PDU(Z_APDU_extendedServicesResponse);
    ill_service(req, req->taskSpecificParameters->u.itemOrder,
        apdu_response->u.extendedServicesResponse);
    s->send_Z_PDU(apdu_response);
    return 1;
}
