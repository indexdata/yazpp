/*
 * Copyright (c) 1998-2005, Index Data.
 * See the file LICENSE for details.
 * 
 * $Id: yaz-z-query.cpp,v 1.18 2005-09-27 17:57:51 adam Exp $
 */

#include <yaz/logrpn.h>
#include <yaz++/z-query.h>
#include <yaz/pquery.h>

using namespace yazpp_1;

Yaz_Z_Query::Yaz_Z_Query()
{
    odr_encode = odr_createmem(ODR_ENCODE);
    odr_decode = odr_createmem(ODR_DECODE);
    odr_print = odr_createmem(ODR_PRINT);
}

Yaz_Z_Query& Yaz_Z_Query::operator=(const Yaz_Z_Query &p)
{
    if (this != &p)
    {
        odr_reset(odr_encode);
        if (!p.m_buf)
        {
            m_buf = 0;
            m_len = 0;
        }
        else
        {
            m_len = p.m_len;
            m_buf = (char*) odr_malloc(odr_encode, m_len);
            memcpy(m_buf, p.m_buf, m_len);
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
    m_buf = 0;
    odr_reset(odr_encode);
    Z_Query *query = (Z_Query*) odr_malloc(odr_encode, sizeof(*query));
    query->which = Z_Query_type_1;
    query->u.type_1 = p_query_rpn(odr_encode, PROTO_Z3950, rpn);
    if (!query->u.type_1)
        return -1;
    if (!z_Query(odr_encode, &query, 0, 0))
        return -1;
    // z_Query(odr_print, &query, 0, 0);
    m_buf = odr_getbuf(odr_encode, &m_len, 0);
    return m_len;
}

void Yaz_Z_Query::set_Z_Query(Z_Query *z_query)
{
    m_buf = 0;
    odr_reset(odr_encode);
    if (!z_Query(odr_encode, &z_query, 0, 0))
        return;
    m_buf = odr_getbuf(odr_encode, &m_len, 0);
}

Yaz_Z_Query::~Yaz_Z_Query()
{
    odr_destroy(odr_encode);
    odr_destroy(odr_decode);
    odr_destroy(odr_print);
}

Z_Query *Yaz_Z_Query::get_Z_Query()
{
    Z_Query *query;
    if (!m_buf)
        return 0;
    odr_reset(odr_decode);
    odr_setbuf(odr_decode, m_buf, m_len, 0);
    if (!z_Query(odr_decode, &query, 0, 0))
        return 0;
    return query;
}

void Yaz_Z_Query::print(char *str, int len)
{
    Z_Query *query;
    *str = 0;
    if (!m_buf)
        return;
    odr_setbuf(odr_decode, m_buf, m_len, 0);
    if (!z_Query(odr_decode, &query, 0, 0))
        return;
    WRBUF wbuf = zquery2pquery(query);
    if (wbuf)
    {
        if (wrbuf_len(wbuf) > len-1)
        {
            memcpy(str, wrbuf_buf(wbuf), len-1);
            str[len-1] = '\0';
        }
        else
            strcpy(str, wrbuf_buf(wbuf));
        wrbuf_free(wbuf,1);
    }
    odr_reset(odr_decode);
}

int Yaz_Z_Query::match(Yaz_Z_Query *other)
{
    if (m_len != other->m_len)
        return 0;
    if (!m_buf || !other->m_buf)
        return 0;
    if (memcmp(m_buf, other->m_buf, m_len))
        return 0;
    return 1;
}

WRBUF Yaz_Z_Query::zquery2pquery(Z_Query *q)
{
    WRBUF w = wrbuf_alloc();
    wrbuf_put_zquery(w, q);
    return w;
}

/*
 * Local variables:
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * End:
 * vim: shiftwidth=4 tabstop=8 expandtab
 */

