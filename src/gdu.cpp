/* This file is part of the yazpp toolkit.
 * Copyright (C) Index Data 
 * See the file LICENSE for details.
 */

#if HAVE_CONFIG_H
#include <config.h>
#endif
#include <yazpp/gdu.h>

using namespace yazpp_1;

GDU::GDU(Z_APDU *apdu)
{
    ODR encode = odr_createmem(ODR_ENCODE);
    Z_GDU *gdu = (Z_GDU *) odr_malloc(encode, sizeof(*gdu));
    gdu->which = Z_GDU_Z3950;
    gdu->u.z3950 = apdu;
    base(gdu, encode);
}

GDU::GDU(Z_GDU *gdu)
{
    base(gdu, odr_createmem(ODR_ENCODE));
}

GDU::GDU()
{
    base(0, odr_createmem(ODR_ENCODE));
}

GDU::GDU(const GDU &g)
{
    base(g.get(), odr_createmem(ODR_ENCODE));
}

void GDU::base(Z_GDU *gdu, ODR encode)
{
    m_decode = odr_createmem(ODR_DECODE);
    m_gdu = 0;
    if (gdu && z_GDU(encode, &gdu, 0, "encode"))
    {
        int len;
        char *buf = odr_getbuf(encode, &len, 0);

        odr_setbuf(m_decode, buf, len, 0);
        if (!z_GDU(m_decode, &m_gdu, 0, 0))
            m_gdu = 0;
    }
    odr_destroy(encode);
}

int GDU::get_size()
{
    int len = 0;
    ODR encode = odr_createmem(ODR_ENCODE);
    if (m_gdu && z_GDU(encode, &m_gdu, 0, "encode"))
        odr_getbuf(encode, &len, 0);
    odr_destroy(encode);
    return len;
}

GDU &GDU::operator=(const GDU &g)
{
    if (this != &g)
    {
        odr_destroy(m_decode);

        base(g.get(), odr_createmem(ODR_ENCODE));
    }
    return *this;
}

GDU::~GDU()
{
    odr_destroy(m_decode);
}

Z_GDU *GDU::get() const
{
    return m_gdu;
}

void GDU::move_away_gdu(ODR dst, Z_GDU **gdu)
{
    *gdu = m_gdu;
    m_gdu = 0;
    NMEM nmem = odr_extract_mem(m_decode);
    if (!dst->mem)
        dst->mem = nmem_create();
    nmem_transfer(dst->mem, nmem);
    nmem_destroy(nmem);
}

/*
 * Local variables:
 * c-basic-offset: 4
 * c-file-style: "Stroustrup"
 * indent-tabs-mode: nil
 * End:
 * vim: shiftwidth=4 tabstop=8 expandtab
 */

