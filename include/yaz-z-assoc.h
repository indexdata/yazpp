/*
 * Copyright (c) 1998-1999, Index Data.
 * See the file LICENSE for details.
 * Sebastian Hammer, Adam Dickmeiss
 * 
 * $Id: yaz-z-assoc.h,v 1.3 1999-04-21 12:09:01 adam Exp $
 */

#include <proto.h>
#include <odr.h>
#include <yaz-pdu-observer.h>

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
    void connectNotify();
    /// Failure notification
    void failNotify();
    /// Timeout notification
    void timeoutNotify();
    /// Timeout specify
    void timeout(int timeout);
    /// Begin Z39.50 client role
    void client(const char *addr);
    /// Begin Z39.50 server role
    void server(const char *addr);
    /// Close connection
    void close();
    /// Decode Z39.50 PDU.
    Z_APDU *decode_Z_PDU(const char *buf, int len);
    /// Encode Z39.50 PDU.
    int encode_Z_PDU(Z_APDU *apdu, char **buf, int *len);
    /// Send Z39.50 PDU
    int send_Z_PDU(Z_APDU *apdu);
    /// Receive Z39.50 PDU
    virtual void recv_Z_PDU(Z_APDU *apdu) = 0;
    /// Create Z39.50 PDU with reasonable defaults
    Z_APDU *create_Z_PDU(int type);
    /// Request Alloc
    ODR odr_encode ();
    ODR odr_decode ();
    ODR odr_print ();

    /// OtherInformation
    void get_otherInfoAPDU(Z_APDU *apdu, Z_OtherInformation ***oip);
    Z_OtherInformationUnit *update_otherInformation (
	Z_OtherInformation **otherInformationP, int createFlag,
	int *oid, int categoryValue);
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
    void set_apdu_log(const char *file);
 private:
    static int yaz_init_flag;
    static int yaz_init_func();
    IYaz_PDU_Observable *m_PDU_Observable;
    ODR m_odr_in;
    ODR m_odr_out;
    ODR m_odr_print;
};
