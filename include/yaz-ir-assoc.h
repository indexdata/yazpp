/*
 * Copyright (c) 1998-1999, Index Data.
 * See the file LICENSE for details.
 * Sebastian Hammer, Adam Dickmeiss
 * 
 * $Log: yaz-ir-assoc.h,v $
 * Revision 1.4  1999-03-23 14:17:57  adam
 * More work on timeout handling. Work on yaz-client.
 *
 * Revision 1.3  1999/02/02 14:01:12  adam
 * First WIN32 port of YAZ++.
 *
 * Revision 1.2  1999/01/28 13:08:39  adam
 * Yaz_PDU_Assoc better encapsulated. Memory leak fix in
 * yaz-socket-manager.cc.
 *
 * Revision 1.1.1.1  1999/01/28 09:41:07  adam
 * First implementation of YAZ++.
 *
 */


#include <proto.h>
#include <odr.h>
#include <yaz-pdu-observer.h>

/** Information Retrieval Assocation.
    This object implements the client - and server role of a generic
    Z39.50 Association.
*/
class YAZ_EXPORT Yaz_IR_Assoc : public IYaz_PDU_Observer {
 public:
    /// Create object using the PDU Observer specified
    Yaz_IR_Assoc(IYaz_PDU_Observable *the_PDU_Observable);
    /// Destroy assocation and close PDU Observer
    virtual ~Yaz_IR_Assoc();
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
 private:
    static int yaz_init_flag;
    static int yaz_init_func();
    IYaz_PDU_Observable *m_PDU_Observable;
    ODR m_odr_in;
    ODR m_odr_out;
    ODR m_odr_print;
};
