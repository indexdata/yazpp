/*
 * Copyright (c) 1998-2005, Index Data.
 * See the file LICENSE for details.
 * 
 * $Id: query.h,v 1.1 2006-03-29 13:14:15 adam Exp $
 */

#ifndef YAZ_PP_QUERY_H
#define YAZ_PP_QUERY_H

#include <yaz/yconfig.h>

namespace yazpp_1 {
/** Query
    Generic Query.
*/
class YAZ_EXPORT Yaz_Query {
 public:
    /// Print query in buffer described by str and len
    virtual void print (char *str, int len) = 0;
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

