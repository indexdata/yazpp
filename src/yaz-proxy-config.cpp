/*
 * Copyright (c) 1998-2003, Index Data.
 * See the file LICENSE for details.
 * 
 * $Id: yaz-proxy-config.cpp,v 1.4 2003-10-06 08:08:49 adam Exp $
 */

#include <ctype.h>
#include <yaz/log.h>
#include <yaz++/proxy.h>

Yaz_ProxyConfig::Yaz_ProxyConfig()
{
    m_copy = 0;
#if HAVE_XML2
    m_docPtr = 0;
    m_proxyPtr = 0;
#endif
}

Yaz_ProxyConfig::~Yaz_ProxyConfig()
{
#if HAVE_XML2
    if (!m_copy && m_docPtr)
	xmlFreeDoc(m_docPtr);
#endif
}

void Yaz_ProxyConfig::operator=(const Yaz_ProxyConfig &conf)
{
#if HAVE_XML2
    m_docPtr = conf.m_docPtr;
    m_proxyPtr = conf.m_proxyPtr;
#endif
    m_copy = 1;
}

int Yaz_ProxyConfig::read_xml(const char *fname)
{
#if HAVE_XML2
    xmlDocPtr ndoc = xmlParseFile(fname);

    if (!ndoc)
    {
	yaz_log(LOG_WARN, "Config file %s not found or parse error", fname);
	return -1;  // no good
    }
    xmlNodePtr proxyPtr = xmlDocGetRootElement(ndoc);
    if (!proxyPtr || proxyPtr->type != XML_ELEMENT_NODE ||
	strcmp((const char *) proxyPtr->name, "proxy"))
    {
	yaz_log(LOG_WARN, "No proxy element in %s", fname);
	xmlFreeDoc(ndoc);
	return -1;
    }
    m_proxyPtr = proxyPtr;

    // OK: release previous and make it the current one.
    if (m_docPtr)
	xmlFreeDoc(m_docPtr);
    m_docPtr = ndoc;
    return 0;
#else
    return -2;
#endif
}

#if HAVE_XML2
const char *Yaz_ProxyConfig::get_text(xmlNodePtr ptr)
{
    for(ptr = ptr->children; ptr; ptr = ptr->next)
	if (ptr->type == XML_TEXT_NODE)
	{
	    xmlChar *t = ptr->content;
	    if (t)
	    {
		while (*t == ' ')
		    t++;
		return (const char *) t;
	    }
	}
    return 0;
}
#endif

#if HAVE_XML2
void Yaz_ProxyConfig::return_limit(xmlNodePtr ptr,
				   int *limit_bw,
				   int *limit_pdu,
				   int *limit_req)
{
    for (ptr = ptr->children; ptr; ptr = ptr->next)
    {
	if (ptr->type == XML_ELEMENT_NODE 
	    && !strcmp((const char *) ptr->name, "bandwidth"))
	{
	    const char *t = get_text(ptr);
	    if (t)
		*limit_bw = atoi(t);
	}
	if (ptr->type == XML_ELEMENT_NODE 
	    && !strcmp((const char *) ptr->name, "retrieve"))
	{
	    const char *t = get_text(ptr);
	    if (t)
		*limit_req = atoi(t);
	}
	if (ptr->type == XML_ELEMENT_NODE 
	    && !strcmp((const char *) ptr->name, "pdu"))
	{
	    const char *t = get_text(ptr);
	    if (t)
		*limit_pdu = atoi(t);
	}
    }
}
#endif

#if HAVE_XML2
void Yaz_ProxyConfig::return_target_info(xmlNodePtr ptr,
					 const char **url,
					 int *keepalive,
					 int *limit_bw,
					 int *limit_pdu,
					 int *limit_req,
					 int *target_idletime,
					 int *client_idletime)
{
    ptr = ptr->children;
    for (; ptr; ptr = ptr->next)
    {
	if (ptr->type == XML_ELEMENT_NODE 
	    && !strcmp((const char *) ptr->name, "url"))
	{
	    const char *t = get_text(ptr);
	    if (t)
		*url = t;
	}
	if (ptr->type == XML_ELEMENT_NODE 
	    && !strcmp((const char *) ptr->name, "keepalive"))
	{
	    const char *t = get_text(ptr);
	    if (!t || *t == '1')
		*keepalive = 1;
	    else
		*keepalive = 0;
	}
	if (ptr->type == XML_ELEMENT_NODE 
	    && !strcmp((const char *) ptr->name, "limit"))
	    return_limit(ptr, limit_bw, limit_pdu, limit_req);
	if (ptr->type == XML_ELEMENT_NODE 
	    && !strcmp((const char *) ptr->name, "target-timeout"))
	{
	    const char *t = get_text(ptr);
	    if (t)
	    {
		*target_idletime = atoi(t);
		if (*target_idletime < 0)
		    *target_idletime = 0;
	    }
	}
	if (ptr->type == XML_ELEMENT_NODE 
	    && !strcmp((const char *) ptr->name, "client-timeout"))
	{
	    const char *t = get_text(ptr);
	    if (t)
	    {
		*client_idletime = atoi(t);
		if (*client_idletime < 0)
		    *client_idletime = 0;
	    }
	}
    }
}
#endif

int Yaz_ProxyConfig::atoi_l(const char **cp)
{
    int v = 0;
    while (**cp && isdigit(**cp))
    {
	v = v*10 + (**cp - '0');
	(*cp)++;
    }
    return v;
}

int Yaz_ProxyConfig::match_list(int v, const char *m)
{
  while(m && *m)
  {
      while(*m && isspace(*m))
	  m++;
      if (*m == '*')
	  return 1;
      int l = atoi_l(&m);
      int h = l;
      if (*m == '-')
      {
	  ++m;
	  h = atoi_l(&m);
      }
      if (v >= l && v <= h)
	  return 1;
      if (*m == ',')
	  m++;
  }
  return 0;
}

#if HAVE_XML2
int Yaz_ProxyConfig::check_type_1_attributes(ODR odr, xmlNodePtr ptr,
					     Z_AttributeList *attrs,
					     char **addinfo)
{
    for(ptr = ptr->children; ptr; ptr = ptr->next)
    {
	if (ptr->type == XML_ELEMENT_NODE &&
	    !strcmp((const char *) ptr->name, "query"))
	{
	    const char *match_type = 0;
	    const char *match_value = 0;
	    const char *match_error = 0;
	    struct _xmlAttr *attr;
	    for (attr = ptr->properties; attr; attr = attr->next)
	    {
		if (!strcmp((const char *) attr->name, "type") &&
		    attr->children && attr->children->type == XML_TEXT_NODE)
		    match_type = (const char *) attr->children->content;
		if (!strcmp((const char *) attr->name, "value") &&
		    attr->children && attr->children->type == XML_TEXT_NODE)
		    match_value = (const char *) attr->children->content;
		if (!strcmp((const char *) attr->name, "error") &&
		    attr->children && attr->children->type == XML_TEXT_NODE)
		    match_error = (const char *) attr->children->content;
	    }
	    int i;

	    if (match_type && match_value)
	    {
		for (i = 0; i<attrs->num_attributes; i++)
		{
		    Z_AttributeElement *el = attrs->attributes[i];
		    char value_str[20];
		    
		    value_str[0] = '\0';
		    if (!el->attributeType)
			continue;
		    int type = *el->attributeType;

		    if (!match_list(type, match_type))
			continue;
		    if (el->which == Z_AttributeValue_numeric && 
			el->value.numeric)
		    {
			if (!match_list(*el->value.numeric, match_value))
			    continue;
			sprintf (value_str, "%d", *el->value.numeric);
		    }
		    else
			continue;
		    if (match_error)
		    {
			if (*value_str)
			    *addinfo = odr_strdup(odr, value_str);
			return atoi(match_error);
		    }
		    return 0;
		}
	    }
	}
    }
    return 0;
}
#endif

#if HAVE_XML2
int Yaz_ProxyConfig::check_type_1_structure(ODR odr, xmlNodePtr ptr,
					    Z_RPNStructure *q,
					    char **addinfo)
{
    int c;
    if (q->which == Z_RPNStructure_complex)
    {
	int e = check_type_1_structure(odr, ptr, q->u.complex->s1, addinfo);
	if (e)
	    return e;
	e = check_type_1_structure(odr, ptr, q->u.complex->s2, addinfo);
	return e;
    }
    else if (q->which == Z_RPNStructure_simple)
    {
	if (q->u.simple->which == Z_Operand_APT)
	{
	    return check_type_1_attributes(
		odr, ptr, q->u.simple->u.attributesPlusTerm->attributes,
		addinfo);
	}
    }
    return 0;
}
#endif

#if HAVE_XML2
int Yaz_ProxyConfig::check_type_1(ODR odr, xmlNodePtr ptr, Z_RPNQuery *query,
				  char **addinfo)
{
    // possibly check for Bib-1
    return check_type_1_structure(odr, ptr, query->RPNStructure, addinfo);
}
#endif

int Yaz_ProxyConfig::check_query(ODR odr, const char *name, Z_Query *query,
				 char **addinfo)
{
#if HAVE_XML2
    xmlNodePtr ptr;
    
    ptr = find_target_node(name);
    if (ptr)
    {
	if (query->which == Z_Query_type_1 || query->which == Z_Query_type_101)
	    return check_type_1(odr, ptr, query->u.type_1, addinfo);
    }
#endif
    return 0;
}

#if HAVE_XML2
xmlNodePtr Yaz_ProxyConfig::find_target_node(const char *name)
{
    xmlNodePtr ptr;
    for (ptr = m_proxyPtr->children; ptr; ptr = ptr->next)
    {
	if (ptr->type == XML_ELEMENT_NODE &&
	    !strcmp((const char *) ptr->name, "target"))
	{
	    // default one ? 
	    if (!name)
	    {
		// <target default="1"> ?
		struct _xmlAttr *attr;
		for (attr = ptr->properties; attr; attr = attr->next)
		    if (!strcmp((const char *) attr->name, "default") &&
			attr->children && attr->children->type == XML_TEXT_NODE)
		    {
			xmlChar *t = attr->children->content;
			if (!t || *t == '1')
			    return ptr;
		    }
	    }
	    else
	    {
		// <target name="name"> ?
		struct _xmlAttr *attr;
		for (attr = ptr->properties; attr; attr = attr->next)
		    if (!strcmp((const char *) attr->name, "name"))
		    {
			if (attr->children
			    && attr->children->type==XML_TEXT_NODE
			    && attr->children->content 
			    && (!strcmp((const char *) attr->children->content,
					name)
				|| !strcmp((const char *) attr->children->content,
					   "*")))
			{
			    return ptr;
			}
		    }
	    }
	}
    }
    return 0;
}
#endif

void Yaz_ProxyConfig::get_target_info(const char *name,
				      const char **url,
				      int *keepalive,
				      int *limit_bw,
				      int *limit_pdu,
				      int *limit_req,
				      int *target_idletime,
				      int *client_idletime,
				      int *max_clients)
{
#if HAVE_XML2
    xmlNodePtr ptr;
    if (!m_proxyPtr)
    {
	*url = name;
	return;
    }
    for (ptr = m_proxyPtr->children; ptr; ptr = ptr->next)
    {
	if (ptr->type == XML_ELEMENT_NODE &&
	    !strcmp((const char *) ptr->name, "max-clients"))
	{
	    const char *t = get_text(ptr);
	    if (t)
	    {
		*max_clients = atoi(t);
		if (*max_clients  < 1)
		    *max_clients = 1;
	    }
	}
    }
    ptr = find_target_node(name);
    if (ptr)
    {
	if (name)
	    *url = name;
	return_target_info(ptr, url, keepalive, limit_bw, limit_pdu, limit_req,
			   target_idletime, client_idletime);
    }
#else
    *url = name;
    return;
#endif
}


