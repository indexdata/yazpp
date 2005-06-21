/*
 * Copyright (c) 1998-2005, Index Data.
 * See the file LICENSE for details.
 * 
 * $Id: gdu.h,v 1.1 2005-06-21 17:37:15 adam Exp $
 */

#ifndef YAZPP_GDU_INCLUDED
#define YAZPP_GDU_INCLUDED

#include <yaz/yconfig.h>
#include <yaz/proto.h>

namespace yazpp_1 {

    class YAZ_EXPORT GDU {
    public:
	GDU(Z_GDU *gdu);
	GDU(Z_APDU *apdu);
	~GDU();
	Z_GDU *get();
	void extract_odr_to(ODR dst);
    private:
	void base(Z_GDU *gdu, ODR o);
	Z_GDU *m_gdu;
	ODR m_decode;
    };

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
