/*
 * Copyright (c) 1998-1999, Index Data.
 * See the file LICENSE for details.
 * Sebastian Hammer, Adam Dickmeiss
 * 
 * $Id: yaz-pdu-observer.h,v 1.7 1999-12-06 13:52:45 adam Exp $
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
class YAZ_EXPORT IYaz_PDU_Observable {
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
    /// Destroy completely
    virtual void destroy() = 0;
    /// Set Idle Time
    virtual void idleTime (int timeout) = 0;
    /// open with existing socket
    virtual void socket(IYaz_PDU_Observer *observer, int fd) = 0;
};

/** Protocol Data Unit Observer.
    This interface is used together with the IYaz_PDU_Observable interface
    and acts as a callback interface for it.
 */
class YAZ_EXPORT IYaz_PDU_Observer {
 public:
    /// A PDU has been received
    virtual void recv_PDU(const char *buf, int len) = 0;
    /// Called when Iyaz_PDU_Observable::connect was successful.
    virtual void connectNotify() = 0;
    /// Called whenever the connection was closed
    virtual void failNotify() = 0;
    /// Called whenever there is a timeout
    virtual void timeoutNotify() = 0;
    /// Make clone of observer using IYaz_PDU_Observable interface
    virtual IYaz_PDU_Observer *clone(IYaz_PDU_Observable *the_PDU_Observable) = 0;
};

#endif
