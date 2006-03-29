/*
 * Copyright (c) 1998-2005, Index Data.
 * See the file LICENSE for details.
 * 
 * $Id: gdu.h,v 1.1 2006-03-29 13:14:15 adam Exp $
 */

#ifndef YAZPP_GDU_INCLUDED
#define YAZPP_GDU_INCLUDED

#include <yaz/yconfig.h>
#include <yaz/proto.h>

namespace yazpp_1 {
    class YAZ_EXPORT GDU {
    public:
        GDU(const GDU &);
        GDU(Z_GDU *gdu);
        GDU(Z_APDU *apdu);
        GDU();
        ~GDU();
        GDU &operator=(const GDU &);
        Z_GDU *get() const;
        void move_away_gdu(ODR dst, Z_GDU **gdu);
    private:
        void base(Z_GDU *gdu, ODR o);
        Z_GDU *m_gdu;
        ODR m_decode;
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

