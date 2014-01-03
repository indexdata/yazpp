/* This file is part of the yazpp toolkit.
 * Copyright (C) Index Data 
 * See the file LICENSE for details.
 */

#if HAVE_CONFIG_H
#include <config.h>
#endif
#include <stdlib.h>
#include <yazpp/gdu.h>
#include <yaz/test.h>
#include <yaz/log.h>

using namespace yazpp_1;

static void tst1(void)
{
    ODR odr = odr_createmem(ODR_ENCODE);

    const char *url =
        "http://localhost:9036/XXX/cproxydebug-7/node102/p/105/c=content_connector"
        "a=usr/pw#&? r=cfusr/cfpw p=1.2.3.4:80/www.indexdata.com/staff/";
    int use_full_host = 0;
    Z_GDU *gdu_req = z_get_HTTP_Request_uri(odr, url, 0, use_full_host);

    GDU a(gdu_req);
    GDU b;

    b = a;

    odr_destroy(odr);
}

int main(int argc, char **argv)
{
    YAZ_CHECK_INIT(argc, argv);
    tst1();
    YAZ_CHECK_TERM;
}

/*
 * Local variables:
 * c-basic-offset: 4
 * c-file-style: "Stroustrup"
 * indent-tabs-mode: nil
 * End:
 * vim: shiftwidth=4 tabstop=8 expandtab
 */

