/*
 * Copyright (c) 2001, Index Data.
 * See the file LICENSE for details.
 * 
 * $Id: yaz-z-databases.h,v 1.1 2001-11-06 17:08:05 adam Exp $
 */

#include <yaz/proto.h>

/** Z39.50 Databases list
 */
class YAZ_EXPORT Yaz_Z_Databases {
public:
/// Make Query from rpn string
    Yaz_Z_Databases();
    ~Yaz_Z_Databases();
    void set (int num, const char **db);
    void get (NMEM n, int *num, char ***db);
    void get (ODR o, int *num, char ***db);
    int match (Yaz_Z_Databases &db);
    int match (int num, const char **db);
 private:
    char **m_list;
    int m_num;
    NMEM nmem;
};
