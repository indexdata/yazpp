/*
 * Copyright (c) 1998-2000, Index Data.
 * See the file LICENSE for details.
 * 
 * $Id: yaz-z-query.h,v 1.7 2000-09-08 10:23:42 adam Exp $
 */

#include <yaz/proto.h>
#include <yaz-query.h>

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
 private:
    char *buf;
    int len;
    ODR odr_decode;
    ODR odr_encode;
    ODR odr_print;
};
