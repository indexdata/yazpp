/*
 * Copyright (c) 1998-2000, Index Data.
 * See the file LICENSE for details.
 * 
 * $Id: z-assoc.h,v 1.4 2003-10-16 16:10:43 adam Exp $
 */

#ifndef YAZ_Z_ASSOC_INCLUDED
#define YAZ_Z_ASSOC_INCLUDED

#include <yaz/proto.h>
#include <yaz/odr.h>
#include <yaz++/pdu-observer.h>

/** Z39.50 Assocation.
    This object implements the client - and server role of a generic
    Z39.50 Association.
*/
class YAZ_EXPORT Yaz_Z_Assoc : public IYaz_PDU_Observer {
 public:
    /// Create object using the PDU Observer specified
    Yaz_Z_Assoc(IYaz_PDU_Observable *the_PDU_Observable);
    /// Destroy assocation and close PDU Observer
    virtual ~Yaz_Z_Assoc();
    /// Receive PDU
    void recv_PDU(const char *buf, int len);
    /// Connect notification
    virtual void connectNotify() = 0;
    /// Failure notification
    virtual void failNotify() = 0;
    /// Timeout notification
    virtual void timeoutNotify() = 0;
    /// Timeout specify
    void timeout(int timeout);
    /// Begin Z39.50 client role
    int client(const char *addr);
    /// Begin Z39.50 server role
    void server(const char *addr);
    /// Close connection
    void close();
    /// Decode Z39.50 PDU.
    Z_APDU *decode_Z_PDU(const char *buf, int len);
    /// Encode Z39.50 PDU.
    int encode_Z_PDU(Z_APDU *apdu, char **buf, int *len);
    /// Send Z39.50 PDU
    int send_Z_PDU(Z_APDU *apdu, int *len);
    /// Receive Z39.50 PDU
    virtual void recv_Z_PDU(Z_APDU *apdu, int len) = 0;
    /// Create Z39.50 PDU with reasonable defaults
    Z_APDU *create_Z_PDU(int type);
    /// Request Alloc
    ODR odr_encode ();
    ODR odr_decode ();
    ODR odr_print ();

    void set_APDU_log(const char *fname);
    const char *get_APDU_log();

    /// OtherInformation
    void get_otherInfoAPDU(Z_APDU *apdu, Z_OtherInformation ***oip);
    Z_OtherInformationUnit *update_otherInformation (
	Z_OtherInformation **otherInformationP, int createFlag,
	int *oid, int categoryValue, int deleteFlag);
    void set_otherInformationString (
	Z_OtherInformation **otherInformationP,
	int *oid, int categoryValue,
	const char *str);
    void set_otherInformationString (
	Z_OtherInformation **otherInformation,
	int oidval, int categoryValue,
	const char *str);
    void set_otherInformationString (
	Z_APDU *apdu,
	int oidval, int categoryValue,
	const char *str);

    Z_ReferenceId *getRefID(char* str);
    Z_ReferenceId **get_referenceIdP(Z_APDU *apdu);
    void transfer_referenceId(Z_APDU *from, Z_APDU *to);

    const char *get_hostname();

    int set_APDU_yazlog(int v);

 private:
    static int yaz_init_flag;
    static int yaz_init_func();
    IYaz_PDU_Observable *m_PDU_Observable;
    ODR m_odr_in;
    ODR m_odr_out;
    ODR m_odr_print;
    int m_log;
    FILE *m_APDU_file;
    char *m_APDU_fname;
    char *m_hostname;
    int m_APDU_yazlog;
};

#endif
