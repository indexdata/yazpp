/*
 * Copyright (c) 1998-2005, Index Data.
 * See the file LICENSE for details.
 * 
 * $Id: query.h,v 1.2 2007-03-20 07:54:11 adam Exp $
 */

#ifndef YAZ_PP_QUERY_H
#define YAZ_PP_QUERY_H

#include <stddef.h>
#include <yaz/yconfig.h>

namespace yazpp_1 {
/** Query
    Generic Query.
*/
class YAZ_EXPORT Yaz_Query {
 public:
    /// Print query in buffer described by str and len
    virtual void print (char *str, size_t len) = 0;
    virtual ~Yaz_Query();
};
};

#endif

/*
 * Local variables:
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * End:
 * vim: shiftwidth=4 tabstop=8 expandtab
 */

