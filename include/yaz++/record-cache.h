/*
 * Copyright (c) 2002-2004, Index Data.
 * See the file LICENSE for details.
 * 
 * $Id: record-cache.h,v 1.1 2004-03-29 22:46:50 adam Exp $
 */


#include <yaz/nmem.h>
#include <yaz/z-core.h>

class Yaz_RecordCache_Entry;

class YAZ_EXPORT Yaz_RecordCache {
 public:
    Yaz_RecordCache ();
    ~Yaz_RecordCache ();
    void add (ODR o, Z_NamePlusRecordList *npr, int start, int hits);
    
    int lookup (ODR o, Z_NamePlusRecordList **npr, int start, int num,
		Odr_oid *syntax, Z_RecordComposition *comp);
    void clear();

    void copy_searchRequest(Z_SearchRequest *sr);
    void copy_presentRequest(Z_PresentRequest *pr);
    void set_max_size(int sz);
 private:
    NMEM m_mem;
    Yaz_RecordCache_Entry *m_entries;
    Z_SearchRequest *m_searchRequest;
    Z_PresentRequest *m_presentRequest;
    int match (Yaz_RecordCache_Entry *entry,
	       Odr_oid *syntax, int offset,
	       Z_RecordComposition *comp);
    int m_max_size;
};
