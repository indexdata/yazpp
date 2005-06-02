/*
 * Copyright (c) 1998-2003, Index Data.
 * See the file LICENSE for details.
 * 
 * $Id: pdu-assoc.h,v 1.6 2005-06-02 06:40:21 adam Exp $
 */

#ifndef YAZ_PDU_ASSOC_INCLUDED
#define YAZ_PDU_ASSOC_INCLUDED

#include <yaz/comstack.h>
#include <yaz++/socket-observer.h>
#include <yaz++/pdu-observer.h>

namespace yazpp_1 {
/** Simple Protocol Data Unit Assocation.
    This object sends - and receives PDU's using the COMSTACK
    network utility. To use the association in client role, use
    the method connect. The server role is initiated by using the
    listen method.
 */
class YAZ_EXPORT Yaz_PDU_Assoc : public IYaz_PDU_Observable, yazpp_1::IYazSocketObserver {
    friend class Yaz_PDU_AssocThread;
 private:
    enum { 
	Connecting,
	Listen,
	Ready,
	Closed,
	Writing,
	Accepting
    } m_state;
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
    COMSTACK m_cs;
    yazpp_1::IYazSocketObservable *m_socketObservable;
    IYaz_PDU_Observer *m_PDU_Observer;
    char *m_input_buf;
    int m_input_len;
    PDU_Queue *m_queue_out;
    PDU_Queue *m_queue_in;
    int Yaz_PDU_Assoc::flush_PDU();
    int *m_destroyed;
    int m_idleTime;
    int m_log;
    void init(yazpp_1::IYazSocketObservable *socketObservable);
 public:
    COMSTACK comstack(const char *type_and_host, void **vp);
    /// Create object using specified socketObservable
    Yaz_PDU_Assoc(yazpp_1::IYazSocketObservable *socketObservable);
    /// Create Object using existing comstack
    Yaz_PDU_Assoc(yazpp_1::IYazSocketObservable *socketObservable,
		  COMSTACK cs);
    /// Close socket and destroy object.
    /// virtual ~Yaz_PDU_Assoc();
    /// Clone the object
    IYaz_PDU_Observable *clone();
    /// Send PDU
    int send_PDU(const char *buf, int len);
    /// connect to server (client role)
    int connect(IYaz_PDU_Observer *observer, const char *addr);
    /// listen for clients (server role)
    int listen(IYaz_PDU_Observer *observer, const char *addr);
    /// Socket notification
    void socketNotify(int event);
    /// Close socket
    void close();
    /// Close and destroy
    void destroy();
    /// Set Idle Time
    void idleTime (int timeout);
    /// Child start...
    virtual void childNotify(COMSTACK cs);
    const char *getpeername();
};

class YAZ_EXPORT Yaz_PDU_AssocThread : public Yaz_PDU_Assoc {
 public:
    Yaz_PDU_AssocThread(yazpp_1::IYazSocketObservable *socketObservable);
 private:
    void childNotify(COMSTACK cs);

};
};

#endif

