/*
 * Copyright (c) 1998-1999, Index Data.
 * See the file LICENSE for details.
 * Sebastian Hammer, Adam Dickmeiss
 * 
 * $Log: yaz-z-assoc.cpp,v $
 * Revision 1.2  1999-04-20 10:30:05  adam
 * Implemented various stuff for client and proxy. Updated calls
 * to ODR to reflect new name parameter.
 *
 * Revision 1.1  1999/04/09 11:46:57  adam
 * Added object Yaz_Z_Assoc. Much more functional client.
 *
 */

#include <assert.h>

#include <log.h>
#include <yaz-z-assoc.h>

int Yaz_Z_Assoc::yaz_init_func()
{
    logf (LOG_LOG, "nmem_init");
    nmem_init();
    logf (LOG_LOG, "done");
    return 1;
}

int Yaz_Z_Assoc::yaz_init_flag = Yaz_Z_Assoc::yaz_init_func();

Yaz_Z_Assoc::Yaz_Z_Assoc(IYaz_PDU_Observable *the_PDU_Observable)
{
    m_PDU_Observable = the_PDU_Observable;
    m_odr_in = odr_createmem (ODR_DECODE);
    m_odr_out = odr_createmem (ODR_ENCODE);
    m_odr_print = odr_createmem (ODR_PRINT);
}

Yaz_Z_Assoc::~Yaz_Z_Assoc()
{
    m_PDU_Observable->destroy();
    delete m_PDU_Observable;
    odr_destroy (m_odr_print);
    odr_destroy (m_odr_out);
    odr_destroy (m_odr_in);
}

void Yaz_Z_Assoc::recv_PDU(const char *buf, int len)
{
    logf (LOG_LOG, "recv_PDU len=%d", len);
    Z_APDU *apdu = decode_Z_PDU (buf, len);
    if (apdu)
	recv_Z_PDU (apdu);
}

Z_APDU *Yaz_Z_Assoc::create_Z_PDU(int type)
{
    return zget_APDU(m_odr_out, type);
}

int Yaz_Z_Assoc::send_Z_PDU(Z_APDU *apdu)
{
    char *buf;
    int len;
    logf (LOG_LOG, "Yaz_Z_Assoc:send_Z_PDU");
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
        odr_dumpBER(log_file(), buf, len);
        return 0;
    }
    else
    {
        logf (LOG_LOG, "decoded ok");
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
    *buf = odr_getbuf (m_odr_out, len, 0);
    odr_reset (m_odr_out);
    return *len;
}

void Yaz_Z_Assoc::connectNotify()
{
    logf (LOG_LOG, "connectNotify");
}

void Yaz_Z_Assoc::failNotify()
{
    logf (LOG_LOG, "failNotify");
}

void Yaz_Z_Assoc::timeoutNotify()
{
    logf (LOG_LOG, "timeoutNotify");
}

void Yaz_Z_Assoc::client(const char *addr)
{
    m_PDU_Observable->connect (this, addr);
}

void Yaz_Z_Assoc::close()
{
    m_PDU_Observable->close ();
}

void Yaz_Z_Assoc::server(const char *addr)
{
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

Z_OtherInformationUnit *Yaz_Z_Assoc::update_otherInformation (
    Z_OtherInformation **otherInformationP, int createFlag,
    int *oid, int categoryValue)
{
    int i;
    Z_OtherInformation *otherInformation = *otherInformationP;
    if (!otherInformation)
    {
	if (!createFlag)
	    return 0;
	otherInformation = *otherInformationP = (Z_OtherInformation *)
	    odr_malloc (odr_encode(), sizeof(*otherInformation));
	otherInformation->num_elements = 0;
	otherInformation->list = (Z_OtherInformationUnit **)
	    odr_malloc (odr_encode(), 8*sizeof(*otherInformation));
	for (i = 0; i<8; i++)
	    otherInformation->list[i] = 0;
    }
    for (i = 0; i<otherInformation->num_elements; i++)
    {
	assert (otherInformation->list[i]);
	if (!oid)
	{
	    if (!otherInformation->list[i]->category)
		return otherInformation->list[i];
	}
	else
	{
	    if (otherInformation->list[i]->category &&
		categoryValue ==
		*otherInformation->list[i]->category->categoryValue &&
		!oid_oidcmp (oid, otherInformation->list[i]->category->
			     categoryTypeId))
		return otherInformation->list[i];
	}
    }
    if (!createFlag)
	return 0;
    otherInformation->list[i] = (Z_OtherInformationUnit*)
	odr_malloc (odr_encode(), sizeof(Z_OtherInformationUnit));
    if (oid)
    {
	otherInformation->list[i]->category = (Z_InfoCategory*)
	    odr_malloc (odr_encode(), sizeof(Z_InfoCategory));
	otherInformation->list[i]->category->categoryTypeId = (int*)
	    odr_oiddup (odr_encode(), oid);
	otherInformation->list[i]->category->categoryValue = (int*)
	    odr_malloc (odr_encode(), sizeof(int));
	*otherInformation->list[i]->category->categoryValue =
	    categoryValue;
    }
    else
	otherInformation->list[i]->category = 0;
    otherInformation->list[i]->which = Z_OtherInfo_characterInfo;
    otherInformation->list[i]->information.characterInfo = 0;
    
    otherInformation->num_elements = i+1;
    return otherInformation->list[i];
}

