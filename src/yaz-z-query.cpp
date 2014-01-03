/* This file is part of the yazpp toolkit.
 * Copyright (C) Index Data 
 * See the file LICENSE for details.
 */

#if HAVE_CONFIG_H
#include <config.h>
#endif
#include <yaz/querytowrbuf.h>
#include <yazpp/z-query.h>
#include <yaz/pquery.h>
#include <assert.h>

using namespace yazpp_1;

class Yaz_Z_Query::Rep {
    friend class Yaz_Z_Query;
    char *buf;
    int len;
    ODR odr_decode;
    ODR odr_encode;
    ODR odr_print;
};


Yaz_Z_Query::Yaz_Z_Query()
{
    m_p = new Rep;
    m_p->odr_encode = odr_createmem(ODR_ENCODE);
    m_p->odr_decode = odr_createmem(ODR_DECODE);
    m_p->odr_print = odr_createmem(ODR_PRINT);
    m_p->len = 0;
    m_p->buf = 0;
}


Yaz_Z_Query::Yaz_Z_Query(const Yaz_Z_Query &q)
{
    m_p = new Rep;

    m_p->odr_encode = odr_createmem(ODR_ENCODE);
    m_p->odr_decode = odr_createmem(ODR_DECODE);
    m_p->odr_print = odr_createmem(ODR_PRINT);

    m_p->len = q.m_p->len;
    m_p->buf = (char*) odr_malloc(m_p->odr_encode, m_p->len);
    memcpy(m_p->buf, q.m_p->buf, m_p->len);
}

Yaz_Z_Query& Yaz_Z_Query::operator=(const Yaz_Z_Query &q)
{
    if (this != &q)
    {
        odr_reset(m_p->odr_encode);
        if (!q.m_p->buf)
        {
            m_p->buf = 0;
            m_p->len = 0;
        }
        else
        {
            m_p->len = q.m_p->len;
            m_p->buf = (char*) odr_malloc(m_p->odr_encode, m_p->len);
            memcpy(m_p->buf, q.m_p->buf, m_p->len);
        }
    }
    return *this;
}

Yaz_Z_Query& Yaz_Z_Query::operator=(const char *rpn)
{
    set_rpn(rpn);
    return *this;
}

int Yaz_Z_Query::set_rpn(const char *rpn)
{
    m_p->buf = 0;
    odr_reset(m_p->odr_encode);
    Z_Query *query = (Z_Query*) odr_malloc(m_p->odr_encode, sizeof(*query));
    query->which = Z_Query_type_1;
    query->u.type_1 = p_query_rpn(m_p->odr_encode, rpn);
    if (!query->u.type_1)
        return -1;
    if (!z_Query(m_p->odr_encode, &query, 0, 0))
        return -1;
    m_p->buf = odr_getbuf(m_p->odr_encode, &m_p->len, 0);
    return m_p->len;
}

void Yaz_Z_Query::set_Z_Query(Z_Query *z_query)
{
    m_p->buf = 0;
    odr_reset(m_p->odr_encode);
    if (!z_Query(m_p->odr_encode, &z_query, 0, 0))
        return;
    m_p->buf = odr_getbuf(m_p->odr_encode, &m_p->len, 0);
}

Yaz_Z_Query::~Yaz_Z_Query()
{
    odr_destroy(m_p->odr_encode);
    odr_destroy(m_p->odr_decode);
    odr_destroy(m_p->odr_print);
    delete m_p;
}

Z_Query *Yaz_Z_Query::get_Z_Query()
{
    Z_Query *query;
    if (!m_p->buf)
        return 0;
    odr_reset(m_p->odr_decode);
    odr_setbuf(m_p->odr_decode, m_p->buf, m_p->len, 0);
    if (!z_Query(m_p->odr_decode, &query, 0, 0))
        return 0;
    return query;
}

void Yaz_Z_Query::print(char *str, size_t len)
{
    Z_Query *query;
    *str = 0;
    if (!m_p->buf)
        return;
    odr_setbuf(m_p->odr_decode, m_p->buf, m_p->len, 0);
    if (!z_Query(m_p->odr_decode, &query, 0, 0))
        return;
    WRBUF wbuf = wrbuf_alloc();
    yaz_query_to_wrbuf(wbuf, query);
    if (wrbuf_len(wbuf) > len-1)
    {
        memcpy(str, wrbuf_buf(wbuf), len-1);
        str[len-1] = '\0';
    }
    else
        strcpy(str, wrbuf_cstr(wbuf));
    wrbuf_destroy(wbuf);
    odr_reset(m_p->odr_decode);
}

int Yaz_Z_Query::match(const Yaz_Z_Query *other)
{
    if (m_p->len != other->m_p->len)
        return 0;
    if (!m_p->buf || !other->m_p->buf)
        return 0;
    if (memcmp(m_p->buf, other->m_p->buf, m_p->len))
        return 0;
    return 1;
}

/*
 * Local variables:
 * c-basic-offset: 4
 * c-file-style: "Stroustrup"
 * indent-tabs-mode: nil
 * End:
 * vim: shiftwidth=4 tabstop=8 expandtab
 */

