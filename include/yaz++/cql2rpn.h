/*
 * Copyright (c) 1998-2004, Index Data.
 * See the file LICENSE for details.
 * 
 * $Id: cql2rpn.h,v 1.1 2004-03-29 22:46:50 adam Exp $
 */

#include <yaz/cql.h>
#include <yaz/z-core.h>

class YAZ_EXPORT Yaz_cql2rpn {
 public:
    Yaz_cql2rpn();
    ~Yaz_cql2rpn();
    void set_pqf_file(const char *fname);
    int query_transform(const char *cql, Z_RPNQuery **rpnquery, ODR o,
			char **addinfop);
 private:
    cql_transform_t m_transform;
};

