/*
 * Copyright (c) 1998-2003, Index Data.
 * See the file LICENSE for details.
 * 
 * $Id: proxy.h,v 1.20 2003-10-20 18:31:43 adam Exp $
 */

#include <yaz++/z-assoc.h>
#include <yaz++/z-query.h>
#include <yaz++/z-databases.h>

#if HAVE_XML2
#include <libxml/parser.h>
#include <libxml/tree.h>
#endif

class Yaz_Proxy;

#define MAX_ZURL_PLEX 10

#define PROXY_LOG_APDU_CLIENT 1
#define PROXY_LOG_APDU_SERVER 2
#define PROXY_LOG_REQ_CLIENT 4
#define PROXY_LOG_REQ_SERVER 8

struct Yaz_RecordCache_Entry;

class YAZ_EXPORT Yaz_ProxyConfig {
public:
    Yaz_ProxyConfig();
    ~Yaz_ProxyConfig();
    int read_xml(const char *fname);

    int get_target_no(int no,
		      const char **name,
		      const char **url,
		      int *limit_bw,
		      int *limit_pdu,
		      int *limit_req,
		      int *target_idletime,
		      int *client_idletime,
		      int *max_clients,
		      int *keepalive_limit_bw,
		      int *keepalive_limit_pdu,
		      int *pre_init);
    
    void get_generic_info(int *log_mask, int *max_clients);

    void get_target_info(const char *name, const char **url,
			 int *limit_bw, int *limit_pdu, int *limit_req,
			 int *target_idletime, int *client_idletime,
			 int *max_clients,
			 int *keepalive_limit_bw, int *keepalive_limit_pdu,
			 int *pre_init);

    int check_query(ODR odr, const char *name, Z_Query *query, char **addinfo);
    int check_syntax(ODR odr, const char *name,
		     Odr_oid *syntax, char **addinfo);
private:
    void operator=(const Yaz_ProxyConfig &conf);
#if HAVE_XML2
    int mycmp(const char *hay, const char *item, size_t len);
    xmlDocPtr m_docPtr;
    xmlNodePtr m_proxyPtr;
    void return_target_info(xmlNodePtr ptr, const char **url,
			    int *limit_bw, int *limit_pdu, int *limit_req,
			    int *target_idletime, int *client_idletime,
			    int *keepalive_limit_bw, int *keepalive_limit_pdu,
			    int *pre_init);
    void return_limit(xmlNodePtr ptr,
		      int *limit_bw, int *limit_pdu, int *limit_req);
    int check_type_1(ODR odr, xmlNodePtr ptr, Z_RPNQuery *query,
		     char **addinfo);
    xmlNodePtr find_target_node(const char *name);
    const char *get_text(xmlNodePtr ptr);
    int check_type_1_attributes(ODR odr, xmlNodePtr ptr,
				Z_AttributeList *attrs,
				char **addinfo);
    int check_type_1_structure(ODR odr, xmlNodePtr ptr, Z_RPNStructure *q,
			       char **addinfo);
#endif
    int m_copy;
    int match_list(int v, const char *m);
    int atoi_l(const char **cp);

};

class YAZ_EXPORT Yaz_RecordCache {
 public:
    Yaz_RecordCache ();
    ~Yaz_RecordCache ();
    void add (ODR o, Z_NamePlusRecordList *npr, int start, int hits);
    
    int lookup (ODR o, Z_NamePlusRecordList **npr, int start, int num,
		Odr_oid *syntax, Z_RecordComposition *comp);
    void clear();

    void copy_searchRequest(Z_SearchRequest *sr);
    void copy_presentRequest(Z_PresentRequest *pr);
    void set_max_size(int sz);
 private:
    NMEM m_mem;
    Yaz_RecordCache_Entry *m_entries;
    Z_SearchRequest *m_searchRequest;
    Z_PresentRequest *m_presentRequest;
    int match (Yaz_RecordCache_Entry *entry,
	       Odr_oid *syntax, int offset,
	       Z_RecordComposition *comp);
    int m_max_size;
};

class YAZ_EXPORT Yaz_bw {
 public:
    Yaz_bw(int sz);
    ~Yaz_bw();
    void add_bytes(int m);
    int get_total();
 private:
    long m_sec;   // time of most recent bucket
    int *m_bucket;
    int m_ptr;
    int m_size;
};

/// Private class
class YAZ_EXPORT Yaz_ProxyClient : public Yaz_Z_Assoc {
    friend class Yaz_Proxy;
    Yaz_ProxyClient(IYaz_PDU_Observable *the_PDU_Observable,
		    Yaz_Proxy *parent);
    ~Yaz_ProxyClient();
    void recv_Z_PDU(Z_APDU *apdu, int len);
    IYaz_PDU_Observer* sessionNotify
	(IYaz_PDU_Observable *the_PDU_Observable, int fd);
    void shutdown();
    Yaz_Proxy *m_server;
    void failNotify();
    void timeoutNotify();
    void connectNotify();
    int send_to_target(Z_APDU *apdu);
    const char *get_session_str();
    char *m_cookie;
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
    int m_resultSetStartPoint;
    int m_bytes_sent;
    int m_bytes_recv;
    int m_pdu_recv;
    ODR m_init_odr;
    Z_APDU *m_initResponse;
    Yaz_RecordCache m_cache;
    void pre_init_client();
    int m_target_idletime;
    Yaz_Proxy *m_root;
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
    int m_log_mask;
    int m_keepalive_limit_bw;
    int m_keepalive_limit_pdu;
    int m_client_idletime;
    int m_target_idletime;
    char *m_proxyTarget;
    char *m_default_target;
    char *m_proxy_authentication;
    long m_seed;
    char *m_optimize;
    int m_session_no;         // sequence for each client session
    char m_session_str[30];  // session string (time:session_no)
    Yaz_ProxyConfig *m_config;
    char *m_config_fname;
    int m_bytes_sent;
    int m_bytes_recv;
    int m_bw_max;
    Yaz_bw m_bw_stat;
    int m_pdu_max;
    Yaz_bw m_pdu_stat;
    Z_APDU *m_bw_hold_PDU;
    int m_max_record_retrieve;
    void handle_max_record_retrieve(Z_APDU *apdu);
    void display_diagrecs(Z_DiagRec **pp, int num);
    Z_Records *create_nonSurrogateDiagnostics(ODR o, int error,
					      const char *addinfo);

    Z_APDU *handle_query_validation(Z_APDU *apdu);
    Z_APDU *handle_syntax_validation(Z_APDU *apdu);
    const char *load_balance(const char **url);
    int m_reconfig_flag;
    Yaz_ProxyConfig *check_reconfigure();
    int m_request_no;
    int m_invalid_session;
    int m_marcxml_flag;
    void convert_to_marcxml(Z_NamePlusRecordList *p);
 public:
    Yaz_Proxy(IYaz_PDU_Observable *the_PDU_Observable,
	      Yaz_Proxy *parent = 0);
    ~Yaz_Proxy();
    void recv_Z_PDU(Z_APDU *apdu, int len);
    void recv_Z_PDU_0(Z_APDU *apdu);
    IYaz_PDU_Observer* sessionNotify
	(IYaz_PDU_Observable *the_PDU_Observable, int fd);
    void failNotify();
    void timeoutNotify();
    void connectNotify();
    const char *option(const char *name, const char *value);
    void set_default_target(const char *target);
    void set_proxy_authentication (const char *auth);
    char *get_proxy_target() { return m_proxyTarget; };
    char *get_session_str() { return m_session_str; };
    void set_max_clients(int m) { m_max_clients = m; };
    void set_client_idletime (int t) { m_client_idletime = (t > 1) ? t : 600; };
    void set_target_idletime (int t) { m_target_idletime = (t > 1) ? t : 600; };
    int get_target_idletime () { return m_target_idletime; }
    int set_config(const char *name);
    void reconfig() { m_reconfig_flag = 1; }
    int send_to_client(Z_APDU *apdu);
    void server(const char *addr);
    void pre_init();
    int get_log_mask() { return m_log_mask; };
};

