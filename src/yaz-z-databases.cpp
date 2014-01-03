/* This file is part of the yazpp toolkit.
 * Copyright (C) Index Data 
 * See the file LICENSE for details.
 */

#if HAVE_CONFIG_H
#include <config.h>
#endif
#include <string.h>

#include <yazpp/z-databases.h>

using namespace yazpp_1;

Yaz_Z_Databases::Yaz_Z_Databases()
{
    nmem = nmem_create ();
    m_num = 0;
    m_list = 0;
}

Yaz_Z_Databases::~Yaz_Z_Databases()
{
    nmem_destroy (nmem);
}

void Yaz_Z_Databases::set (int num, const char **db)
{
    nmem_reset (nmem);

    m_list = (char **) nmem_malloc (nmem, num * sizeof(char*));
    m_num = num;
    for (int i = 0; i<num; i++)
        m_list[i] = nmem_strdup (nmem, db[i] ? db[i] : "Default");
}

void Yaz_Z_Databases::get (NMEM n, int *num, char ***db)
{
    *num = m_num;
    *db = (char **) nmem_malloc (n, m_num * sizeof(char*));
    for (int i = 0; i < m_num; i++)
        (*db)[i] = nmem_strdup (n, m_list[i]);
}

void Yaz_Z_Databases::get (ODR o, int *num, char ***db)
{
    get (o->mem, num, db);
}

int Yaz_Z_Databases::match (Yaz_Z_Databases &db)
{
    if (db.m_num != m_num)
        return 0;
    for (int i = 0; i<m_num; i++)
        if (strcmp (m_list[i], db.m_list[i]))
            return 0;
    return 1;
}

int Yaz_Z_Databases::match (int num, const char **db)
{
    if (num != m_num)
        return 0;
    for (int i = 0; i<m_num; i++)
        if (strcmp (m_list[i], db[i]))
            return 0;
    return 1;
}
/*
 * Local variables:
 * c-basic-offset: 4
 * c-file-style: "Stroustrup"
 * indent-tabs-mode: nil
 * End:
 * vim: shiftwidth=4 tabstop=8 expandtab
 */

