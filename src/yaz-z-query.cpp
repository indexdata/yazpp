/*
 * Copyright (c) 1998-2003, Index Data.
 * See the file LICENSE for details.
 * 
 * $Id: yaz-z-query.cpp,v 1.16 2005-06-02 06:40:21 adam Exp $
 */

#include <yaz++/z-query.h>
#include <yaz/pquery.h>

using namespace yazpp_1;

Yaz_Z_Query::Yaz_Z_Query()
{
    odr_encode = odr_createmem (ODR_ENCODE);
    odr_decode = odr_createmem (ODR_DECODE);
    odr_print = odr_createmem (ODR_PRINT);
}

int Yaz_Z_Query::set_rpn (const char *rpn)
{
    m_buf = 0;
    odr_reset (odr_encode);
    Z_Query *query = (Z_Query*) odr_malloc (odr_encode, sizeof(*query));
    query->which = Z_Query_type_1;
    query->u.type_1 = p_query_rpn (odr_encode, PROTO_Z3950, rpn);
    if (!query->u.type_1)
	return -1;
    if (!z_Query (odr_encode, &query, 0, 0))
	return -1;
    // z_Query(odr_print, &query, 0, 0);
    m_buf = odr_getbuf (odr_encode, &m_len, 0);
    return m_len;
}

void Yaz_Z_Query::set_Z_Query(Z_Query *z_query)
{
    m_buf = 0;
    odr_reset (odr_encode);
    if (!z_Query (odr_encode, &z_query, 0, 0))
	return;
    m_buf = odr_getbuf (odr_encode, &m_len, 0);
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
    odr_setbuf (odr_decode, m_buf, m_len, 0);
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

void Yaz_Z_Query::oid2str(Odr_oid *o, WRBUF buf)
{
    for (; *o >= 0; o++) {
	char ibuf[16];
	sprintf(ibuf, "%d", *o);
	wrbuf_puts(buf, ibuf);
	if (o[1] > 0)
	    wrbuf_putc(buf, '.');
    }
}

void Yaz_Z_Query::pr_term(WRBUF wbuf, char *buf, int len)
{
    int i;
    wrbuf_putc(wbuf, '"');
    for (i = 0; i<len; i++)
    {
	int ch = buf[i];
	if (ch == '"')
	    wrbuf_putc(wbuf, '\\');
	wrbuf_putc(wbuf, ch);
    }
    wrbuf_puts(wbuf, "\" ");
}

int Yaz_Z_Query::rpn2pquery(Z_RPNStructure *s, WRBUF buf)
{
    if (s->which == Z_RPNStructure_simple)
    {
	Z_Operand *o = s->u.simple;
	
	if (o->which == Z_Operand_APT) 
	{
	    Z_AttributesPlusTerm *at = s->u.simple->u.attributesPlusTerm;
	    if (at->attributes) {
		int i;
		for (i = 0; i < at->attributes->num_attributes; i++) {
		    wrbuf_puts(buf, "@attr ");
		    if (at->attributes->attributes[i]->attributeSet) {
			oid2str(at->attributes->attributes[i]->attributeSet, buf);
			wrbuf_putc(buf, ' ');
		    }
		    wrbuf_printf(buf, "%d=", *at->attributes->attributes[i]->attributeType);
		    wrbuf_printf(buf, "%d ", *at->attributes->attributes[i]->value.numeric);
		}
	    }
	    if (at->term->which == Z_Term_general)
	    {
		pr_term(buf, (char*) at->term->u.general->buf,
			at->term->u.general->len);
	    }
	    else if (at->term->which == Z_Term_characterString)
	    {
		wrbuf_puts(buf, "@term string ");
		pr_term(buf, at->term->u.characterString,
			strlen(at->term->u.characterString));

	    }
	}
	else if (o->which == Z_Operand_resultSetId)
	{
	    wrbuf_printf(buf, "@set %s ", o->u.resultSetId);
	}
    }
    else if (s->which == Z_RPNStructure_complex)
    {
	Z_Complex *c = s->u.complex;
	
	switch (c->roperator->which) {
	case Z_Operator_and: wrbuf_puts(buf, "@and "); break;
	case Z_Operator_or: wrbuf_puts(buf, "@or "); break;
	case Z_Operator_and_not: wrbuf_puts(buf, "@not "); break;
	case Z_Operator_prox: wrbuf_puts(buf, "@prox "); break;
	default: wrbuf_puts(buf, "@unknown ");
	}
	if (!rpn2pquery(c->s1, buf))
	    return 0;
	if (!rpn2pquery(c->s2, buf))
	    return 0;
    }
    return 1;
}

WRBUF Yaz_Z_Query::zquery2pquery(Z_Query *q)
{
    if (q->which != Z_Query_type_1 && q->which != Z_Query_type_101) 
	return 0;
    WRBUF buf = wrbuf_alloc();
    if (q->u.type_1->attributeSetId) {
	/* Output attribute set ID */
	wrbuf_puts(buf, "@attrset ");
	oid2str(q->u.type_1->attributeSetId, buf);
	wrbuf_putc(buf, ' ');
    }
    return rpn2pquery(q->u.type_1->RPNStructure, buf) ? buf : 0;
}


