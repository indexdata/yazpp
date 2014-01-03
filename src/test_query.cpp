/* This file is part of the yazpp toolkit.
 * Copyright (C) Index Data 
 * See the file LICENSE for details.
 */

#if HAVE_CONFIG_H
#include <config.h>
#endif
#include <stdlib.h>
#include <yazpp/z-query.h>
#include <yaz/test.h>
#include <yaz/log.h>

using namespace yazpp_1;

int tst1(const char *query_str_in, const char *query_expected)
{
    Yaz_Z_Query q;

    q = query_str_in;

    Yaz_Z_Query q2;

    q2 = q;

    char query_str_out[100];
    q2.print(query_str_out, sizeof(query_str_out)-1);

    if (strcmp(query_str_out, query_expected))
    {
	yaz_log(YLOG_LOG, "query mismatch out=%s expected=%s",
		query_str_out, query_expected);
        return 0;
    }
    return 1;
}

int main(int argc, char **argv)
{
    YAZ_CHECK_INIT(argc, argv);
    YAZ_CHECK(tst1("", ""));
    YAZ_CHECK(tst1("x", "RPN @attrset Bib-1 x"));
    YAZ_CHECK(tst1("@and a b", "RPN @attrset Bib-1 @and a b"));
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

