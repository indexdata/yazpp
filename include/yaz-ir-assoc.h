/*
 * Copyright (c) 1998-1999, Index Data.
 * See the file LICENSE for details.
 * Sebastian Hammer, Adam Dickmeiss
 * 
 * $Id: yaz-ir-assoc.h,v 1.7 1999-04-21 12:09:01 adam Exp $
 */

#include <yaz-z-assoc.h>
#include <yaz-z-query.h>

/** Information Retrieval Assocation.
    This object implements the client - and server role of a generic
    Z39.50 Association.
*/
class YAZ_EXPORT Yaz_IR_Assoc: public Yaz_Z_Assoc {
 public:
    /// Create object using the PDU Observer specified
    Yaz_IR_Assoc(IYaz_PDU_Observable *the_PDU_Observable);
    /// Destroy assocation and close PDU Observer
    virtual ~Yaz_IR_Assoc();
    /// Receive Z39.50 PDU
    void recv_Z_PDU(Z_APDU *apdu);
    /// Set Database Names
    void set_databaseNames (int num, const char **list);
    void set_databaseNames(const char *dblist, const char *sep);
    /// Get Database Names
    void get_databaseNames (int *num, char ***list);

    void client(const char *addr);

    /// Set Preferred Record Syntax
    void set_preferredRecordSyntax (int value);
    void set_preferredRecordSyntax (const char *syntax);
    /// Get Preferred Record Syntax
    void get_preferredRecordSyntax (int *val);
    void get_preferredRecordSyntax (const char **syntax);

    /// Set ElementSetName
    void set_elementSetName (const char *elementSetName);
    /// Get ElementSetName
    void get_elementSetName (const char **elementSetName);
    void get_elementSetName (Z_ElementSetNames **elementSetNames);

    int get_lastReceived();
    void set_lastReceived(int lastReceived);

    /// Settings
    void set_proxy(const char *str);
    const char *get_proxy();
    const char *get_host();

    void set_cookie(const char *str);
    const char *get_cookie();

    /// Send Services
    int send_initRequest();
    int send_searchRequest(Yaz_Z_Query *query);
    int send_presentRequest(int start, int number);
    /// Recv Services
    virtual void recv_initRequest(Z_InitRequest *initRequest);
    virtual void recv_initResponse(Z_InitResponse *initResponse);
    virtual void recv_searchRequest(Z_SearchRequest *searchRequest);
    virtual void recv_presentRequest(Z_PresentRequest *presentRequest);
    virtual void recv_searchResponse(Z_SearchResponse *searchResponse);
    virtual void recv_presentResponse(Z_PresentResponse *presentResponse);
 private:
    char *m_proxy;
    char *m_host;
    char *m_cookie;
    int m_num_databaseNames;
    char **m_databaseNames;
    int m_preferredRecordSyntax;
    Z_ElementSetNames *m_elementSetNames;
    int m_lastReceived;
};
