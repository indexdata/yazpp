/*
 * Copyright (c) 2000, Index Data.
 * See the file LICENSE for details.
 * 
 * $Id: yaz-z-server.h,v 1.1 2000-09-08 10:23:42 adam Exp $
 */

#include <yaz-z-assoc.h>

class YAZ_EXPORT Yaz_Z_Server : public Yaz_Z_Assoc {
public:
    Yaz_Z_Server(IYaz_PDU_Observable *the_PDU_Observable);
    virtual void recv_Z_PDU(Z_APDU *apdu);
private:
    int m_no;
};
