/*
 * Copyright (c) 1998-2005, Index Data.
 * See the file LICENSE for details.
 * 
 * $Id: pdu-observer.h,v 1.1 2006-03-29 13:14:15 adam Exp $
 */

#ifndef YAZ_PDU_OBSERVER_H
#define YAZ_PDU_OBSERVER_H

#include <yaz/yconfig.h>

namespace yazpp_1 {

class IPDU_Observer;

/** Protocol Data Unit Observable.
    This interface implements a Protocol Data Unit (PDU) network driver.
    The PDU's is not encoded/decoded by this interface. They are simply
    transmitted/received over the network. To use this interface the
    IPDU_Observer interface must be implemented.
 */
class YAZ_EXPORT IPDU_Observable {
 public:
    /// Send encoded PDU buffer of specified length
    virtual int send_PDU(const char *buf, int len) = 0;
    /// Connect with server specified by addr.
    virtual int connect(IPDU_Observer *observer, const char *addr) = 0;
    /// Listen on address addr.
    virtual int listen(IPDU_Observer *observer, const char *addr) = 0;
    /// Close connection
    virtual void close() = 0;
    /// Make clone of this object using this interface
    virtual IPDU_Observable *clone() = 0;
    /// Destroy completely
    virtual void destroy() = 0;
    /// Set Idle Time
    virtual void idleTime (int timeout) = 0;
    /// Get peername
    virtual const char *getpeername() = 0;

    virtual ~IPDU_Observable();
};

/** Protocol Data Unit Observer.
    This interface is used together with the IPDU_Observable interface
    and acts as a callback interface for it.
 */
class YAZ_EXPORT IPDU_Observer {
 public:
    /// A PDU has been received
    virtual void recv_PDU(const char *buf, int len) = 0;
    /// Called when Iyaz_PDU_Observable::connect was successful.
    virtual void connectNotify() = 0;
    /// Called whenever the connection was closed
    virtual void failNotify() = 0;
    /// Called whenever there is a timeout
    virtual void timeoutNotify() = 0;
    /// Make clone of observer using IPDU_Observable interface
    virtual IPDU_Observer *sessionNotify(
        IPDU_Observable *the_PDU_Observable, int fd) = 0;

    virtual ~IPDU_Observer();
};
};

#endif
/*
 * Local variables:
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * End:
 * vim: shiftwidth=4 tabstop=8 expandtab
 */

