/*
 * Copyright (c) 2004, Index Data.
 * See the file LICENSE for details.
 * 
 * $Id: zlint.cpp,v 1.1 2004-02-18 22:11:41 adam Exp $
 */

#include <yaz/pquery.h>
#include <yaz/options.h>
#include <yaz/otherinfo.h>
#include <yaz/charneg.h>
#include <yaz/sortspec.h>
#include <yaz/log.h>
#include <yaz++/pdu-assoc.h>
#include <yaz++/socket-manager.h>
#include <yaz++/z-assoc.h>

#define REFID_BUF1 "zlint\000check1"
#define REFID_LEN1 12
#define REFID_BUF2 "zlint\000check2"
#define REFID_LEN2 12

enum test_code {
    TEST_FINISHED,
    TEST_CONTINUE,
};

#if 0
class Zlint;
class Zlint_test {
public:
    virtual void init_send(Zlint *z) = 0;
    virtual test_code init_recv(Zlint *z, Z_InitResponse *ir) = 0;
    virtual test_code other_recv(Zlint *z, Z_APDU *ir, Z_InitResponse *ir) = 0;
};
#endif

const char *try_syntax [] = {
    "usmarc",
    "unimarc",
    "danmarc",
    "sutrs",
    "grs1",
    "xml",
    "normarc",
    0
};
const char *try_query[] = {
    "@attr 1=4 petersson",
    "@attr 1=1016 petersson",
    "@attr 1=4 kingdom",
    "@attr 1=1016 kingdom",
    "@attr 1=62 sword",
    "sword"
    "seven",
    "@attr 1=4 water",
    "@attr 1=1016 water",
    "computer",
    "@attr 1=4 computer",
    "@attr 1=1016 computer",
    "water",
    "join",
    "about",
    "map",
    0,
};

const char *try_sort [] = {
    "1=4 <",
    "1=4 >",
    "1=62 >",
    "1=62 <",
    0
};
const char *try_scan [] = {
    "@attr 1=4 ab",
    "@attr 1=1003 ab",
    "@attr 1=1016 ab",
    0
};

class Zlint : public Yaz_Z_Assoc {
    int m_tst_no;

    int m_subtst_no;

    int m_query_no;
    int m_scan_no;
    int m_sort_no;
    int m_record_syntax_no;
    int m_got_result_set;
    IYaz_PDU_Observable *m_PDU_Observable;
    char *m_host;
    char *m_database;
    int m_timeout_init;
    int m_timeout_connect;
    int m_protocol_version;
    char m_session_str[20];
    int initResponseGetVersion(Z_InitResponse *init);
public:
    void prepare();
    void recv_GDU(Z_GDU *apdu, int len);
    Zlint(IYaz_PDU_Observable *the_PDU_Observable);
    void args(int argc, char **argv);
    void connectNotify();
    void failNotify();
    void closeNextTest();
    void sendTest();
    int nextTest();
    void testContinue();
    void timeoutNotify();
    IYaz_PDU_Observer *sessionNotify(
	IYaz_PDU_Observable *the_PDU_Observable, int fd);
    void connect();
    Z_ReferenceId *mk_refid(const char *buf, int len);
};

int Zlint::initResponseGetVersion(Z_InitResponse *init)
{
    int no = 0;
    int off = 0;
    int i;
    for (i = 0; i<12; i++)
	if (ODR_MASK_GET(init->protocolVersion, no))
	{
	    no = i+1;
	    if (off)
		yaz_log(LOG_WARN, "%sbad formatted version");
	}
	else
	    off = 1;
    return no;
}

Z_ReferenceId *Zlint::mk_refid(const char *buf, int len)
{
    Z_ReferenceId *id = 
	(Z_ReferenceId *) odr_malloc(odr_encode(), sizeof(*id));
    id->size = id->len = len;
    id->buf = (unsigned char*) odr_malloc(odr_encode(), len);
    memcpy(id->buf, buf, len);
    return id;
}

void Zlint::recv_GDU(Z_GDU *gdu, int len)
{
    yaz_log(LOG_LOG, "%sgot PDU", m_session_str);
    if (gdu->which != Z_GDU_Z3950)
    {
	yaz_log(LOG_LOG, "%sreceived non-Z39.50 response", m_session_str);
	closeNextTest();
    }
    if (gdu->u.z3950 && gdu->u.z3950->which == Z_APDU_initResponse)
    {
	int i;
	Z_InitResponse *init = gdu->u.z3950->u.initResponse;
	int ver = initResponseGetVersion(init);
	int result = init->result ? *init->result : 0;
	if (!result)
	    yaz_log(LOG_WARN, "%sinit rejected");
	switch(m_tst_no)
	{
	case 0:
	    if (ver > 3 || ver < 2)
		yaz_log(LOG_WARN, "%sgot version %d, expected 2 or 3",
			m_session_str, ver);
	    m_protocol_version = ver;
	    if (!result)
		closeNextTest();
	    else
	    {
		close();
		nextTest();
		connect();
	    }
	    break;
	case 1:
	    if (ver != 2)
		yaz_log(LOG_WARN, "%sgot version %d, expected 2",
			m_session_str, ver);
	    closeNextTest();
	    break;
	case 2:
	    if (ver < 2 || ver > 5)
		yaz_log(LOG_WARN, "%sgot version %d, expected 2-5",
			m_session_str,ver);
	    closeNextTest();
	    break;
	case 3:
	    if (!init->referenceId)
		yaz_log(LOG_WARN, "%smissing referenceID from init response",
			m_session_str);
	    else if (init->referenceId->len != REFID_LEN1
		     || memcmp(init->referenceId->buf, REFID_BUF1, REFID_LEN1))
		yaz_log(LOG_WARN, "%sreference ID does not match");
	    closeNextTest();
	    break;
	case 4:
	    if (m_subtst_no == 0)
	    {
		if (!init->referenceId)
		    yaz_log(LOG_WARN, "%smissing referenceID from first init response",
			    m_session_str);
		else if (init->referenceId->len != REFID_LEN1
			 || memcmp(init->referenceId->buf, REFID_BUF1, REFID_LEN1))
		    yaz_log(LOG_WARN, "%sreference ID does not match");
		m_subtst_no++;
	    }
	    else
	    {
		if (!init->referenceId)
		    yaz_log(LOG_WARN, "%smissing referenceID from second init response",
			    m_session_str);
		else if (init->referenceId->len != REFID_LEN2
			 || memcmp(init->referenceId->buf, REFID_BUF2, REFID_LEN2))
		    yaz_log(LOG_WARN, "%sreference ID does not match");
		closeNextTest();
	    }	    
	    break;
	case 5:
	    if (init->options)
	    {
		int i;
		int no_set = 0;
		int no_reset = 0;
		for (i = 0; i <= 24; i++)
		    if (ODR_MASK_GET(init->options, i))
			no_set++;
		    else
			no_reset++;
		if (no_set < 2)
		    yaz_log(LOG_WARN, "%ssuspicuously few option bits set",
			    m_session_str);
		if (no_reset == 0)
		    yaz_log(LOG_WARN, "%ssuspicuously many option bits set",
			    m_session_str);
	    }
	    closeNextTest();
	    break;
	case 6:
	    if (ODR_MASK_GET(init->options, Z_Options_negotiationModel))
	    {
		Z_CharSetandLanguageNegotiation *p =
		    yaz_get_charneg_record(init->otherInfo);
		
		if (p) {
		    
		    char *charset=NULL, *lang=NULL;
		    int selected;
		    NMEM m = nmem_create();
		    
		    yaz_get_response_charneg(m, p, &charset, &lang,
					     &selected);
		    yaz_log(LOG_LOG, "%sAccepted character set : %s",
			    m_session_str, charset);
		    yaz_log(LOG_LOG, "%sAccepted code language : %s",
			    m_session_str, lang ? lang : "none");
		    yaz_log(LOG_LOG, "%sAccepted records in ...: %d",
			    m_session_str, selected );
		    nmem_destroy(m);
		}
	    }
	    closeNextTest();
	    break;
	case 7:
	    if (m_subtst_no * m_subtst_no * 100000 + 2000 < *init->maximumRecordSize)
		yaz_log(LOG_WARN, "%smaximumRecordSize bigger than proposed size");

	    if (m_subtst_no * m_subtst_no * 100000 + 2000 < *init->preferredMessageSize)
		yaz_log(LOG_WARN, "%smaximumRecordSize bigger than proposed size");
	    if (m_subtst_no < 3)
	    {
		close();
		m_subtst_no++;
		connect();
	    }
	    else
		closeNextTest();
	    break;
	case 9:
	    if (result && ODR_MASK_GET(init->options, Z_Options_scan))
		sendTest();
	    else
		closeNextTest();
	    break;
	case 10:
	    if (result && ODR_MASK_GET(init->options, Z_Options_sort))
		sendTest();
	    else
		closeNextTest();
	    break;
	default:
	    if (result)
		sendTest();
	    else
		closeNextTest();
	}
    }
    else if (gdu->u.z3950 && gdu->u.z3950->which == Z_APDU_searchResponse)
    {
	Z_SearchResponse *sr = gdu->u.z3950->u.searchResponse;
	switch(m_tst_no)
	{
	case 8:
	    if (sr->records && (sr->records->which == Z_Records_NSD 
				|| 
				sr->records->which == Z_Records_multipleNSD))
	    {
		yaz_log(LOG_WARN, "%sSearch Error", m_session_str);
		m_query_no++;
		sendTest();
	    }
	    else if (!sr->resultCount || *sr->resultCount == 0)
	    {
		m_query_no++;
		sendTest();
	    }
	    else
	    {
		yaz_log(LOG_LOG, "%sgot %d result count with %s",
			m_session_str, *sr->resultCount,
			try_query[m_query_no]);
		m_got_result_set = 1;
		sendTest();
	    }
	    break;
	default:
	    closeNextTest();
	}
    }
    else if (gdu->u.z3950 && gdu->u.z3950->which == Z_APDU_presentResponse)
    {
	Z_PresentResponse *sr = gdu->u.z3950->u.presentResponse;
	switch(m_tst_no)
	{
	case 8:
	    if (sr->records && (sr->records->which == Z_Records_NSD 
				|| 
				sr->records->which == Z_Records_multipleNSD))
	    {
		yaz_log(LOG_LOG, "%spresent returned NSD for %s",
			m_session_str, try_syntax[m_record_syntax_no]);
	    }
	    else if (sr->records && sr->records->which == Z_Records_DBOSD
		     && sr->records->u.databaseOrSurDiagnostics->num_records>0
		     && sr->records->u.databaseOrSurDiagnostics->records[0])
	    {
		if (sr->records->u.databaseOrSurDiagnostics->records[0]->which == Z_NamePlusRecord_databaseRecord)
		{
		    Z_External *ext = sr->records->u.databaseOrSurDiagnostics->records[0]->u.databaseRecord;
		    Odr_oid *expectRecordSyntax =
			yaz_str_to_z3950oid(odr_decode(), CLASS_RECSYN,
					    try_syntax[m_record_syntax_no]);
		    if (oid_oidcmp(expectRecordSyntax,
			       ext->direct_reference))
			yaz_log(LOG_WARN, "%spresent bad record type for %s",
				m_session_str,
				try_syntax[m_record_syntax_no]);
		    else
			yaz_log(LOG_LOG, "%spresent OK for %s", m_session_str,
				try_syntax[m_record_syntax_no]);
		}
		else if (sr->records->u.databaseOrSurDiagnostics->records[0]->which == Z_NamePlusRecord_surrogateDiagnostic)
		    yaz_log(LOG_LOG, "%spresent returned SD %s", m_session_str,
			    try_syntax[m_record_syntax_no]);
		else
		    yaz_log(LOG_WARN, "%spresent returned fragment %s",
			    m_session_str,
			    try_syntax[m_record_syntax_no]);
	    }
	    else
	    {
		yaz_log(LOG_WARN, "%spresent returned no records or diagnostics", m_session_str);
		
	    }
	    m_record_syntax_no++;
	    sendTest();
	}
    }
    else if (gdu->u.z3950 && gdu->u.z3950->which == Z_APDU_scanResponse)
    {
	Z_ScanResponse *sr =  gdu->u.z3950->u.scanResponse;
	switch(m_tst_no)
	{
	case 9:
	    if (sr->entries->nonsurrogateDiagnostics)
	    {
		yaz_log(LOG_LOG, "%sscan NSD for %s", m_session_str,
			try_scan[m_scan_no]);
		m_scan_no++;
		sendTest();
	    }
	    else if (sr->entries->entries && sr->entries->num_entries > 0)
	    {
		yaz_log(LOG_LOG, "%sscan OK for %s", m_session_str,
			try_scan[m_scan_no]);
		closeNextTest();
	    }
	    else
	    {
		yaz_log(LOG_WARN, "%sscan no entries/diagnostics for %s",
			m_session_str,
			try_scan[m_scan_no]);
		m_scan_no++;
		sendTest();
	    }
	    break;
	default:
	    closeNextTest();
	}
    }
    else if (gdu->u.z3950 && gdu->u.z3950->which == Z_APDU_sortResponse)
    {
	Z_SortResponse *sr =  gdu->u.z3950->u.sortResponse;
	switch(m_tst_no)
	{
	case 10:
	    if (sr->diagnostics)
	    {
		yaz_log(LOG_LOG, "%ssort NSD for %s", m_session_str,
			try_sort[m_sort_no]);
		m_sort_no++;
		sendTest();
	    }
	    else
	    {
		yaz_log(LOG_LOG, "%ssort OK for %s", m_session_str,
			try_sort[m_sort_no]);
		closeNextTest();
	    }
	    break;
	default:
	    closeNextTest();
	}
    }
    else
	closeNextTest();
}

Zlint::Zlint(IYaz_PDU_Observable *the_PDU_Observable) : 
    Yaz_Z_Assoc(the_PDU_Observable)
{
    m_PDU_Observable = the_PDU_Observable;
    m_host = 0;
    m_database = 0;
    m_timeout_connect = 30;
    m_timeout_init = 30;
    m_tst_no = 0;
    m_subtst_no = 0;
    m_protocol_version = 0;
    sprintf(m_session_str, "%d ", m_tst_no);
}

void Zlint::connectNotify()
{
    Z_APDU *apdu = create_Z_PDU(Z_APDU_initRequest);
    Z_InitRequest *init = apdu->u.initRequest;
    int len;
    Z_OtherInformation **oi;

    timeout(m_timeout_init);

    switch(m_tst_no)
    {
    case 0:
	/* check if target properly negotiates to v3 .. */
	ODR_MASK_ZERO(init->protocolVersion);
	ODR_MASK_SET(init->protocolVersion, Z_ProtocolVersion_1);
	ODR_MASK_SET(init->protocolVersion, Z_ProtocolVersion_2);
	ODR_MASK_SET(init->protocolVersion, Z_ProtocolVersion_3);
	break;
    case 1:
	/* check if target properly negotiates to v2 .. */
	ODR_MASK_ZERO(init->protocolVersion);
	ODR_MASK_SET(init->protocolVersion, Z_ProtocolVersion_1);
	ODR_MASK_SET(init->protocolVersion, Z_ProtocolVersion_2);
	break;
    case 2:
	/* check latest version of target  - up to v9 */
	ODR_MASK_ZERO(init->protocolVersion);
	int i;
	for (i = 0; i< 9; i++)
	    ODR_MASK_SET(init->protocolVersion, i);
	break;
    case 3:
	/* send refID in init request */
	ODR_MASK_SET(init->protocolVersion, Z_ProtocolVersion_3);
	init->referenceId = mk_refid(REFID_BUF1, REFID_LEN1);
	break;
    case 4:
	/* send double init with differnet refID's */
	ODR_MASK_SET(init->protocolVersion, Z_ProtocolVersion_3);
	ODR_MASK_SET(init->options, Z_Options_concurrentOperations);
	init->referenceId = mk_refid(REFID_BUF1, REFID_LEN1);
	send_Z_PDU(apdu, &len);
	
	apdu = create_Z_PDU(Z_APDU_initRequest);
	init = apdu->u.initRequest;

	ODR_MASK_SET(init->protocolVersion, Z_ProtocolVersion_3);
	ODR_MASK_SET(init->options, Z_Options_concurrentOperations);

	init->referenceId = mk_refid(REFID_BUF2, REFID_LEN2);
	break;
    case 5:
	/* set all options.. see what target really supports .. */
	ODR_MASK_SET(init->protocolVersion, Z_ProtocolVersion_3);
	ODR_MASK_ZERO(init->options);
	for (i = 0; i <= 24; i++)
	    ODR_MASK_SET(init->options, i);
	break;
    case 6:
	ODR_MASK_SET(init->protocolVersion, Z_ProtocolVersion_3);
	yaz_oi_APDU(apdu, &oi);
	if (oi)
	{
	    Z_OtherInformationUnit *p0;
	    const char *negotiationCharset[] = {
		"UTF-8",
		"UTF-16",
		"UCS-2",
		"UCS-4",
		"ISO-8859-1"
	    };
	    char *yazLang = 0;

	    if ((p0=yaz_oi_update(oi, odr_encode(), NULL, 0, 0))) {
    		ODR_MASK_SET(init->options, Z_Options_negotiationModel);

    		p0->which = Z_OtherInfo_externallyDefinedInfo;
    		p0->information.externallyDefinedInfo =

		    yaz_set_proposal_charneg(
			odr_encode(),
			negotiationCharset, 5,
			(const char**)&yazLang, yazLang ? 1 : 0, 1);
	    }
	}
	break;
    case 7:
	*init->maximumRecordSize = m_subtst_no * m_subtst_no* 100000 + 2000;
	*init->preferredMessageSize = m_subtst_no * m_subtst_no *100000 + 2000;
	break;
    case 8:
	/* search */
	ODR_MASK_SET(init->protocolVersion, Z_ProtocolVersion_3);
	ODR_MASK_SET(init->options, Z_Options_namedResultSets);
	break;
    case 9:
	/* scan */
	ODR_MASK_SET(init->protocolVersion, Z_ProtocolVersion_3);
	ODR_MASK_SET(init->options, Z_Options_namedResultSets);
	ODR_MASK_SET(init->options, Z_Options_scan);
	break;
    case 10:
	/* sort */
	ODR_MASK_SET(init->protocolVersion, Z_ProtocolVersion_3);
	ODR_MASK_SET(init->options, Z_Options_namedResultSets);
	ODR_MASK_SET(init->options, Z_Options_sort);
	break;
    }
    int r = send_Z_PDU(apdu, &len);
}

int Zlint::nextTest()
{
    m_subtst_no = 0;
    while(m_tst_no >= 0)
    {
	m_tst_no++;
	sprintf(m_session_str, "%d ", m_tst_no);
	switch(m_tst_no)
	{
	case 0:
	    return 1;
	case 1:
	    return 1;
	case 2:
	    return 1;
	case 3:
	    return 1;
	case 4:
	    return 1;
	case 5:
	    return 1;
	case 6:
	    return 1;
	case 7:
	    return 1;
	case 8:
	    m_query_no = 0;
	    m_record_syntax_no = 0;
	    m_got_result_set = 0;
	    return 1;
	case 9:
	    m_scan_no = 0;
	    return 1;
	case 10:
	    m_sort_no = 0;
	    return 1;
	default:
	    m_tst_no = -1;
	}
    }
    return 0;
}

// current test failed badly - goto next or stop..
void Zlint::closeNextTest()
{
    close();
    if (m_tst_no != 0)
    {
	nextTest();
	connect();
    }
}

void Zlint::failNotify()
{
    yaz_log(LOG_WARN, "%sconnection closed by foreign host", m_session_str);
    testContinue();
}

void Zlint::timeoutNotify()
{
    yaz_log(LOG_WARN, "%sconnection timed out", m_session_str);
    testContinue();
}

void Zlint::testContinue()
{
    close();
    switch(m_tst_no)
    {
    case 8:
	if (m_got_result_set)
	{
	    // must search again to establish.. keep query
	    m_got_result_set = 0;
	    m_record_syntax_no++;
	}
	else
	{
	    // try new search ..
	    m_query_no++;
	}
	connect();
	return;
    case 9:
	m_scan_no++;
	connect();
	return;
    }
    nextTest();
}

void Zlint::sendTest()
{
    Z_APDU *apdu;
    switch(m_tst_no)
    {
    case 8:
	if (m_got_result_set == 0)
	{
	    apdu = zget_APDU(odr_encode(), Z_APDU_searchRequest);
	    Z_SearchRequest *sr;
	    sr = apdu->u.searchRequest;
	    sr->query = (Z_Query *) odr_malloc(odr_encode(), sizeof(*sr->query));
	    if (try_query[m_query_no] && sr)
	    {
		sr->query->which = Z_Query_type_1;
		Z_RPNQuery *rpn;
		YAZ_PQF_Parser pqf_parser = yaz_pqf_create ();
		
		sr->databaseNames = &m_database;
		sr->num_databaseNames = 1;
		
		rpn = yaz_pqf_parse(pqf_parser, odr_encode(), try_query[m_query_no]);

		yaz_pqf_destroy (pqf_parser);

		if (rpn)
		{
		    int len;
		    yaz_log(LOG_LOG, "%spqf: %s",
			    m_session_str, try_query[m_query_no]);

		    sr->query->u.type_1 = rpn;
		    send_Z_PDU(apdu, &len);
		}
		else
		    closeNextTest();
	    }
	    else
	    {
		yaz_log(LOG_WARN, "%sunable to get any hit count", 
			m_session_str);
		closeNextTest();
	    }
	}
	else if (m_got_result_set && try_syntax[m_record_syntax_no])
	{
	    int len;
	    apdu = zget_APDU(odr_encode(), Z_APDU_presentRequest);
	    Z_PresentRequest *pr = apdu->u.presentRequest;
	    *pr->numberOfRecordsRequested = 1;
	    *pr->resultSetStartPoint = 1;
	    
	    pr->preferredRecordSyntax =
		yaz_str_to_z3950oid(odr_encode(), CLASS_RECSYN,
				    try_syntax[m_record_syntax_no]);
	    send_Z_PDU(apdu, &len);
	}
	else
	    closeNextTest();
 	break;
    case 9:
	apdu = zget_APDU(odr_encode(), Z_APDU_scanRequest);
	if (apdu && try_scan[m_scan_no])
	{
	    int len;
	    YAZ_PQF_Parser pqf_parser = yaz_pqf_create ();
	    Z_ScanRequest *sr = apdu->u.scanRequest;
	    sr->termListAndStartPoint = yaz_pqf_scan(pqf_parser,
						     odr_encode(),
						     &sr->attributeSet,
						     try_scan[m_scan_no]);

	    sr->databaseNames = &m_database;
	    sr->num_databaseNames = 1;

	    yaz_pqf_destroy (pqf_parser);
	    send_Z_PDU(apdu, &len);
	}
	else
	    closeNextTest();
	break;
    case 10:
	apdu = zget_APDU(odr_encode(), Z_APDU_sortRequest);
	if (apdu && try_sort[m_sort_no])
	{
	    char *setstring = "default";
	    int len;
	    Z_SortRequest *sr = apdu->u.sortRequest;

	    sr->num_inputResultSetNames = 1;
	    sr->num_inputResultSetNames = 1;
	    sr->inputResultSetNames = (Z_InternationalString **)
		odr_malloc (odr_encode(), sizeof(*sr->inputResultSetNames));
	    sr->inputResultSetNames[0] = odr_strdup (odr_encode(), setstring);
	    sr->sortedResultSetName = odr_strdup(odr_encode(), setstring);
	    sr->sortSequence = yaz_sort_spec(odr_encode(), try_sort[m_sort_no]);
	    send_Z_PDU(apdu, &len);
	}
	else
	    closeNextTest();
	break;
    default:
	closeNextTest();
    }
}

IYaz_PDU_Observer *Zlint::sessionNotify(
    IYaz_PDU_Observable *the_PDU_Observable, int fd)
{
    return 0;
}

void Zlint::connect()
{
    if (m_host && m_tst_no != -1)
    {
	yaz_log(LOG_LOG, "%sconnecting to %s", m_session_str, m_host);
	timeout(m_timeout_connect);
	client(m_host);
    }
}

void Zlint::args(int argc, char **argv)
{
    char *arg;
    int ret;
    while ((ret = options("v", argv, argc, &arg)) != -2)
    {
        switch (ret)
	{
	case 'v':
	    break;
	case 0:
	    if (arg)
	    {
		const char *basep;
		m_host = xstrdup(arg);
		cs_get_host_args(m_host, &basep);
		if (!basep || !*basep)
		    basep = "Default";
		m_database = xstrdup(basep);
	    }
	    break;
	}
    }
}

int main(int argc, char **argv)
{
    Yaz_SocketManager mySocketManager;
    Zlint z(new Yaz_PDU_Assoc(&mySocketManager));
    
    z.args(argc, argv);

    z.connect();
    while (mySocketManager.processEvent() > 0)
	;
    exit (0);
}
