/*
 * Copyright (c) 1998-2000, Index Data.
 * See the file LICENSE for details.
 * 
 * $Id: proxy.h,v 1.2 2002-10-23 10:15:17 adam Exp $
 */

#include <yaz++/z-assoc.h>
#include <yaz++/z-query.h>
#include <yaz++/z-databases.h>

class Yaz_Proxy;

/// Private class
class YAZ_EXPORT Yaz_ProxyClient : public Yaz_Z_Assoc {
    friend Yaz_Proxy;
    Yaz_ProxyClient(IYaz_PDU_Observable *the_PDU_Observable);
    ~Yaz_ProxyClient();
    void recv_Z_PDU(Z_APDU *apdu);
    IYaz_PDU_Observer* sessionNotify
	(IYaz_PDU_Observable *the_PDU_Observable, int fd);
    void shutdown();
    Yaz_Proxy *m_server;
    void failNotify();
    void timeoutNotify();
    void connectNotify();
    char m_cookie[32];
    Yaz_ProxyClient *m_next;
    Yaz_ProxyClient **m_prev;
    int m_init_flag;
    Yaz_Z_Query *m_last_query;
    Yaz_Z_Databases m_last_databases;
    char *m_last_resultSetId;
    int m_last_ok;
    int m_last_resultCount;
    int m_sr_transform;
    int m_seqno;
    int m_waiting;
    ODR m_init_odr;
    Z_APDU *m_initResponse;
};

/// Information Retrieval Proxy Server.
class YAZ_EXPORT Yaz_Proxy : public Yaz_Z_Assoc {
 private:
    char *get_cookie(Z_OtherInformation **otherInfo);
    char *get_proxy(Z_OtherInformation **otherInfo);
    Yaz_ProxyClient *get_client(Z_APDU *apdu);
    Z_APDU *result_set_optimize(Z_APDU *apdu);
    void shutdown();
    
    Yaz_ProxyClient *m_client;
    IYaz_PDU_Observable *m_PDU_Observable;
    Yaz_ProxyClient *m_clientPool;
    Yaz_Proxy *m_parent;
    int m_seqno;
    int m_max_clients;
    int m_keepalive;
    int m_idletime;
    char *m_proxyTarget;
    char *m_proxy_authentication;
    long m_seed;
    char *m_optimize;
 public:
    Yaz_Proxy(IYaz_PDU_Observable *the_PDU_Observable);
    ~Yaz_Proxy();
    void recv_Z_PDU(Z_APDU *apdu);
    IYaz_PDU_Observer* sessionNotify
	(IYaz_PDU_Observable *the_PDU_Observable, int fd);
    void failNotify();
    void timeoutNotify();
    void connectNotify();
    const char *option(const char *name, const char *value);
    void set_proxy_target(const char *target);
    void set_proxy_authentication (const char *auth);
    char *get_proxy_target() { return m_proxyTarget; };
    void set_max_clients(int m) { m_max_clients = m; };
    void set_idletime (int t) { m_idletime = (t > 1) ? t : 600; };
};

