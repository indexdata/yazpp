/* This file is part of the yazpp toolkit.
 * Copyright (C) Index Data 
 * All rights reserved.
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of Index Data nor the names of its contributors
 *       may be used to endorse or promote products derived from this
 *       software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE REGENTS AND CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#if HAVE_CONFIG_H
#include <config.h>
#endif
#include <yazpp/timestat.h>
#include <time.h>

using namespace yazpp_1;

class TimeStat::Rep {
    friend class TimeStat;
    time_t sec;   // time of most recent bucket
    int *bucket;
    int ptr;
    int size;
};

TimeStat::TimeStat(int sz)
{
    m_p = new Rep;
    m_p->sec = 0;
    m_p->size = sz;
    m_p->bucket = new int[m_p->size];
    m_p->ptr = 0;
}

TimeStat::~TimeStat()
{
    delete [] m_p->bucket;
    delete m_p;
}

int TimeStat::get_total()
{
    add_bytes(0);
    int bw = 0;
    int i;
    for (i = 0; i < m_p->size; i++)
        bw += m_p->bucket[i];
    return bw;
}

void TimeStat::add_bytes(int b)
{
    time_t now = time(0);

    if (now >= m_p->sec)
    {
        int d = now - m_p->sec;
        if (d > m_p->size)
            d = m_p->size;
        while (--d >= 0)
        {
            if (++m_p->ptr == m_p->size)
                m_p->ptr = 0;
            m_p->bucket[m_p->ptr] = 0;
        }
        m_p->bucket[m_p->ptr] += b;
    }
    m_p->sec = now;
}

/*
 * Local variables:
 * c-basic-offset: 4
 * c-file-style: "Stroustrup"
 * indent-tabs-mode: nil
 * End:
 * vim: shiftwidth=4 tabstop=8 expandtab
 */

