/*
 * Copyright (c) 2000-2003, Index Data.
 * See the file LICENSE for details.
 * 
 * $Id: yaz-bw.cpp,v 1.1 2003-10-01 13:13:51 adam Exp $
 */

#include <time.h>
#include <yaz/log.h>
#include <yaz++/proxy.h>


Yaz_bw::Yaz_bw(int sz)
{
    m_sec = 0;
    m_size = sz;
    m_bucket = new int[m_size];
    m_ptr = 0;
}

Yaz_bw::~Yaz_bw()
{
    delete [] m_bucket;
}

int Yaz_bw::get_total()
{
    add_bytes(0);
    int bw = 0;
    int i;
    for (i = 0; i<m_size; i++)
	bw += m_bucket[i];
    return bw;
}

void Yaz_bw::add_bytes(int b)
{
    long now = time(0);

    int d = now - m_sec;
    if (d > m_size)
	d = m_size;
    while (--d >= 0)
    {
	if (++m_ptr == m_size)
	    m_ptr = 0;
	m_bucket[m_ptr] = 0;
    }
    m_bucket[m_ptr] += b;
    m_sec = now;
}

