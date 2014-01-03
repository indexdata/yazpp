/* This file is part of the yazpp toolkit.
 * Copyright (C) Index Data 
 * See the file LICENSE for details.
 */

#if HAVE_CONFIG_H
#include <config.h>
#endif
#include <stdio.h>
#include <stdarg.h>

#include <yaz/comstack.h>
#include <yaz/options.h>
#include <yaz/otherinfo.h>
#include <yaz/charneg.h>
#include <yaz/log.h>
#include <yaz/odr.h>

#include <zlint.h>

Zlint_test::~Zlint_test()
{

}

class Zlint_t {
public:
    friend class Zlint;
    Zlint_t(Zlint_test *t);
    ~Zlint_t();
private:
    Zlint_test *m_t;
    Zlint_t *m_next;
    int m_test_number_sequence;
    int m_test_ok;
    int m_test_reported;
};

Zlint::Zlint(IPDU_Observable *the_PDU_Observable) :
    Z_Assoc(the_PDU_Observable)

{
    m_PDU_Observable = the_PDU_Observable;
    m_host = 0;
    m_tests = 0;
    m_cur_test = 0;
    m_database = 0;
}

Zlint::~Zlint()
{
    while (m_tests)
    {
        Zlint_t *t = m_tests;
        m_tests = t->m_next;
        delete t;
    }
    xfree(m_host);
    xfree(m_database);
}

void Zlint::set_host(const char *cp)
{
    xfree(m_host);
    m_host = xstrdup(cp);
    client(m_host);
    timeout(30);

    const char *basep;
    cs_get_host_args(m_host, &basep);
    if (!basep || !*basep)
        basep = "Default";
    xfree(m_database);
    m_database = xstrdup(basep);
}

void Zlint::timeoutNotify()
{
    if (m_cur_test)
    {
        if (m_cur_test->m_t->recv_fail(this, 2) != TEST_FINISHED)
        {
            client(m_host);
            timeout(30);
            return;
        }
        close_goto_next();
    }
}

void Zlint::failNotify()
{
    if (m_cur_test)
    {
        if (m_cur_test->m_t->recv_fail(this, 1) != TEST_FINISHED)
        {
            client(m_host);
            timeout(30);
            return;
        }
        close_goto_next();
    }
}

void Zlint::connectNotify()
{
    if (m_cur_test)
    {
        if (m_cur_test->m_t->init(this) != TEST_FINISHED)
            return;
        close_goto_next();
    }
}

void Zlint::recv_GDU(Z_GDU *gdu, int len)
{
    if (m_cur_test)
    {
        int r = m_cur_test->m_t->recv_gdu(this, gdu);
        if (r == TEST_CONTINUE)
            return;
        if (r == TEST_REOPEN)
        {
            client(m_host);
            timeout(30);
            return;
        }
        close_goto_next();
    }
}

void Zlint::close_goto_next()
{
    if (m_cur_test)
        m_cur_test = m_cur_test->m_next;
    if (m_cur_test)
    {
        client(m_host);
        timeout(30);
    }
    else
        close();
}

IPDU_Observer *Zlint::sessionNotify(
    IPDU_Observable *the_PDU_Observable, int fd)
{
    return 0;
}

Z_ReferenceId *Zlint::mk_refid(const char *buf, int len)
{
    return odr_create_Odr_oct(odr_encode(),
#if YAZ_VERSIONL < 0x50000
                              (unsigned char *)
#endif
                              buf, len);
}

int Zlint::initResponseGetVersion(Z_InitResponse *init)
{
    int no = 0;
    int i;
    for (i = 0; i<12; i++)
        if (ODR_MASK_GET(init->protocolVersion, no))
        {
            no = i+1;
        }
    return no;
}

void Zlint::add_test(Zlint_test *t)
{
    Zlint_t **d = &m_tests;
    while (*d)
        d = &(*d)->m_next;
    *d = new Zlint_t(t);
    if (!m_cur_test)
        m_cur_test = m_tests;
}

void Zlint::msg_check_for(const char *fmt, ...)
{
    m_cur_test->m_test_ok = 0;
    m_cur_test->m_test_number_sequence++;
    m_cur_test->m_test_reported = 0;

    va_list ap;
    va_start(ap, fmt);
    char buf[1024];
    vsnprintf(buf, sizeof(buf), fmt, ap);
    printf ("Checking %s .. ", buf);
    va_end(ap);
}

void Zlint::msg_check_info(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    char buf[1024];
    vsnprintf(buf, sizeof(buf), fmt, ap);
    printf (" %s\n", buf);
    va_end(ap);
}

void Zlint::msg_check_ok()
{
    if (!m_cur_test->m_test_reported)
    {
        m_cur_test->m_test_ok = 1;
        m_cur_test->m_test_reported = 1;
        printf ("OK\n");
    }
}

void Zlint::msg_check_fail(const char *fmt, ...)
{
    if (!m_cur_test->m_test_reported)
    {
        m_cur_test->m_test_ok = 0;
        m_cur_test->m_test_reported = 1;
        printf ("Fail\n");
    }
    va_list ap;
    va_start(ap, fmt);
    char buf[1024];
    vsnprintf(buf, sizeof(buf), fmt, ap);
    printf (" %s\n", buf);
    va_end(ap);
}

void Zlint::msg_check_notapp()
{
    if (!m_cur_test->m_test_reported)
    {
        m_cur_test->m_test_ok = 2;
        m_cur_test->m_test_reported = 1;
        printf ("Unsupported\n");
    }
}

void Zlint::getDatabase(char ***db, int *num)
{
    *db = (char**) odr_malloc(odr_encode(), 2*sizeof(char *));
    (*db)[0] = m_database;
    (*db)[1] = 0;
    *num = 1;
}

Zlint_t::Zlint_t(Zlint_test *t)
{
    m_test_number_sequence = 0;
    m_test_ok = 0;
    m_test_reported = 0;
    m_t = t;
    m_next = 0;
}

Zlint_t::~Zlint_t()
{
    delete m_t;
}

Zlint_code Zlint_test_simple::recv_fail(Zlint *z, int reason)
{
    z->msg_check_fail("target closed connection");
    return TEST_FINISHED;
}
/*
 * Local variables:
 * c-basic-offset: 4
 * c-file-style: "Stroustrup"
 * indent-tabs-mode: nil
 * End:
 * vim: shiftwidth=4 tabstop=8 expandtab
 */

