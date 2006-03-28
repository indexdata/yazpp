/*
 * Copyright (c) 1998-2005, Index Data.
 * See the file LICENSE for details.
 * 
 * $Id: z-query.h,v 1.8 2006-03-28 19:51:38 adam Exp $
 */

#ifndef YAZPP_Z_QUERY_INCLUDED
#define YAZPP_Z_QUERY_INCLUDED

#include <yaz/proto.h>
#include <yaz++/query.h>

namespace yazpp_1 {
/** Z39.50 Query
    RPN, etc.
*/
class YAZ_EXPORT Yaz_Z_Query : public Yaz_Query {
 public:
    /// Make Query from rpn string
    Yaz_Z_Query();
    /// Delete Query
    virtual ~Yaz_Z_Query();
    /// Set RPN
    int set_rpn (const char *rpn);
    /// Set Z Query
    void set_Z_Query (Z_Query *z_query);
    /// Get Z Query
    Z_Query *get_Z_Query ();
    /// print query
    void print(char *str, int len);
    /// match query
    int match(Yaz_Z_Query *other);
    /// Copy
    Yaz_Z_Query &operator=(const Yaz_Z_Query &);
    /// Assign RPN string to it
    Yaz_Z_Query& operator=(const char *rpn);
 private:
    char *m_buf;
    int m_len;
    ODR odr_decode;
    ODR odr_encode;
    ODR odr_print;
    WRBUF zquery2pquery(Z_Query *q);
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

