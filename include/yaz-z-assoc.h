/*
 * Copyright (c) 1998-1999, Index Data.
 * See the file LICENSE for details.
 * Sebastian Hammer, Adam Dickmeiss
 * 
 * $Id: yaz-z-assoc.h,v 1.2 1999-04-20 10:30:05 adam Exp $
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
    Z_OtherInformationUnit *update_otherInformation (
	Z_OtherInformation **otherInformationP, int createFlag,
	int *oid, int categoryValue);
 private:
    static int yaz_init_flag;
    static int yaz_init_func();
    IYaz_PDU_Observable *m_PDU_Observable;
    ODR m_odr_in;
    ODR m_odr_out;
    ODR m_odr_print;
};
