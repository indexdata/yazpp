/*
 * Copyright (c) 1998-2001, Index Data.
 * See the file LICENSE for details.
 * 
 * $Id: yaz-z-query.cpp,v 1.11 2002-10-09 12:50:26 adam Exp $
 */

#include <yaz++/z-query.h>
#include <yaz/pquery.h>

Yaz_Z_Query::Yaz_Z_Query()
{
    odr_encode = odr_createmem (ODR_ENCODE);
    odr_decode = odr_createmem (ODR_DECODE);
    odr_print = odr_createmem (ODR_PRINT);
}

int Yaz_Z_Query::set_rpn (const char *rpn)
{
    buf = 0;
    odr_reset (odr_encode);
    Z_Query *query = (Z_Query*) odr_malloc (odr_encode, sizeof(*query));
    query->which = Z_Query_type_1;
    query->u.type_1 = p_query_rpn (odr_encode, PROTO_Z3950, rpn);
    if (!query->u.type_1)
	return -1;
    if (!z_Query (odr_encode, &query, 0, 0))
	return -1;
    // z_Query(odr_print, &query, 0, 0);
    buf = odr_getbuf (odr_encode, &len, 0);
    return len;
}

void Yaz_Z_Query::set_Z_Query(Z_Query *z_query)
{
    buf = 0;
    odr_reset (odr_encode);
    if (!z_Query (odr_encode, &z_query, 0, 0))
	return;
    buf = odr_getbuf (odr_encode, &len, 0);
}

Yaz_Z_Query::~Yaz_Z_Query()
{
    odr_destroy (odr_encode);
    odr_destroy (odr_decode);
    odr_destroy (odr_print);
}

Z_Query *Yaz_Z_Query::get_Z_Query ()
{
    Z_Query *query;
    if (!buf)
	return 0;
    odr_setbuf (odr_decode, buf, len, 0);
    if (!z_Query(odr_decode, &query, 0, 0))
	return 0;
    return query;
}

void Yaz_Z_Query::print(char *str, int len)
{

}

int Yaz_Z_Query::match(Yaz_Z_Query *other)
{
    if (len != other->len)
	return 0;
    if (!buf || !other->buf)
	return 0;
    if (memcmp(buf, other->buf, len))
	return 0;
    return 1;
}
