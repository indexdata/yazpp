/*
 * Copyright (c) 1998-1999, Index Data.
 * See the file LICENSE for details.
 * Sebastian Hammer, Adam Dickmeiss
 * 
 * $Log: yaz-pdu-observer.h,v $
 * Revision 1.1.1.1  1999-01-28 09:41:07  adam
 * First implementation of YAZ++.
 *
 */

#ifndef YAZ_PDU_OBSERVER_H
#define YAZ_PDU_OBSERVER_H

class IYaz_PDU_Observer;

/** Protocol Data Unit Observable.
    This interface implements a Protocol Data Unit (PDU) network driver.
    The PDU's is not encoded/decoded by this interface. They are simply
    transmitted/received over the network. To use this interface the
    IYaz_PDU_Observer interface must be implemented.
 */
class IYaz_PDU_Observable {
 public:
    /// Send encoded PDU buffer of specified length
    virtual int send_PDU(const char *buf, int len) = 0;
    /// Connect with server specified by addr.
    virtual void connect(IYaz_PDU_Observer *observer, const char *addr) = 0;
    /// Listen on address addr.
    virtual void listen(IYaz_PDU_Observer *observer, const char *addr) = 0;
    /// Close connection
    virtual void close() = 0;
    /// Make clone of this object using this interface
    virtual IYaz_PDU_Observable *clone() = 0;
};

/** Protocol Data Unit Observer.
    This interface is used together with the IYaz_PDU_Observable interface
    and acts as a callback interface for it.
 */
class IYaz_PDU_Observer {
 public:
    /// A PDU has been received
    virtual void recv_PDU(const char *buf, int len) = 0;
    /// Called when Iyaz_PDU_Observabvle::connect was successful.
    virtual void connectNotify() = 0;
    /// Called whenever the connection was closed
    virtual void failNotify() = 0;
    /// Make clone of observer using IYaz_PDU_Observable interface
    virtual IYaz_PDU_Observer *clone(IYaz_PDU_Observable *the_PDU_Observable) = 0;
};

#endif
