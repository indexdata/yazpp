/* This file is part of the yazpp toolkit.
 * Copyright (C) Index Data 
 * See the file LICENSE for details.
 */

#if HAVE_CONFIG_H
#include <config.h>
#endif
#include <yaz/log.h>
#include <yaz/proto.h>
#include <yaz/copy_types.h>
#include <yazpp/record-cache.h>

using namespace yazpp_1;

struct RecordCache::Rep {
    NMEM nmem;
    RecordCache_Entry *entries;
    Z_SearchRequest *searchRequest;
    Z_PresentRequest *presentRequest;
    int match(RecordCache_Entry *entry, Odr_oid *syntax, int offset,
              Z_RecordComposition *comp);
    size_t max_size;
};

struct RecordCache::RecordCache_Entry {
    int m_offset;
    Z_NamePlusRecord *m_record;
    Z_RecordComposition *m_comp;
    RecordCache_Entry *m_next;
};

RecordCache::RecordCache ()
{
    m_p = new Rep;
    m_p->nmem = nmem_create();
    m_p->entries = 0;
    m_p->presentRequest = 0;
    m_p->searchRequest = 0;
    m_p->max_size = 200000;
}

RecordCache::~RecordCache ()
{
    nmem_destroy(m_p->nmem);
    delete m_p;
}

void RecordCache::set_max_size(size_t sz)
{
    m_p->max_size = sz;
}

void RecordCache::clear ()
{
    nmem_destroy(m_p->nmem);
    m_p->nmem = nmem_create();
    m_p->entries = 0;
    m_p->presentRequest = 0;
    m_p->searchRequest = 0;
}

void RecordCache::copy_searchRequest(Z_SearchRequest *sr)
{
    ODR encode = odr_createmem(ODR_ENCODE);
    ODR decode = odr_createmem(ODR_DECODE);

    m_p->searchRequest = 0;
    m_p->presentRequest = 0;
    int v = z_SearchRequest (encode, &sr, 1, 0);
    if (v)
    {
        int len;
        char *buf = odr_getbuf(encode, &len, 0);
        odr_setbuf(decode, buf, len, 0);
        z_SearchRequest(decode, &m_p->searchRequest, 1, 0);
        nmem_transfer(m_p->nmem, decode->mem);
    }
    odr_destroy(encode);
    odr_destroy(decode);
}

void RecordCache::copy_presentRequest(Z_PresentRequest *pr)
{
    ODR encode = odr_createmem(ODR_ENCODE);
    ODR decode = odr_createmem(ODR_DECODE);

    m_p->searchRequest = 0;
    m_p->presentRequest = 0;
    int v = z_PresentRequest (encode, &pr, 1, 0);
    if (v)
    {
        int len;
        char *buf = odr_getbuf(encode, &len, 0);
        odr_setbuf(decode, buf, len, 0);
        z_PresentRequest(decode, &m_p->presentRequest, 1, 0);
        nmem_transfer(m_p->nmem, decode->mem);
    }
    odr_destroy(encode);
    odr_destroy(decode);
}

void RecordCache::add(ODR o, Z_NamePlusRecordList *npr, int start,
                      Z_RecordComposition *comp)
{
    if (nmem_total(m_p->nmem) > m_p->max_size)
        return;
    // Insert individual records in cache
    int i;
    for (i = 0; i < npr->num_records; i++)
    {
        RecordCache_Entry *entry = (RecordCache_Entry *)
            nmem_malloc(m_p->nmem, sizeof(*entry));
        entry->m_record =
            yaz_clone_z_NamePlusRecord(npr->records[i], m_p->nmem);
        entry->m_comp = yaz_clone_z_RecordComposition(comp, m_p->nmem);
        entry->m_offset = i + start;
        entry->m_next = m_p->entries;
        m_p->entries = entry;
    }
}

void RecordCache::add(ODR o, Z_NamePlusRecordList *npr, int start,
                      int hits)
{
    // Build appropriate compspec for this response
    Z_RecordComposition *comp = 0;
    if (hits == -1 && m_p->presentRequest)
        comp = m_p->presentRequest->recordComposition;
    else if (hits > 0 && m_p->searchRequest)
    {
        Z_ElementSetNames *esn;

        if (hits <= *m_p->searchRequest->smallSetUpperBound)
            esn = m_p->searchRequest->smallSetElementSetNames;
        else
            esn = m_p->searchRequest->mediumSetElementSetNames;
        comp = (Z_RecordComposition *) nmem_malloc(m_p->nmem, sizeof(*comp));
        comp->which = Z_RecordComp_simple;
        comp->u.simple = esn;
    }
    add(o, npr, start, comp);
}

int RecordCache::Rep::match(RecordCache_Entry *entry,
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

int RecordCache::lookup(ODR o, Z_NamePlusRecordList **npr,
                        int start, int num,
                        Odr_oid *syntax,
                        Z_RecordComposition *comp)
{
    int i;
    yaz_log(YLOG_DEBUG, "cache lookup start=%d num=%d", start, num);

    for (i = 0; i<num; i++)
    {
        RecordCache_Entry *entry = m_p->entries;
        for(; entry; entry = entry->m_next)
            if (m_p->match(entry, syntax, start+i, comp))
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
        RecordCache_Entry *entry = m_p->entries;
        for(; entry; entry = entry->m_next)
            if (m_p->match(entry, syntax, start+i, comp))
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
 * c-file-style: "Stroustrup"
 * indent-tabs-mode: nil
 * End:
 * vim: shiftwidth=4 tabstop=8 expandtab
 */

