/*
 * Copyright (c) 1998-2000, Index Data.
 * See the file LICENSE for details.
 * 
 * $Id: yaz-query.h,v 1.1 2000-10-11 11:58:16 adam Exp $
 */


/** Query
    Generic Query.
*/
class YAZ_EXPORT Yaz_Query {
 public:
    /// Print query in buffer described by str and len
    virtual void print (char *str, int len) = 0;
};

