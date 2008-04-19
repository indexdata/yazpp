/* This file is part of the yazpp toolkit.
 * Copyright (C) 1998-2008 Index Data and Mike Taylor
 * See the file LICENSE for details.
 */

#include <yaz/log.h>
#include <yazpp/z-server.h>

using namespace yazpp_1;

int Yaz_Facility_ILL::init(Z_Server *s, Z_InitRequest *initRequest,
                           Z_InitResponse *initResponse)
{
    Z_Options *req = initRequest->options;
    Z_Options *res = initResponse->options;
    
    if (ODR_MASK_GET(req, Z_Options_extendedServices))
        ODR_MASK_SET(res, Z_Options_extendedServices);
    return 1;
}

int Yaz_Facility_ILL::recv(Z_Server *s, Z_APDU *apdu_request)
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
/*
 * Local variables:
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * End:
 * vim: shiftwidth=4 tabstop=8 expandtab
 */

