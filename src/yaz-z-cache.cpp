/*
 * Copyright (c) 1998-2003, Index Data.
 * See the file LICENSE for details.
 * 
 * $Id: yaz-z-cache.cpp,v 1.1 2003-07-18 13:27:20 adam Exp $
 */

#include <yaz/log.h>
#include <yaz++/proxy.h>

struct Yaz_RecordCache_Entry {
    int m_offset;
    Z_NamePlusRecord *m_record;
    Yaz_RecordCache_Entry *m_next;
};


Yaz_RecordCache::Yaz_RecordCache ()
{
    m_mem = nmem_create();
    m_entries = 0;
}

Yaz_RecordCache::~Yaz_RecordCache ()
{
    nmem_destroy(m_mem);
}

void Yaz_RecordCache::clear ()
{
    nmem_destroy(m_mem);
    m_mem = nmem_create();
    m_entries = 0;
}

#if 0
void Yaz_RecordCache::prepare_present(Z_RecordComposition *comp)
{
    if (!comp)
	m_recordComposition = 0;
    else
    {
	m_recordComposition = nmem_malloc(m_mem, sizeof(*m_recordComposition));
	m_recordComposition->which = comp->which;
	if (comp->which == Z_RecordComp_simple)
	{
	    m_recordComposition->u.simple = (Z_ElementSetNames *)
		nmem_malloc(m_mem, sizeof(Z_ElementSetNames));
	}
    }

}
#endif

void Yaz_RecordCache::add (ODR o, Z_NamePlusRecordList *npr, int start)
{
    NMEM tmp_mem = odr_extract_mem(o);
    nmem_transfer(m_mem, tmp_mem);
    nmem_destroy(tmp_mem);

    int i;
    for (i = 0; i<npr->num_records; i++)
    {
	Yaz_RecordCache_Entry *entry = (Yaz_RecordCache_Entry *)
	    nmem_malloc(m_mem, sizeof(*entry));
	entry->m_record = npr->records[i];
	entry->m_offset = i + start;
	entry->m_next = m_entries;
	m_entries = entry;
    }
}

int Yaz_RecordCache::lookup (ODR o, Z_NamePlusRecordList **npr,
			     int start, int num,
			     Odr_oid *syntax)
{
    int i;
    yaz_log(LOG_LOG, "cache lookup start=%d num=%d", start, num);

    for (i = 0; i<num; i++)
    {
	Yaz_RecordCache_Entry *entry = m_entries;
	for(; entry; entry = entry->m_next)
	    if (entry->m_offset == start + i &&
		entry->m_record->which == Z_NamePlusRecord_databaseRecord &&
		!oid_oidcmp(entry->m_record->u.databaseRecord->direct_reference,
			    syntax))
		break;
	if (!entry)
	    return 0;
    }
    *npr = (Z_NamePlusRecordList *) odr_malloc(o, sizeof(**npr));
    (*npr)->num_records = num;
    (*npr)->records = (Z_NamePlusRecord **)
	odr_malloc(o, num * sizeof(Z_NamePlusRecord *));
    for (i = 0; i<num; i++)
    {
	Yaz_RecordCache_Entry *entry = m_entries;
	for(; entry; entry = entry->m_next)
	    if (entry->m_offset == start + i &&
		entry->m_record->which == Z_NamePlusRecord_databaseRecord &&
		!oid_oidcmp(entry->m_record->u.databaseRecord->direct_reference,
			    syntax))
		break;
	if (!entry)
	    return 0;
	(*npr)->records[i] = entry->m_record;
    }
    return 1;
}
