/*
 * Copyright (c) 1998-1999, Index Data.
 * See the file LICENSE for details.
 * Sebastian Hammer, Adam Dickmeiss
 * 
 * $Log: yaz-client.cpp,v $
 * Revision 1.4  1999-03-23 14:17:57  adam
 * More work on timeout handling. Work on yaz-client.
 *
 * Revision 1.3  1999/02/02 14:01:18  adam
 * First WIN32 port of YAZ++.
 *
 * Revision 1.2  1999/01/28 13:08:42  adam
 * Yaz_PDU_Assoc better encapsulated. Memory leak fix in
 * yaz-socket-manager.cc.
 *
 * Revision 1.1.1.1  1999/01/28 09:41:07  adam
 * First implementation of YAZ++.
 *
 */

#include <log.h>
#include <yaz-ir-assoc.h>
#include <yaz-pdu-assoc.h>
#include <yaz-socket-manager.h>
#include <yaz-z-query.h>

class YAZ_EXPORT MyClient : public Yaz_IR_Assoc {
public:
    MyClient(IYaz_PDU_Observable *the_PDU_Observable);
    void recv_Z_PDU(Z_APDU *apdu);
    IYaz_PDU_Observer *clone(IYaz_PDU_Observable *the_PDU_Observable);
    void init();
    void search(Yaz_Z_Query *query);
    void present(int start, int number);
    void set_databaseNames (int num, char **list);
    void set_syntax (const char *syntax);
    void set_elementSetName (const char *elementSetName);
private:
    int m_num_databaseNames;
    char **m_databaseNames;
    int m_recordSyntax;
    Z_ElementSetNames *m_elementSetNames;
};

void MyClient::recv_Z_PDU(Z_APDU *apdu)
{
    logf (LOG_LOG, "recv_APDU");
    switch (apdu->which)
    {
    case Z_APDU_initResponse:
        logf (LOG_LOG, "got InitResponse");
        break;
    case Z_APDU_searchResponse:
        logf (LOG_LOG, "got searchResponse");
        break;
    case Z_APDU_presentResponse:
        logf (LOG_LOG, "got presentResponse");
        break;
    }
}

IYaz_PDU_Observer *MyClient::clone(IYaz_PDU_Observable *the_PDU_Observable)
{
    return new MyClient(the_PDU_Observable);
}

MyClient::MyClient(IYaz_PDU_Observable *the_PDU_Observable) :
    Yaz_IR_Assoc (the_PDU_Observable)
{
    m_num_databaseNames = 0;
    m_databaseNames = 0;
    m_recordSyntax = VAL_NONE;
}

void MyClient::set_databaseNames (int num, char **list)
{
    int i;
    for (i = 0; i<m_num_databaseNames; i++)
	delete m_databaseNames[i];
    delete m_databaseNames;
    m_databaseNames = 0;
    m_num_databaseNames = num;
    m_databaseNames = new (char*) [num];
    for (i = 0; i<m_num_databaseNames; i++)
    {
	m_databaseNames[i] = new char[strlen(list[i])+1];
	strcpy (m_databaseNames[i], list[i]);
    }
}

void MyClient::set_syntax (const char *syntax)
{
    m_recordSyntax = VAL_NONE;
    if (syntax && *syntax)
	m_recordSyntax = oid_getvalbyname (syntax);
}

void MyClient::set_elementSetName (const char *elementSetName)
{
    if (m_elementSetNames)
	delete [] m_elementSetNames->u.generic;
    delete m_elementSetNames;
    m_elementSetNames = 0;
    if (elementSetName && *elementSetName)
    {
	m_elementSetNames = new Z_ElementSetNames;
	m_elementSetNames->which = Z_ElementSetNames_generic;
	m_elementSetNames->u.generic = new char[strlen(elementSetName)+1];
	strcpy (m_elementSetNames->u.generic, elementSetName);
    }
}

void MyClient::search(Yaz_Z_Query *query)
{
    Z_APDU *apdu = create_Z_PDU(Z_APDU_searchRequest);
    Z_SearchRequest *req = apdu->u.searchRequest;

    req->num_databaseNames = m_num_databaseNames;
    req->databaseNames = m_databaseNames;
    req->query = query->get_Z_Query();

    int oid_syntax[OID_SIZE];
    oident prefsyn;
    if (m_recordSyntax != VAL_NONE)
    {
	prefsyn.proto = PROTO_Z3950;
	prefsyn.oclass = CLASS_RECSYN;
	prefsyn.value = (enum oid_value) m_recordSyntax;
	oid_ent_to_oid(&prefsyn, oid_syntax);
	req->preferredRecordSyntax = oid_syntax;
    }
    send_Z_PDU(apdu);
}

void MyClient::present(int start, int number)
{
    Z_APDU *apdu = create_Z_PDU(Z_APDU_presentRequest);
    Z_PresentRequest *req = apdu->u.presentRequest;

    req->resultSetStartPoint = &start;
    req->numberOfRecordsRequested = &number;

    int oid_syntax[OID_SIZE];
    oident prefsyn;
    if (m_recordSyntax != VAL_NONE)
    {
	prefsyn.proto = PROTO_Z3950;
	prefsyn.oclass = CLASS_RECSYN;
	prefsyn.value = (enum oid_value) m_recordSyntax;
	oid_ent_to_oid(&prefsyn, oid_syntax);
	req->preferredRecordSyntax = oid_syntax;
    }
    Z_RecordComposition compo;
    if (m_elementSetNames)
    {
	req->recordComposition = &compo;
        compo.which = Z_RecordComp_simple;
        compo.u.simple = m_elementSetNames;
    }
    send_Z_PDU(apdu);
}

void MyClient::init()
{
    Z_APDU *apdu = create_Z_PDU(Z_APDU_initRequest);
    Z_InitRequest *req = apdu->u.initRequest;
    
    ODR_MASK_SET(req->options, Z_Options_search);
    ODR_MASK_SET(req->options, Z_Options_present);
    ODR_MASK_SET(req->options, Z_Options_namedResultSets);
    ODR_MASK_SET(req->options, Z_Options_triggerResourceCtrl);
    ODR_MASK_SET(req->options, Z_Options_scan);
    ODR_MASK_SET(req->options, Z_Options_sort);
    ODR_MASK_SET(req->options, Z_Options_extendedServices);
    ODR_MASK_SET(req->options, Z_Options_delSet);

    ODR_MASK_SET(req->protocolVersion, Z_ProtocolVersion_1);
    ODR_MASK_SET(req->protocolVersion, Z_ProtocolVersion_2);
    ODR_MASK_SET(req->protocolVersion, Z_ProtocolVersion_3);

    send_Z_PDU(apdu);
}

int main(int argc, char **argv)
{
    Yaz_SocketManager mySocketManager;
    Yaz_PDU_Assoc *some = new Yaz_PDU_Assoc(&mySocketManager, 0);

    MyClient z(some);

    z.client(argc < 2 ? "localhost:9999" : argv[1]);
    z.init();
    while (mySocketManager.processEvent() > 0)
	;
    return 0;
}
