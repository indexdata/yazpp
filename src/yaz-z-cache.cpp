/* This file is part of the yazpp toolkit.
 * Copyright (C) 1998-2009 Index Data and Mike Taylor
 * See the file LICENSE for details.
 */

#include <yaz/log.h>
#include <yaz/proto.h>
#include <yazpp/record-cache.h>

using namespace yazpp_1;

struct yazpp_1::RecordCache_Entry {
    int m_offset;
    Z_NamePlusRecord *m_record;
    Z_RecordComposition *m_comp;
    RecordCache_Entry *m_next;
};

RecordCache::RecordCache ()
{
    m_mem = nmem_create();
    m_entries = 0;
    m_presentRequest = 0;
    m_searchRequest = 0;
    m_max_size = 200000;
}

RecordCache::~RecordCache ()
{
    nmem_destroy(m_mem);
}

void RecordCache::set_max_size(int sz)
{
    m_max_size = sz;
}

void RecordCache::clear ()
{
    nmem_destroy(m_mem);
    m_mem = nmem_create();
    m_entries = 0;
    m_presentRequest = 0;
    m_searchRequest = 0;
}

void RecordCache::copy_searchRequest(Z_SearchRequest *sr)
{
    ODR encode = odr_createmem(ODR_ENCODE);
    ODR decode = odr_createmem(ODR_DECODE);

    m_searchRequest = 0;
    m_presentRequest = 0;
    int v = z_SearchRequest (encode, &sr, 1, 0);
    if (v)
    {
        int len;
        char *buf = odr_getbuf(encode, &len, 0);
        odr_setbuf(decode, buf, len, 0);
        z_SearchRequest(decode, &m_searchRequest, 1, 0);
        nmem_transfer(m_mem, decode->mem);
    }
    odr_destroy(encode);
    odr_destroy(decode);
}

void RecordCache::copy_presentRequest(Z_PresentRequest *pr)
{
    ODR encode = odr_createmem(ODR_ENCODE);
    ODR decode = odr_createmem(ODR_DECODE);
    
    m_searchRequest = 0;
    m_presentRequest = 0;
    int v = z_PresentRequest (encode, &pr, 1, 0);
    if (v)
    {
        int len;
        char *buf = odr_getbuf(encode, &len, 0);
        odr_setbuf(decode, buf, len, 0);
        z_PresentRequest(decode, &m_presentRequest, 1, 0);
        nmem_transfer(m_mem, decode->mem);
    }
    odr_destroy(encode);
    odr_destroy(decode);
}

void RecordCache::add (ODR o, Z_NamePlusRecordList *npr, int start,
                           int hits)
{
    if (nmem_total(m_mem) > m_max_size)
        return;
    // Build appropriate compspec for this response
    Z_RecordComposition *comp = 0;
    if (hits == -1 && m_presentRequest)
        comp = m_presentRequest->recordComposition;
    else if (hits > 0 && m_searchRequest)
    {
        Z_ElementSetNames *esn;

        if (hits <= *m_searchRequest->smallSetUpperBound)
            esn = m_searchRequest->smallSetElementSetNames;
        else
            esn = m_searchRequest->mediumSetElementSetNames;
        comp = (Z_RecordComposition *) nmem_malloc(m_mem, sizeof(*comp));
        comp->which = Z_RecordComp_simple;
        comp->u.simple = esn;
    }

    // Z_NamePlusRecordList *npr to be owned by m_mem..
    NMEM tmp_mem = odr_extract_mem(o);
    nmem_transfer(m_mem, tmp_mem);
    nmem_destroy(tmp_mem);
    
    // Insert individual records in cache
    int i;
    for (i = 0; i<npr->num_records; i++)
    {
        RecordCache_Entry *entry = (RecordCache_Entry *)
            nmem_malloc(m_mem, sizeof(*entry));
        entry->m_record = (Z_NamePlusRecord *)
            nmem_malloc(m_mem, sizeof(*entry->m_record));
        entry->m_record->databaseName = npr->records[i]->databaseName;
        entry->m_record->which = npr->records[i]->which;
        entry->m_record->u.databaseRecord  = npr->records[i]->u.databaseRecord;
        entry->m_comp = comp;
        entry->m_offset = i + start;
        entry->m_next = m_entries;
        m_entries = entry;
    }
}

int RecordCache::match (RecordCache_Entry *entry,
                            Odr_oid *syntax, int offset,
                            Z_RecordComposition *comp)
{
    // See if our compspec match...
    int match = 0;
    ODR o1 = odr_createmem(ODR_ENCODE);
    ODR o2 = odr_createmem(ODR_ENCODE);
    
    z_RecordComposition(o1, &comp, 1, 0);
    z_RecordComposition(o2, &entry->m_comp, 1, 0);

    int len1 = -1;
    char *buf1 = odr_getbuf(o1, &len1, 0);
    int len2 = -1;
    char *buf2 = odr_getbuf(o2, &len2, 0);
    
    if (buf1 && buf2 && len1 && len1 == len2 && !memcmp(buf1, buf2, len1))
        match = 1;
    else if (!buf1 && !buf2 && !len1 && !len2)
        match = 1;
    
    odr_destroy(o1);
    odr_destroy(o2);
    if (!match)
        return 0;
    if (!syntax)
        return 0;
    // See if offset, OID match..
    if (entry->m_offset == offset &&
        entry->m_record->which == Z_NamePlusRecord_databaseRecord &&
        !oid_oidcmp(entry->m_record->u.databaseRecord->direct_reference,
                    syntax))
        return 1;
#if 0
    char mstr1[100];
    oid_to_dotstring(entry->m_record->u.databaseRecord->direct_reference, mstr1);
    char mstr2[100];
    oid_to_dotstring(syntax, mstr2);
    yaz_log(YLOG_LOG, "match fail 3 d=%s s=%s", mstr1, mstr2);
#endif

    return 0;
}

int RecordCache::lookup (ODR o, Z_NamePlusRecordList **npr,
                             int start, int num,
                             Odr_oid *syntax,
                             Z_RecordComposition *comp)
{
    int i;
    yaz_log(YLOG_DEBUG, "cache lookup start=%d num=%d", start, num);

    for (i = 0; i<num; i++)
    {
        RecordCache_Entry *entry = m_entries;
        for(; entry; entry = entry->m_next)
            if (match(entry, syntax, start+i, comp))
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
        RecordCache_Entry *entry = m_entries;
        for(; entry; entry = entry->m_next)
            if (match(entry, syntax, start+i, comp))
                break;
        if (!entry)
            return 0;
        (*npr)->records[i] = (Z_NamePlusRecord *)
            odr_malloc(o, sizeof(Z_NamePlusRecord));
        (*npr)->records[i]->databaseName = entry->m_record->databaseName;
        (*npr)->records[i]->which = entry->m_record->which;
        (*npr)->records[i]->u.databaseRecord =
            entry->m_record->u.databaseRecord;
    }
    return 1;
}
/*
 * Local variables:
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * End:
 * vim: shiftwidth=4 tabstop=8 expandtab
 */

