/*
 * Copyright (c) 1998-1999, Index Data.
 * See the file LICENSE for details.
 * Sebastian Hammer, Adam Dickmeiss
 * 
 * $Log: yaz-z-query.cpp,v $
 * Revision 1.6  1999-12-06 13:52:45  adam
 * Modified for new location of YAZ header files. Experimental threaded
 * operation.
 *
 * Revision 1.5  1999/04/27 07:52:13  adam
 * Improved proxy; added query match for result set re-use.
 *
 * Revision 1.4  1999/04/21 12:09:01  adam
 * Many improvements. Modified to proxy server to work with "sessions"
 * based on cookies.
 *
 * Revision 1.3  1999/04/20 10:30:05  adam
 * Implemented various stuff for client and proxy. Updated calls
 * to ODR to reflect new name parameter.
 *
 * Revision 1.2  1999/04/09 11:46:57  adam
 * Added object Yaz_Z_Assoc. Much more functional client.
 *
 * Revision 1.1  1999/03/23 14:17:57  adam
 * More work on timeout handling. Work on yaz-client.
 *
 */

#include <yaz-z-query.h>
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
    z_Query(odr_print, &query, 0, 0);
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
