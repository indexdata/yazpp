/*
 * Copyright (c) 1998-1999, Index Data.
 * See the file LICENSE for details.
 * Sebastian Hammer, Adam Dickmeiss
 * 
 * $Log: yaz-pdu-assoc.h,v $
 * Revision 1.1.1.1  1999-01-28 09:41:07  adam
 * First implementation of YAZ++.
 *
 */

#include <comstack.h>
#include <yaz-socket-observer.h>
#include <yaz-pdu-observer.h>

/** Simple Protocol Data Unit Assocation.
    This object sends - and receives PDU's using the COMSTACK
    network utility. To use the association in client role, use
    the method connect. The server role is initiated by using the
    listen method.
 */
class Yaz_PDU_Assoc : public IYaz_PDU_Observable, IYazSocketObserver {
 private:
    enum { Connecting, Connected, Listen, Ready, Closed } m_state;
    class PDU_Queue {
    public:
	PDU_Queue(const char *buf, int len);
	PDU_Queue::~PDU_Queue();
	char *m_buf;
	int m_len;
	PDU_Queue *m_next;
    };
    Yaz_PDU_Assoc *m_parent;
    Yaz_PDU_Assoc *m_children;
    Yaz_PDU_Assoc *m_next;
    COMSTACK Yaz_PDU_Assoc::comstack();
    COMSTACK m_cs;
    IYazSocketObservable *m_socketObservable;
    IYaz_PDU_Observer *m_PDU_Observer;
    char *m_input_buf;
    int m_input_len;
    PDU_Queue *m_queue_out;
    int Yaz_PDU_Assoc::flush_PDU();
 public:
    /// Create object using specified socketObservable
    Yaz_PDU_Assoc(IYazSocketObservable *socketObservable, COMSTACK cs);
    /// Close socket and destroy object.
    virtual ~Yaz_PDU_Assoc();
    /// Clone the object
    IYaz_PDU_Observable *clone();
    /// Send PDU
    int send_PDU(const char *buf, int len);
    /// connect to server (client role)
    void connect(IYaz_PDU_Observer *observer, const char *addr);
    /// listen for clients (server role)
    void listen(IYaz_PDU_Observer *observer, const char *addr);
    /// Socket notification
    void socketNotify(int event);
    /// Close socket
    void close();
};
