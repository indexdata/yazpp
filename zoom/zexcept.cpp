// Z39.50 Exception classes

#if HAVE_CONFIG_H
#include <config.h>
#endif
#include <iostream>
#include <errno.h>
#include <string.h>             // for strerror(), strlen(), strcpy()
#include <stdio.h>
#include "zoom.h"


namespace ZOOM {
    exception::exception(int errcode) {
        code = errcode;
    }

    exception::~exception() {
        // Nothing to do, but G++ requires this to be explicit anyway
    }

    int exception::errcode() const {
        return code;
    }

    std::string exception::errmsg() const {
        char buf[40];
        sprintf(buf, "error #%d", code);
        return buf;
    }



    systemException::systemException() : exception(errno){
        code = errno;
    }

    std::string systemException::errmsg() const {
        // For thread safety on linux (and most unix systems), we need
        // to use the reentrant version of the error translation
        // function.  Microsoft's strerror() is thread safe, since it
        // returns a pointer to thread local storage.  Unfortunately
        // there several different reentrant versions.  Here, we check
        // for glibc, since we are using gcc.  It appears at least the
        // current version of gcc has strerror_r() available by
        // default.
        #ifdef __GLIBC__
            char buf[1024];
            // PJD: result not necessarily equal to buf
            const char* result = strerror_r(code, buf, sizeof(buf));
            if (result != 0)
                return result;
            return exception::errmsg();
        #else
            return strerror(code);
        #endif
    }



    bib1Exception::bib1Exception(int errcode, const std::string &addinfo) :
        exception(errcode), info(addinfo) {
        // std::cerr << "WARNING: made bib1Exception(" << errcode << "=" <<
        //   ZOOM_diag_str(errcode) << ", '" << addinfo << "')\n";
    }

    bib1Exception::~bib1Exception() {
        //fprintf(stderr, "deleting bib1Exception 0x%lx (%d, 0x%lx=%s)\n",
                //(long) this, code, (long) info, info);
        //delete info;
        //  ### Don't actually do the deletion for now.  Exception
        //  reference semantics are too weird for me to grok so I'm
        //  doing The Wrong Thing in the knowledge that it will more
        //  or less work -- it just leaks memory.  (Or does it?)
    }

    std::string bib1Exception::errmsg() const {
        return ZOOM_diag_str(code);
    }

    std::string bib1Exception::addinfo() const {
        return info;
    }



    queryException::queryException(int qtype, const std::string &source) :
        exception(qtype), q(source) {}

    queryException::~queryException() {
        //delete q; // ### see comment on bib1Exception destructor
    }

    std::string queryException::errmsg() const {
        switch (code) {
        case PREFIX: return "bad prefix search";
        case CCL: return "bad CCL search";
        default: break;
        }
        return "bad search (unknown type)";
    }

    std::string queryException::addinfo() const {
        return q;
    }
}
/*
 * Local variables:
 * c-basic-offset: 4
 * c-file-style: "Stroustrup"
 * indent-tabs-mode: nil
 * End:
 * vim: shiftwidth=4 tabstop=8 expandtab
 */

