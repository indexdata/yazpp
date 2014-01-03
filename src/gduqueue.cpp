/* This file is part of the yazpp toolkit.
 * Copyright (C) Index Data 
 * See the file LICENSE for details.
 */

#if HAVE_CONFIG_H
#include <config.h>
#endif
#include <yazpp/gdu.h>
#include <yazpp/gduqueue.h>

using namespace yazpp_1;

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
 * c-file-style: "Stroustrup"
 * indent-tabs-mode: nil
 * End:
 * vim: shiftwidth=4 tabstop=8 expandtab
 */

