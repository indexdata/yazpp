/*
 * Copyright (c) 2001, Index Data.
 * See the file LICENSE for details.
 * 
 * $Id: z-databases.h,v 1.3 2005-06-25 15:53:19 adam Exp $
 */

#include <yaz/proto.h>

namespace yazpp_1 {
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
};
/*
 * Local variables:
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * End:
 * vim: shiftwidth=4 tabstop=8 expandtab
 */

