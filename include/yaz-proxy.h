/*
 * Copyright (c) 1998-1999, Index Data.
 * See the file LICENSE for details.
 * Sebastian Hammer, Adam Dickmeiss
 * 
 * $Log: yaz-proxy.h,v $
 * Revision 1.1.1.1  1999-01-28 09:41:07  adam
 * First implementation of YAZ++.
 *
 *
 */

#include <yaz-ir-assoc.h>

class Yaz_Proxy;

/// Private class
class Yaz_ProxyClient : public Yaz_IR_Assoc {
    friend Yaz_Proxy;
    Yaz_ProxyClient(IYaz_PDU_Observable *the_PDU_Observable);
    void recv_Z_PDU(Z_APDU *apdu);
    IYaz_PDU_Observer* clone(IYaz_PDU_Observable *the_PDU_Observable);
    Yaz_Proxy *m_server;
    void failNotify();
};

/// Private class
class Yaz_ProxyMap {
    friend Yaz_Proxy;
    char *m_databaseName;      /* from database name */
    char *m_ZURL;              /* to this address */
    Yaz_ProxyMap *m_next;
};

/// Information Retrieval Proxy Server.
class Yaz_Proxy : public Yaz_IR_Assoc {
 public:
    Yaz_Proxy(IYaz_PDU_Observable *the_PDU_Observable);
    ~Yaz_Proxy();
    void recv_Z_PDU(Z_APDU *apdu);
    IYaz_PDU_Observer* clone(IYaz_PDU_Observable *the_PDU_Observable);
    void failNotify();
    Yaz_ProxyClient *m_client;
    IYaz_PDU_Observable *m_PDU_Observable;
 private:
    Yaz_ProxyMap *m_maps;
};
