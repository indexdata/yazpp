/*
 * Copyright (c) 2000, Index Data.
 * See the file LICENSE for details.
 * 
 * $Log: yaz-z-server.cpp,v $
 * Revision 1.1  2000-09-08 10:23:42  adam
 * Added skeleton of yaz-z-server.
 *
 */

#include <yaz/log.h>
#include <yaz-z-server.h>


Yaz_Z_Server::Yaz_Z_Server(IYaz_PDU_Observable *the_PDU_Observable)
    : Yaz_Z_Assoc(the_PDU_Observable)
{
    m_no = 0;
}

void Yaz_Z_Server::recv_Z_PDU (Z_APDU *apdu)
{
    logf (LOG_LOG, "recv_Z_PDU in Yaz_Z_Server");
}

