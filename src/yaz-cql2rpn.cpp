/* This file is part of the yazpp toolkit.
 * Copyright (C) Index Data 
 * See the file LICENSE for details.
 */

#if HAVE_CONFIG_H
#include <config.h>
#endif
#include <yaz/log.h>
#include <yaz/diagsrw.h>
#include <yaz/pquery.h>
#include <yaz/sortspec.h>
#include <yazpp/cql2rpn.h>
#include <yaz/rpn2cql.h>

using namespace yazpp_1;

Yaz_cql2rpn::Yaz_cql2rpn()
{
    m_transform = 0;
}

Yaz_cql2rpn::~Yaz_cql2rpn()
{
    if (m_transform)
        cql_transform_close(m_transform);
}

void Yaz_cql2rpn::set_pqf_file(const char *fname)
{
    if (!m_transform)
        m_transform = cql_transform_open_fname(fname);
}


bool Yaz_cql2rpn::parse_spec_file(const char *fname, int *error)
{
    *error = 0;
    cql_transform_close(m_transform);
    m_transform = cql_transform_open_fname(fname);
    return m_transform ? true : false;
}

int Yaz_cql2rpn::rpn2cql_transform(Z_RPNQuery *q, WRBUF cql, ODR o,
                                   char **addinfop)
{
    int r = cql_transform_rpn2cql_wrbuf(m_transform, cql, q);
    *addinfop = 0;
    if (r)
    {
        const char *addinfo = 0;
        r = cql_transform_error(m_transform, &addinfo);
        *addinfop = odr_strdup_null(o, addinfo);
    }
    return r;
}

int Yaz_cql2rpn::query_transform(const char *cql_query,
                                 Z_RPNQuery **rpnquery, ODR o,
                                 char **addinfop)
{
    const char *addinfo = 0;
    if (!m_transform)
        return -3;
    CQL_parser cp = cql_parser_create();

    int r = cql_parser_string(cp, cql_query);
    if (r)
    {
        r = YAZ_SRW_QUERY_SYNTAX_ERROR;
    }
    else
    {
        WRBUF pqf = wrbuf_alloc();
        r = cql_transform(m_transform, cql_parser_result(cp),
                          wrbuf_vp_puts, pqf);
        if (!r)
        {
            WRBUF sortkeys = wrbuf_alloc();
            WRBUF sortspec = wrbuf_alloc();
            if (cql_sortby_to_sortkeys(cql_parser_result(cp),
                                       wrbuf_vp_puts, sortkeys))
            {
                r = YAZ_SRW_UNSUPP_SORT_TYPE;
            }
            else
            {
                yaz_srw_sortkeys_to_sort_spec(wrbuf_cstr(sortkeys), sortspec);
                Z_SortKeySpecList *sksl =
                    yaz_sort_spec(o, wrbuf_cstr(sortspec));
                if (sksl)
                    yaz_sort_spec_to_type7(sksl, pqf);
            }
            wrbuf_destroy(sortspec);
            wrbuf_destroy(sortkeys);

            YAZ_PQF_Parser pp = yaz_pqf_create();

            *rpnquery = yaz_pqf_parse(pp, o, wrbuf_cstr(pqf));
            if (!*rpnquery)
            {
                size_t off;
                const char *pqf_msg;
                yaz_pqf_error(pp, &pqf_msg, &off);
                r = -1;
            }
            yaz_pqf_destroy(pp);
        }
        else
        {
            r = cql_transform_error(m_transform, &addinfo);
        }
        wrbuf_destroy(pqf);
    }
    cql_parser_destroy(cp);
    *addinfop = odr_strdup_null(o, addinfo);
    return r;
}
/*
 * Local variables:
 * c-basic-offset: 4
 * c-file-style: "Stroustrup"
 * indent-tabs-mode: nil
 * End:
 * vim: shiftwidth=4 tabstop=8 expandtab
 */

