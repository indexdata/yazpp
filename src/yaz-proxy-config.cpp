/*
 * Copyright (c) 1998-2003, Index Data.
 * See the file LICENSE for details.
 * 
 * $Id: yaz-proxy-config.cpp,v 1.1 2003-10-01 13:13:51 adam Exp $
 */

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
					 int *limit_req)
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
    }
}
#endif

void Yaz_ProxyConfig::get_target_info(const char *name,
				      const char **url,
				      int *keepalive,
				      int *limit_bw,
				      int *limit_pdu,
				      int *limit_req)
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
			{
			    return_target_info(ptr, url, keepalive,
					       limit_bw, limit_pdu, limit_req);
			    return;
			}
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
			    *url = name;
			    return_target_info(ptr, url, keepalive,
					       limit_bw, limit_pdu, limit_req);
			    return;
			}
		    }
	    }
	}
    }
#else
    *url = name;
    return;
#endif
}


