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
#include <yazpp/limit-connect.h>

#include <time.h>
#include <string.h>
#include <yaz/xmalloc.h>
#include <yazpp/timestat.h>

using namespace yazpp_1;

struct LimitConnect::Rep {
    int m_period;
    Peer *m_peers;
    Peer **lookup(const char *peername);
};

struct LimitConnect::Peer {
    Peer(int sz, const char *peername);
    ~Peer();
    void add_connect();

    char *m_peername;
    TimeStat m_bw;
    Peer *m_next;
};

LimitConnect::LimitConnect()
{
    m_p = new Rep;
    m_p->m_period = 60;
    m_p->m_peers = 0;
}


LimitConnect::~LimitConnect()
{
    cleanup(true);
    delete m_p;
}

void LimitConnect::set_period(int sec)
{
    m_p->m_period = sec;
}

LimitConnect::Peer::Peer(int sz, const char *peername) : m_bw(sz)
{
    m_peername = xstrdup(peername);
    m_next = 0;
}

LimitConnect::Peer::~Peer()
{
    xfree(m_peername);
}

void LimitConnect::Peer::add_connect()
{
    m_bw.add_bytes(1);
}

LimitConnect::Peer **LimitConnect::Rep::lookup(const char *peername)
{
    Peer **p = &m_peers;
    while (*p)
    {
	if (!strcmp((*p)->m_peername, peername))
	    break;
	p = &(*p)->m_next;
    }
    return p;
}

void LimitConnect::add_connect(const char *peername)
{
    Peer **p = m_p->lookup(peername);
    if (!*p)
	*p = new Peer(m_p->m_period, peername);
    (*p)->add_connect();
}

int LimitConnect::get_total(const char *peername)
{
    Peer **p = m_p->lookup(peername);
    if (!*p)
	return 0;
    return (*p)->m_bw.get_total();
}

void LimitConnect::cleanup(bool all)
{
    Peer **p = &m_p->m_peers;
    while (*p)
    {
	Peer *tp = *p;
	if (all || (tp->m_bw.get_total() == 0))
	{
	    *p = tp->m_next;
	    delete tp;
	}
	else
	    p = &tp->m_next;
    }
}

/*
 * Local variables:
 * c-basic-offset: 4
 * c-file-style: "Stroustrup"
 * indent-tabs-mode: nil
 * End:
 * vim: shiftwidth=4 tabstop=8 expandtab
 */

