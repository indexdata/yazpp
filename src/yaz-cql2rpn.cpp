/* This file is part of the yazpp toolkit.
 * Copyright (C) 1998-2013 Index Data and Mike Taylor
 * See the file LICENSE for details.
 */

#if HAVE_CONFIG_H
#include <config.h>
#endif
#include <yaz/log.h>
#include <yaz/diagsrw.h>
#include <yaz/pquery.h>
#include <yazpp/cql2rpn.h>

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
        WRBUF w = wrbuf_alloc();
        r = cql_transform(m_transform, cql_parser_result(cp), wrbuf_vp_puts, w);
        if (!r)
        {
            YAZ_PQF_Parser pp = yaz_pqf_create();

            *rpnquery = yaz_pqf_parse(pp, o, wrbuf_cstr(w));
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
        wrbuf_destroy(w);
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

