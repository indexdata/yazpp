/*
 * Copyright (c) 1998-2000, Index Data.
 * See the file LICENSE for details.
 * 
 * $Log: yaz-z-assoc.cpp,v $
 * Revision 1.16  2000-12-14 16:00:39  adam
 * Ignoring signal SIGPIPE.
 *
 * Revision 1.15  2000/10/11 11:58:17  adam
 * Moved header files to include/yaz++. Switched to libtool and automake.
 * Configure script creates yaz++-config script.
 *
 * Revision 1.14  2000/09/12 16:40:33  heikki
 * minor
 *
 * Revision 1.13  2000/09/08 10:23:42  adam
 * Added skeleton of yaz-z-server.
 *
 * Revision 1.12  2000/09/05 13:57:28  adam
 * Fixed get_otherInfoAPDU to return otherInfo for extended services.
 *
 * Revision 1.11  2000/09/04 08:59:16  adam
 * Changed call to logging functions (yaz_ added).
 *
 * Revision 1.10  2000/09/04 08:29:22  adam
 * Fixed memory leak(s). Added re-use of associations, rather than
 * re-init, when maximum number of targets are in use.
 *
 * Revision 1.9  2000/08/10 08:42:42  adam
 * Fixes for {set,get}_APDU_log.
 *
 * Revision 1.8  2000/08/07 14:19:59  adam
 * Fixed serious bug regarding timeouts. Improved logging for proxy.
 *
 * Revision 1.7  2000/05/10 11:36:58  ian
 * Added default parameters for refid to request functions.
 * Added default parameter for result set name to search and present request.
 * Commented out forced logging of PDU contents.
 * Added send_deleteResultSetRequest
 *
 * Revision 1.6  1999/12/06 13:52:45  adam
 * Modified for new location of YAZ header files. Experimental threaded
 * operation.
 *
 * Revision 1.5  1999/11/10 10:02:34  adam
 * Work on proxy.
 *
 * Revision 1.4  1999/09/13 12:53:44  adam
 * Proxy removes OtherInfo Proxy Address and Session ID. Other
 * Otherinfo remains untouched.
 *
 * Revision 1.3  1999/04/21 12:09:01  adam
 * Many improvements. Modified to proxy server to work with "sessions"
 * based on cookies.
 *
 * Revision 1.2  1999/04/20 10:30:05  adam
 * Implemented various stuff for client and proxy. Updated calls
 * to ODR to reflect new name parameter.
 *
 * Revision 1.1  1999/04/09 11:46:57  adam
 * Added object Yaz_Z_Assoc. Much more functional client.
 *
 */

#include <assert.h>
#include <signal.h>

#include <yaz/log.h>
#include <yaz++/yaz-z-assoc.h>
#include <yaz/otherinfo.h>

int Yaz_Z_Assoc::yaz_init_func()
{
    signal (SIGPIPE, SIG_IGN);
    nmem_init();
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
	if (*fname && strcmp(fname, "-"))
	    m_APDU_file = fopen (fname, "a");
	else
	    m_APDU_file = stderr;
	odr_setprint(m_odr_print, m_APDU_file);
    }
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
    logf (m_log, "recv_PDU len=%d", len);
    Z_APDU *apdu = decode_Z_PDU (buf, len);
    if (apdu)
    {
	recv_Z_PDU (apdu);
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

int Yaz_Z_Assoc::send_Z_PDU(Z_APDU *apdu)
{
    char *buf;
    int len;
    if (encode_Z_PDU(apdu, &buf, &len) > 0)
	return m_PDU_Observable->send_PDU(buf, len);
    return -1;
}

Z_APDU *Yaz_Z_Assoc::decode_Z_PDU(const char *buf, int len)
{
    Z_APDU *apdu;

    odr_reset (m_odr_in);
    odr_setbuf (m_odr_in, (char*) buf, len, 0);

    if (!z_APDU(m_odr_in, &apdu, 0, 0))
    {
        logf(LOG_LOG, "ODR error on incoming PDU: %s [near byte %d] ",
             odr_errmsg(odr_geterror(m_odr_in)),
             odr_offset(m_odr_in));
        logf(LOG_LOG, "PDU dump:");
        odr_dumpBER(yaz_log_file(), buf, len);
        return 0;
    }
    else
    {
	if (m_APDU_file)
	    z_APDU(m_odr_print, &apdu, 0, "decode");
        return apdu;
    }
}

int Yaz_Z_Assoc::encode_Z_PDU(Z_APDU *apdu, char **buf, int *len)
{
    if (!z_APDU(m_odr_out, &apdu, 0, 0))
    {
	logf (LOG_LOG, "yaz_Z_Assoc::encode_Z_PDU failed");
        return -1;
    }
    if (m_APDU_file)
	z_APDU(m_odr_print, &apdu, 0, "encode");
    *buf = odr_getbuf (m_odr_out, len, 0);
    odr_reset (m_odr_out);
    return *len;
}

const char *Yaz_Z_Assoc::get_hostname()
{
    return m_hostname;
}

void Yaz_Z_Assoc::client(const char *addr)
{
    delete [] m_hostname;
    m_hostname = new char[strlen(addr)+1];
    strcpy (m_hostname, addr);
    m_PDU_Observable->connect (this, addr);
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

    if ( str )
    {
        id = (Z_ReferenceId*) odr_malloc (m_odr_out, sizeof(*id));
        id->size = id->len = strlen(str);
        id->buf = (unsigned char *) str;
    }

    return id;
}

