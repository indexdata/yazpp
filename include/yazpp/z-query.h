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

#ifndef YAZPP_Z_QUERY_INCLUDED
#define YAZPP_Z_QUERY_INCLUDED

#include <yaz/proto.h>
#include <yazpp/query.h>

namespace yazpp_1 {
/** Z39.50 Query
    RPN, etc.
*/
class YAZ_EXPORT Yaz_Z_Query : public Yaz_Query {
 public:
    /// Construct query
    Yaz_Z_Query();
    /// Copy construct
    Yaz_Z_Query(const Yaz_Z_Query &q);
    /// Delete Query
    virtual ~Yaz_Z_Query();
    /// Set RPN
    int set_rpn (const char *rpn);
    /// Set Z Query
    void set_Z_Query (Z_Query *z_query);
    /// Get Z Query
    Z_Query *get_Z_Query ();
    /// print query
    void print(char *str, size_t len);
    /// match query
    int match(const Yaz_Z_Query *other);
    /// Copy
    Yaz_Z_Query &operator=(const Yaz_Z_Query &);
    /// Assign RPN string to it
    Yaz_Z_Query& operator=(const char *rpn);
 private:
    class Rep;
    Rep *m_p;
};
};
#endif
/*
 * Local variables:
 * c-basic-offset: 4
 * c-file-style: "Stroustrup"
 * indent-tabs-mode: nil
 * End:
 * vim: shiftwidth=4 tabstop=8 expandtab
 */

