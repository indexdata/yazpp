/*
 * Copyright (c) 1998-1999, Index Data.
 * See the file LICENSE for details.
 * Sebastian Hammer, Adam Dickmeiss
 * 
 * $Log: yaz-ir-assoc.cpp,v $
 * Revision 1.2  1999-01-28 13:08:43  adam
 * Yaz_PDU_Assoc better encapsulated. Memory leak fix in
 * yaz-socket-manager.cc.
 *
 * Revision 1.1.1.1  1999/01/28 09:41:07  adam
 * First implementation of YAZ++.
 *
 */

#include <log.h>
#include <yaz-ir-assoc.h>

Yaz_IR_Assoc::Yaz_IR_Assoc(IYaz_PDU_Observable *the_PDU_Observable)
{
    m_PDU_Observable = the_PDU_Observable;
    m_odr_in = odr_createmem (ODR_DECODE);
    m_odr_out = odr_createmem (ODR_ENCODE);
    m_odr_print = odr_createmem (ODR_PRINT);
}

Yaz_IR_Assoc::~Yaz_IR_Assoc()
{
    m_PDU_Observable->destroy();
    delete m_PDU_Observable;
    odr_destroy (m_odr_print);
    odr_destroy (m_odr_out);
    odr_destroy (m_odr_in);
}

void Yaz_IR_Assoc::recv_PDU(const char *buf, int len)
{
    logf (LOG_LOG, "recv_PDU len=%d", len);
    Z_APDU *apdu = decode_Z_PDU (buf, len);
    if (apdu)
         recv_Z_PDU (apdu);
}

Z_APDU *Yaz_IR_Assoc::create_Z_PDU(int type)
{
    return zget_APDU(m_odr_out, type);
}

int Yaz_IR_Assoc::send_Z_PDU(Z_APDU *apdu)
{
    char *buf;
    int len;
    if (encode_Z_PDU(apdu, &buf, &len) > 0)
	return m_PDU_Observable->send_PDU(buf, len);
    return -1;
}

Z_APDU *Yaz_IR_Assoc::decode_Z_PDU(const char *buf, int len)
{
    Z_APDU *apdu;

    odr_reset (m_odr_in);
    odr_setbuf (m_odr_in, (char*) buf, len, 0);

    if (!z_APDU(m_odr_in, &apdu, 0))
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

int Yaz_IR_Assoc::encode_Z_PDU(Z_APDU *apdu, char **buf, int *len)
{
    if (!z_APDU(m_odr_out, &apdu, 0))
        return -1;
    *buf = odr_getbuf (m_odr_out, len, 0);
    odr_reset (m_odr_out);
    return *len;
}

void Yaz_IR_Assoc::connectNotify()
{
    logf (LOG_LOG, "connectNotify");
}

void Yaz_IR_Assoc::failNotify()
{
    logf (LOG_LOG, "failNotify");
}

void Yaz_IR_Assoc::client(const char *addr)
{
    m_PDU_Observable->connect (this, addr);
}

void Yaz_IR_Assoc::server(const char *addr)
{
    m_PDU_Observable->listen (this, addr);
}

