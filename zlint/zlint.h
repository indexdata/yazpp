/* This file is part of the yazpp toolkit.
 * Copyright (C) Index Data 
 * All rights reserved.
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of Index Data nor the names of its contributors
 *       may be used to endorse or promote products derived from this
 *       software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE REGENTS AND CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <yazpp/z-assoc.h>

using namespace yazpp_1;

enum Zlint_code {
    TEST_FINISHED,
    TEST_CONTINUE,
    TEST_REOPEN
};

class Zlint_test;
class Zlint_t;

class Zlint : public Z_Assoc {
public:
    Zlint(IPDU_Observable *the_PDU_Observable);
    ~Zlint();
    void add_test(Zlint_test *i);
    void set_host(const char *cp);
    int initResponseGetVersion(Z_InitResponse *init);
    Z_ReferenceId *mk_refid(const char *buf, int len);
    void msg_check_for(const char *fmt, ...);
    void msg_check_ok();
    void msg_check_fail(const char *fmt, ...);
    void msg_check_info(const char *fmt, ...);
    void msg_check_notapp();
    void getDatabase(char ***db, int *num);
private:
    void connectNotify();
    void timeoutNotify();
    void failNotify();
    void recv_GDU(Z_GDU *apdu, int len);
    IPDU_Observable *m_PDU_Observable;
    IPDU_Observer *sessionNotify(IPDU_Observable *the_PDU_Observable, int fd);
    Zlint_t *m_tests;
    Zlint_t *m_cur_test;
    char *m_host;
    char *m_database;
    void close_goto_next();
};

class Zlint_test {
public:
    virtual Zlint_code init(Zlint *z) = 0;
    virtual Zlint_code recv_gdu(Zlint *z, Z_GDU *gdu) = 0;
    virtual Zlint_code recv_fail(Zlint *z, int reason) = 0;
    virtual ~Zlint_test();
};

class Zlint_test_simple : public Zlint_test {
public:
    virtual Zlint_code init(Zlint *z) = 0;
    virtual Zlint_code recv_gdu(Zlint *z, Z_GDU *gdu) = 0;
    virtual Zlint_code recv_fail(Zlint *z, int reason);
};

class Zlint_test_init_01 : public Zlint_test_simple {
public:
    Zlint_test_init_01();
    virtual ~Zlint_test_init_01();
    Zlint_code init(Zlint *z);
    Zlint_code recv_gdu(Zlint *z, Z_GDU *gdu);
};

class Zlint_test_init_02 : public Zlint_test_simple {
public:
    Zlint_test_init_02();
    virtual ~Zlint_test_init_02();
    Zlint_code init(Zlint *z);
    Zlint_code recv_gdu(Zlint *z, Z_GDU *gdu);
};

class Zlint_test_init_03 : public Zlint_test_simple {
public:
    Zlint_test_init_03();
    virtual ~Zlint_test_init_03();
    Zlint_code init(Zlint *z);
    Zlint_code recv_gdu(Zlint *z, Z_GDU *gdu);
};

class Zlint_test_init_04 : public Zlint_test_simple {
public:
    Zlint_test_init_04();
    virtual ~Zlint_test_init_04();
    Zlint_code init(Zlint *z);
    Zlint_code recv_gdu(Zlint *z, Z_GDU *gdu);
};

class Zlint_test_init_05 : public Zlint_test_simple {
    int m_init_response_no;
public:
    Zlint_test_init_05();
    virtual ~Zlint_test_init_05();
    Zlint_code init(Zlint *z);
    Zlint_code recv_gdu(Zlint *z, Z_GDU *gdu);
};

class Zlint_test_init_06 : public Zlint_test_simple {
public:
    Zlint_test_init_06();
    virtual ~Zlint_test_init_06();
    Zlint_code init(Zlint *z);
    Zlint_code recv_gdu(Zlint *z, Z_GDU *gdu);
};

class Zlint_test_init_07 : public Zlint_test_simple {
public:
    Zlint_test_init_07();
    virtual ~Zlint_test_init_07();
    Zlint_code init(Zlint *z);
    Zlint_code recv_gdu(Zlint *z, Z_GDU *gdu);
};

class Zlint_test_init_08 : public Zlint_test {
    int m_no;
public:
    Zlint_test_init_08();
    virtual ~Zlint_test_init_08();
    Zlint_code init(Zlint *z);
    Zlint_code recv_gdu(Zlint *z, Z_GDU *gdu);
    Zlint_code recv_fail(Zlint *z, int reason);
};

class Zlint_test_search_01 : public Zlint_test {
    int m_query_no;
    int m_got_result_set;
    int m_record_syntax_no;
    int m_sort_no;
    Zlint_code sendTest(Zlint *z);
public:
    Zlint_test_search_01();
    virtual ~Zlint_test_search_01();
    Zlint_code init(Zlint *z);
    Zlint_code recv_gdu(Zlint *z, Z_GDU *gdu);
    Zlint_code recv_fail(Zlint *z, int reason);
};

class Zlint_test_scan_01 : public Zlint_test {
    int m_scan_no;
    Zlint_code sendTest(Zlint *z);
public:
    Zlint_test_scan_01();
    virtual ~Zlint_test_scan_01();
    Zlint_code init(Zlint *z);
    Zlint_code recv_gdu(Zlint *z, Z_GDU *gdu);
    Zlint_code recv_fail(Zlint *z, int reason);
};
/*
 * Local variables:
 * c-basic-offset: 4
 * c-file-style: "Stroustrup"
 * indent-tabs-mode: nil
 * End:
 * vim: shiftwidth=4 tabstop=8 expandtab
 */

