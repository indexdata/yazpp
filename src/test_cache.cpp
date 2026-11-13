/* This file is part of the yazpp toolkit.
 * Copyright (C) Index Data
 * See the file LICENSE for details.
 */

#if HAVE_CONFIG_H
#include <config.h>
#endif
#include <yazpp/record-cache.h>
#include <yaz/proto.h>
#include <yaz/oid_db.h>
#include <yaz/test.h>
#include <cstring>

using namespace yazpp_1;

namespace {

Z_RecordComposition *composition(ODR o, const char *name)
{
    Z_RecordComposition *comp = (Z_RecordComposition *)
        odr_malloc(o, sizeof(*comp));
    comp->which = Z_RecordComp_simple;
    comp->u.simple = (Z_ElementSetNames *)
        odr_malloc(o, sizeof(*comp->u.simple));
    comp->u.simple->which = Z_ElementSetNames_generic;
    comp->u.simple->u.generic = odr_strdup(o, name);
    return comp;
}

class Fixture {
public:
    RecordCache cache;
    ODR input;
    ODR output;

    Fixture() : input(odr_createmem(ODR_ENCODE)),
                output(odr_createmem(ODR_ENCODE)) {}
    ~Fixture()
    {
        odr_destroy(input);
        odr_destroy(output);
    }

    Z_NamePlusRecordList *records()
    {
        Z_NamePlusRecordList *list = (Z_NamePlusRecordList *)
            odr_malloc(input, sizeof(*list));
        list->num_records = 2;
        list->records = (Z_NamePlusRecord **)
            odr_malloc(input, 2 * sizeof(*list->records));
        const char *data[] = {"one", "two"};
        for (int i = 0; i < 2; i++)
        {
            Z_NamePlusRecord *record = (Z_NamePlusRecord *)
                odr_malloc(input, sizeof(*record));
            record->databaseName = odr_strdup(input, "test");
            record->which = Z_NamePlusRecord_databaseRecord;
            record->u.databaseRecord = z_ext_record_oid(
                input, yaz_oid_recsyn_sutrs, data[i], strlen(data[i]));
            list->records[i] = record;
        }
        return list;
    }

    void lookup(int start, int num, Z_RecordComposition *comp, int expected,
                const Odr_oid *syntax = yaz_oid_recsyn_sutrs,
                int first = 0)
    {
        odr_reset(output);
        Z_NamePlusRecordList *result = 0;
        int found = cache.lookup(output, &result, start, num,
                                 const_cast<Odr_oid *>(syntax), comp);
        YAZ_CHECK_EQ(found, expected);
        if (!found)
            return;
        YAZ_CHECK(result);
        if (!result)
            return;
        YAZ_CHECK_EQ(result->num_records, num);
        if (result->num_records != num)
            return;
        const char *data[] = {"one", "two"};
        for (int i = 0; i < num; i++)
        {
            Z_NamePlusRecord *record = result->records[i];
            YAZ_CHECK(record && record->databaseName &&
                      !strcmp(record->databaseName, "test"));
            YAZ_CHECK(record &&
                      record->which == Z_NamePlusRecord_databaseRecord);
            if (!record || record->which != Z_NamePlusRecord_databaseRecord)
                continue;
            Z_External *ext = record->u.databaseRecord;
            YAZ_CHECK(ext && ext->which == Z_External_sutrs);
            if (!ext || ext->which != Z_External_sutrs)
                continue;
            YAZ_CHECK(ext->direct_reference &&
                      !oid_oidcmp(ext->direct_reference, syntax));
            const char *value = data[first + i];
            YAZ_CHECK(ext->u.sutrs &&
                      ext->u.sutrs->len == (int) strlen(value) &&
                      !memcmp(ext->u.sutrs->buf, value, strlen(value)));
        }
    }
};

void test_add_lookup_clear()
{
    Fixture f;
    f.lookup(5, 1, 0, 0);
    Z_NamePlusRecordList empty = {0, 0};
    f.cache.add(f.input, &empty, 5, static_cast<Z_RecordComposition *>(0));
    f.lookup(5, 1, 0, 0);

    f.cache.add(f.input, f.records(), 5,
                static_cast<Z_RecordComposition *>(0));
    // Both records and their contents must survive the caller's ODR reset.
    odr_reset(f.input);
    f.lookup(5, 2, 0, 1);
    f.lookup(6, 1, 0, 1, yaz_oid_recsyn_sutrs, 1);
    f.lookup(4, 1, 0, 0);
    f.lookup(5, 3, 0, 0);
    f.lookup(5, 1, 0, 0, yaz_oid_recsyn_usmarc);
    f.lookup(5, 1, 0, 0, 0);

    f.cache.clear();
    f.lookup(5, 1, 0, 0);
    f.cache.clear();
    f.cache.add(f.input, f.records(), 1, -1);
    f.lookup(1, 2, 0, 1);
}

void test_compositions()
{
    Fixture f;
    f.cache.add(f.input, f.records(), 1, composition(f.input, "F"));
    f.cache.add(f.input, f.records(), 1, composition(f.input, "B"));
    odr_reset(f.input);

    Z_RecordComposition *full = composition(f.input, "F");
    Z_RecordComposition *brief = composition(f.input, "B");
    f.lookup(1, 2, full, 1);
    f.lookup(1, 2, brief, 1);
    f.lookup(1, 2, composition(f.input, "other"), 0);
    f.lookup(1, 2, 0, 0);
    f.cache.add(f.input, f.records(), 3,
                static_cast<Z_RecordComposition *>(0));
    f.lookup(3, 2, 0, 1);
    f.lookup(3, 2, full, 0);
}

void test_requests()
{
    Fixture f;
    Z_SearchRequest *search = zget_APDU(f.input, Z_APDU_searchRequest)
        ->u.searchRequest;
    search->num_databaseNames = 1;
    search->databaseNames = (char **) odr_malloc(f.input, sizeof(char *));
    search->databaseNames[0] = odr_strdup(f.input, "test");
    search->query = (Z_Query *) odr_malloc(f.input, sizeof(*search->query));
    search->query->which = Z_Query_type_2;
    search->query->u.type_2 = odr_create_Odr_oct(f.input, "test", 4);
    *search->smallSetUpperBound = 2;
    search->smallSetElementSetNames = composition(f.input, "F")->u.simple;
    search->mediumSetElementSetNames = composition(f.input, "B")->u.simple;
    f.cache.copy_searchRequest(search);
    odr_reset(f.input);

    f.cache.add(f.input, f.records(), 1, 2);
    f.cache.add(f.input, f.records(), 3, 3);
    Z_RecordComposition *full = composition(f.input, "F");
    Z_RecordComposition *brief = composition(f.input, "B");
    f.lookup(1, 2, full, 1);
    f.lookup(1, 2, brief, 0);
    f.lookup(3, 2, brief, 1);
    f.lookup(3, 2, full, 0);

    Z_PresentRequest *present = zget_APDU(f.input, Z_APDU_presentRequest)
        ->u.presentRequest;
    present->recordComposition = full;
    f.cache.copy_presentRequest(present);
    odr_reset(f.input);
    f.cache.add(f.input, f.records(), 5, -1);
    full = composition(f.input, "F");
    f.lookup(5, 2, full, 1);
    f.lookup(5, 2, 0, 0);
    // Copying a present request discards the previous search request.
    f.cache.add(f.input, f.records(), 7, 3);
    f.lookup(7, 2, 0, 1);

    f.cache.clear();
    f.cache.add(f.input, f.records(), 1, -1);
    f.lookup(1, 2, 0, 1);
    f.lookup(1, 2, full, 0);

    // A present request may itself omit the composition.
    present = zget_APDU(f.input, Z_APDU_presentRequest)->u.presentRequest;
    f.cache.copy_presentRequest(present);
    f.cache.add(f.input, f.records(), 3, -1);
    f.lookup(3, 2, 0, 1);
}

void test_max_size()
{
    Fixture f;
    Z_NamePlusRecordList *list = f.records();
    f.cache.add(f.input, list, 1, -1);
    // Lowering the limit stops additions without evicting existing records.
    f.cache.set_max_size(0);
    f.cache.add(f.input, list, 3, -1);
    f.lookup(1, 2, 0, 1);
    f.lookup(3, 2, 0, 0);
    f.cache.set_max_size(200000);
    f.cache.add(f.input, list, 3, -1);
    f.lookup(3, 2, 0, 1);
}

}

int main(int argc, char **argv)
{
    YAZ_CHECK_INIT(argc, argv);
    test_add_lookup_clear();
    test_compositions();
    test_requests();
    test_max_size();
    YAZ_CHECK_TERM;
}

/*
 * Local variables:
 * c-basic-offset: 4
 * c-file-style: "Stroustrup"
 * indent-tabs-mode: nil
 * End:
 * vim: shiftwidth=4 tabstop=8 expandtab
 */
