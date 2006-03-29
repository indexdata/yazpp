/*
 * Copyright (c) 1998-2005, Index Data.
 * See the file LICENSE for details.
 * 
 * $Id: gduqueue.h,v 1.1 2006-03-29 13:14:15 adam Exp $
 */

#ifndef YAZPP_GDUQUEUE_INCLUDED
#define YAZPP_GDUQUEUE_INCLUDED

#include <yaz/yconfig.h>

namespace yazpp_1 {
    class GDU;
    class GDUQueue_List {
        friend class GDUQueue;
    private:
        GDU *m_item;
        GDUQueue_List *m_next;
    };

    class GDUQueue {
    public:
        GDUQueue();
        ~GDUQueue();
        void clear();
        void enqueue(GDU *gdu);
        GDU *dequeue();
        int size();
    private:
        GDUQueue_List *m_list;
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

