/*
 * Copyright (c) 1998-2003, Index Data.
 * See the file LICENSE for details.
 * 
 * $Id: yaz-z-assoc.cpp,v 1.29 2003-10-20 18:31:44 adam Exp $
 */

#include <assert.h>
#include <signal.h>

#include <yaz/log.h>
#include <yaz++/z-assoc.h>
#include <yaz/otherinfo.h>

int Yaz_Z_Assoc::yaz_init_func()
{
#ifndef WIN32
    signal (SIGPIPE, SIG_IGN);
#endif
    return 1;
}

int Yaz_Z_Assoc::yaz_init_flag =  Yaz_Z_Assoc::yaz_init_func();  

Yaz_Z_Assoc::Yaz_Z_Assoc(IYaz_PDU_Observable *the_PDU_Observable)
{
    m_PDU_Observable = the_PDU_Observable;
    m_odr_in = odr_createmem (ODR_DECODE);
    m_odr_out = odr_createmem (ODR_ENCODE);
    m_odr_print = odr_createmem (ODR_PRINT);
    m_log = LOG_DEBUG;
    m_APDU_file = 0;
    m_APDU_fname = 0;
    m_hostname = 0;
    m_APDU_yazlog = 0;
}

void Yaz_Z_Assoc::set_APDU_log(const char *fname)
{
    if (m_APDU_file && m_APDU_file != stderr)
    {
	fclose (m_APDU_file);
	m_APDU_file = 0;
    }
    delete [] m_APDU_fname;
    m_APDU_fname = 0;

    if (fname) 
    {
	m_APDU_fname = new char[strlen(fname)+1];
	strcpy (m_APDU_fname, fname);
	if (!strcmp(fname, "-"))
	    m_APDU_file = stderr;
	else if (*fname == '\0')
	    m_APDU_file = 0;
	else
	    m_APDU_file = fopen (fname, "a");
	odr_setprint(m_odr_print, m_APDU_file);
    }
}

int Yaz_Z_Assoc::set_APDU_yazlog(int v)
{
    int old = m_APDU_yazlog;
    m_APDU_yazlog = v;
    return old;
}

const char *Yaz_Z_Assoc::get_APDU_log()
{
    return m_APDU_fname;
}

Yaz_Z_Assoc::~Yaz_Z_Assoc()
{
    m_PDU_Observable->destroy();  
    delete m_PDU_Observable;
    odr_destroy (m_odr_print);     // note: also runs fclose on m_APDU_file ..
    odr_destroy (m_odr_out);
    odr_destroy (m_odr_in);
    delete [] m_APDU_fname;
    delete [] m_hostname;
}

void Yaz_Z_Assoc::recv_PDU(const char *buf, int len)
{
    yaz_log (m_log, "recv_PDU len=%d", len);
    Z_APDU *apdu = decode_Z_PDU (buf, len);
    if (apdu)
    {
	recv_Z_PDU (apdu, len);
    }
    else
    {
	close();
	failNotify();
    }
}

Z_APDU *Yaz_Z_Assoc::create_Z_PDU(int type)
{
    Z_APDU *apdu = zget_APDU(m_odr_out, type);
    if (apdu->which == Z_APDU_initRequest)
    {
	Z_InitRequest * p = apdu->u.initRequest;
	char *newName = (char*) odr_malloc(m_odr_out, 50);
	strcpy (newName, p->implementationName);
	strcat (newName, " YAZ++");
	p->implementationName = newName;
    }
    return apdu;
}

Z_ReferenceId **Yaz_Z_Assoc::get_referenceIdP(Z_APDU *apdu)
{
    switch (apdu->which)
    {
    case  Z_APDU_initRequest:
	return &apdu->u.initRequest->referenceId; 
    case  Z_APDU_initResponse:
	return &apdu->u.initResponse->referenceId;
    case  Z_APDU_searchRequest:
	return &apdu->u.searchRequest->referenceId;
    case  Z_APDU_searchResponse:
	return &apdu->u.searchResponse->referenceId;
    case  Z_APDU_presentRequest:
	return &apdu->u.presentRequest->referenceId;
    case  Z_APDU_presentResponse:
	return &apdu->u.presentResponse->referenceId;
    case  Z_APDU_deleteResultSetRequest:
	return &apdu->u.deleteResultSetRequest->referenceId;
    case  Z_APDU_deleteResultSetResponse:
	return &apdu->u.deleteResultSetResponse->referenceId;
    case  Z_APDU_accessControlRequest:
	return &apdu->u.accessControlRequest->referenceId;
    case  Z_APDU_accessControlResponse:
	return &apdu->u.accessControlResponse->referenceId;
    case  Z_APDU_resourceControlRequest:
	return &apdu->u.resourceControlRequest->referenceId;
    case  Z_APDU_resourceControlResponse:
	return &apdu->u.resourceControlResponse->referenceId;
    case  Z_APDU_triggerResourceControlRequest:
	return &apdu->u.triggerResourceControlRequest->referenceId;
    case  Z_APDU_resourceReportRequest:
	return &apdu->u.resourceReportRequest->referenceId;
    case  Z_APDU_resourceReportResponse:
	return &apdu->u.resourceReportResponse->referenceId;
    case  Z_APDU_scanRequest:
	return &apdu->u.scanRequest->referenceId;
    case  Z_APDU_scanResponse:
	return &apdu->u.scanResponse->referenceId;
    case  Z_APDU_sortRequest:
	return &apdu->u.sortRequest->referenceId;
    case  Z_APDU_sortResponse:
	return &apdu->u.sortResponse->referenceId;
    case  Z_APDU_segmentRequest:
	return &apdu->u.segmentRequest->referenceId;
    case  Z_APDU_extendedServicesRequest:
	return &apdu->u.extendedServicesRequest->referenceId;
    case  Z_APDU_extendedServicesResponse:
	return &apdu->u.extendedServicesResponse->referenceId;
    case  Z_APDU_close:
	return &apdu->u.close->referenceId;
    }
    return 0;
}

void Yaz_Z_Assoc::transfer_referenceId(Z_APDU *from, Z_APDU *to)
{
    Z_ReferenceId **id_from = get_referenceIdP(from);
    Z_ReferenceId **id_to = get_referenceIdP(to);
    if (id_from && *id_from && id_to)
    {
	*id_to = (Z_ReferenceId*) odr_malloc (m_odr_out, sizeof(**id_to));
	(*id_to)->size = (*id_to)->len = (*id_from)->len;
	(*id_to)->buf = (unsigned char*) odr_malloc (m_odr_out, (*id_to)->len);
	memcpy ((*id_to)->buf, (*id_from)->buf, (*id_to)->len);
    }
    else if (id_to)
	*id_to = 0;
}

int Yaz_Z_Assoc::send_Z_PDU(Z_APDU *apdu, int *plen)
{
    char *buf;
    int len;
    if (encode_Z_PDU(apdu, &buf, &len) > 0)
    {
	if (plen)
	    *plen = len;
	return m_PDU_Observable->send_PDU(buf, len);
    }
    return -1;
}

Z_APDU *Yaz_Z_Assoc::decode_Z_PDU(const char *buf, int len)
{
    Z_APDU *apdu;

    odr_reset (m_odr_in);
    odr_setbuf (m_odr_in, (char*) buf, len, 0);

    if (!z_APDU(m_odr_in, &apdu, 0, 0))
    {
        yaz_log(LOG_LOG, "ODR error on incoming PDU: %s [near byte %d] ",
             odr_errmsg(odr_geterror(m_odr_in)),
             odr_offset(m_odr_in));
        yaz_log(LOG_LOG, "PDU dump:");
        odr_dumpBER(yaz_log_file(), buf, len);
        return 0;
    }
    else
    {
	if (m_APDU_yazlog)
	{   // use YAZ log FILE
	    FILE *save = m_APDU_file;

	    odr_setprint(m_odr_print, yaz_log_file());
	    z_APDU(m_odr_print, &apdu, 0, "decode");
	    m_APDU_file = save;
	}
	if (m_APDU_file)
        {
	    z_APDU(m_odr_print, &apdu, 0, "decode");
	    fflush(m_APDU_file);
	}
        return apdu;
    }
}

int Yaz_Z_Assoc::encode_Z_PDU(Z_APDU *apdu, char **buf, int *len)
{
    if (m_APDU_yazlog)
    {
	FILE *save = m_APDU_file;
	odr_setprint(m_odr_print, yaz_log_file()); // use YAZ log FILE
	z_APDU(m_odr_print, &apdu, 0, "encode");
	m_APDU_file = save;
    }
    if (m_APDU_file)
    {
	z_APDU(m_odr_print, &apdu, 0, "encode");
	fflush(m_APDU_file);
    }
    if (!z_APDU(m_odr_out, &apdu, 0, 0))
    {
	if (m_APDU_file)
	    fprintf (m_APDU_file, "PDU encode failed (above)");
	yaz_log (LOG_LOG, "yaz_Z_Assoc::encode_Z_PDU failed");
        return -1;
    }
    *buf = odr_getbuf (m_odr_out, len, 0);
    odr_reset (m_odr_out);
    return *len;
}

const char *Yaz_Z_Assoc::get_hostname()
{
    return m_hostname;
}

int Yaz_Z_Assoc::client(const char *addr)
{
    delete [] m_hostname;
    m_hostname = new char[strlen(addr)+1];
    strcpy (m_hostname, addr);
    return m_PDU_Observable->connect (this, addr);
}

void Yaz_Z_Assoc::close()
{
    m_PDU_Observable->close ();
}

void Yaz_Z_Assoc::server(const char *addr)
{
    delete [] m_hostname;
    m_hostname = new char[strlen(addr)+1];
    strcpy (m_hostname, addr);
    m_PDU_Observable->listen (this, addr);
}

ODR Yaz_Z_Assoc::odr_encode()
{
    return m_odr_out;
}

ODR Yaz_Z_Assoc::odr_decode()
{
    return m_odr_in;
}
ODR Yaz_Z_Assoc::odr_print()
{
    return m_odr_print;
}

void Yaz_Z_Assoc::timeout(int timeout)
{
    m_PDU_Observable->idleTime(timeout);
}

void Yaz_Z_Assoc::get_otherInfoAPDU(Z_APDU *apdu, Z_OtherInformation ***oip)
{
    switch (apdu->which)
    {
    case Z_APDU_initRequest:
	*oip = &apdu->u.initRequest->otherInfo;
	break;
    case Z_APDU_searchRequest:
	*oip = &apdu->u.searchRequest->otherInfo;
	break;
    case Z_APDU_presentRequest:
	*oip = &apdu->u.presentRequest->otherInfo;
	break;
    case Z_APDU_sortRequest:
	*oip = &apdu->u.sortRequest->otherInfo;
	break;
    case Z_APDU_scanRequest:
	*oip = &apdu->u.scanRequest->otherInfo;
	break;
    case Z_APDU_extendedServicesRequest:
        *oip = &apdu->u.extendedServicesRequest->otherInfo;
        break;
    case Z_APDU_deleteResultSetRequest:
        *oip = &apdu->u.deleteResultSetRequest->otherInfo;
        break;
    case Z_APDU_initResponse:
	*oip = &apdu->u.initResponse->otherInfo;
	break;
    case Z_APDU_searchResponse:
	*oip = &apdu->u.searchResponse->otherInfo;
	break;
    case Z_APDU_presentResponse:
	*oip = &apdu->u.presentResponse->otherInfo;
	break;
    case Z_APDU_sortResponse:
	*oip = &apdu->u.sortResponse->otherInfo;
	break;
    case Z_APDU_scanResponse:
	*oip = &apdu->u.scanResponse->otherInfo;
	break;
    case Z_APDU_extendedServicesResponse:
        *oip = &apdu->u.extendedServicesResponse->otherInfo;
        break;
    case Z_APDU_deleteResultSetResponse:
        *oip = &apdu->u.deleteResultSetResponse->otherInfo;
        break;
    default:
	*oip = 0;
	break;
    }
}

void Yaz_Z_Assoc::set_otherInformationString (
    Z_APDU *apdu,
    int oidval, int categoryValue,
    const char *str)
{
    Z_OtherInformation **otherInformation;
    get_otherInfoAPDU(apdu, &otherInformation);
    if (!otherInformation)
	return;
    set_otherInformationString(otherInformation, oidval, categoryValue, str);
}

void Yaz_Z_Assoc::set_otherInformationString (
    Z_OtherInformation **otherInformation,
    int oidval, int categoryValue,
    const char *str)
{
    int oid[OID_SIZE];
    struct oident ent;
    ent.proto = PROTO_Z3950;
    ent.oclass = CLASS_USERINFO;
    ent.value = (oid_value) oidval;
    if (!oid_ent_to_oid (&ent, oid))
	return ;
    set_otherInformationString(otherInformation, oid, categoryValue, str);
}

void Yaz_Z_Assoc::set_otherInformationString (
    Z_OtherInformation **otherInformation,
    int *oid, int categoryValue, const char *str)
{
    Z_OtherInformationUnit *oi =
	update_otherInformation(otherInformation, 1, oid, categoryValue, 0);
    if (!oi)
	return;
    oi->information.characterInfo = odr_strdup (odr_encode(), str);
}

Z_OtherInformationUnit *Yaz_Z_Assoc::update_otherInformation (
    Z_OtherInformation **otherInformationP, int createFlag,
    int *oid, int categoryValue, int deleteFlag)
{
    return yaz_oi_update (otherInformationP,
			  (createFlag ? odr_encode() : 0),
			  oid, categoryValue, deleteFlag);
}

Z_ReferenceId* Yaz_Z_Assoc::getRefID(char* str)
{
    Z_ReferenceId* id = NULL;

    if (str)
    {
        id = (Z_ReferenceId*) odr_malloc (m_odr_out, sizeof(*id));
        id->size = id->len = strlen(str);
        id->buf = (unsigned char *) str;
    }
    return id;
}

