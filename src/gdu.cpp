/*
 * Copyright (c) 1998-2005, Index Data.
 * See the file LICENSE for details.
 * 
 * $Id: gdu.cpp,v 1.2 2005-06-25 15:53:19 adam Exp $
 */

#include <yaz++/gdu.h>

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

void GDU::base(Z_GDU *gdu, ODR encode)
{
    m_decode = odr_createmem(ODR_DECODE);
    m_gdu = 0;
    if (z_GDU(encode, &gdu, 0, "encode"))
    {
        int len;
        char *buf = odr_getbuf(encode, &len, 0);
        
        odr_setbuf(m_decode, buf, len, 0);
        z_GDU(m_decode, &m_gdu, 0, 0);
    }
    odr_destroy(encode);
}

GDU::~GDU()
{
    odr_destroy(m_decode);
}

Z_GDU *GDU::get()
{
    return m_gdu;
}

void GDU::extract_odr_to(ODR dst)
{
    NMEM nmem = odr_extract_mem(m_decode);
    if (!dst->mem)
        dst->mem = nmem_create();
    nmem_transfer(dst->mem, nmem);
    nmem_destroy(nmem);
}


GDUQueue::GDUQueue()
{
    m_list = 0;
}

int GDUQueue::size()
{
    int no = 0;
    GDUQueue_List *l;
    for (l = m_list; l; l = l->m_next)
        no++;
    return no;
}

void GDUQueue::enqueue(GDU *gdu)
{
    GDUQueue_List *l = new GDUQueue_List;
    l->m_next = m_list;
    l->m_item = gdu;
    m_list = l;
}

GDU *GDUQueue::dequeue()
{
    GDUQueue_List **l = &m_list;
    if (!*l)
        return 0;
    while ((*l)->m_next)
        l = &(*l)->m_next;
    GDU *m = (*l)->m_item;
    delete *l;
    *l = 0;
    return m;
}

void GDUQueue::clear()
{
    GDU *g;
    while ((g = dequeue()))
        delete g;
}

GDUQueue::~GDUQueue()
{
    clear();
}
/*
 * Local variables:
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * End:
 * vim: shiftwidth=4 tabstop=8 expandtab
 */

