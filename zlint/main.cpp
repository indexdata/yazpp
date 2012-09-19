#if HAVE_CONFIG_H
#include <config.h>
#endif
#include <stdlib.h>

#include <yazpp/pdu-assoc.h>
#include <yazpp/socket-manager.h>

#include <zlint.h>

int main(int argc, char **argv)
{
    SocketManager mySocketManager;
    Zlint z(new PDU_Assoc(&mySocketManager));

    if (argc > 1)
        z.set_host(argv[1]);
    else
        z.set_host("localhost:9999");

    Zlint_test_init_01 t01;
    z.add_test(&t01);

    Zlint_test_init_02 t02;
    z.add_test(&t02);

    Zlint_test_init_03 t03;
    z.add_test(&t03);

    Zlint_test_init_04 t04;
    z.add_test(&t04);

    Zlint_test_init_05 t05;
    z.add_test(&t05);

    Zlint_test_init_06 t06;
    z.add_test(&t06);

    Zlint_test_init_07 t07;
    z.add_test(&t07);

    Zlint_test_init_08 t08;
    z.add_test(&t08);

    Zlint_test_search_01 s01;
    z.add_test(&s01);

    Zlint_test_scan_01 scan01;
    z.add_test(&scan01);

    while (mySocketManager.processEvent() > 0)
        ;
    exit (0);
}

/*
 * Local variables:
 * c-basic-offset: 4
 * c-file-style: "Stroustrup"
 * indent-tabs-mode: nil
 * End:
 * vim: shiftwidth=4 tabstop=8 expandtab
 */

