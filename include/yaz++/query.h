/*
 * Copyright (c) 1998-2000, Index Data.
 * See the file LICENSE for details.
 * 
 * $Id: query.h,v 1.3 2005-06-25 15:53:19 adam Exp $
 */

namespace yazpp_1 {
/** Query
    Generic Query.
*/
class YAZ_EXPORT Yaz_Query {
 public:
    /// Print query in buffer described by str and len
    virtual void print (char *str, int len) = 0;
};
};
/*
 * Local variables:
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * End:
 * vim: shiftwidth=4 tabstop=8 expandtab
 */

