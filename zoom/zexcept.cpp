// $Header: /home/cvsroot/yaz++/zoom/zexcept.cpp,v 1.6 2002-11-12 22:43:56 mike Exp $

// Z39.50 Exception classes

#include <errno.h>
#include <string.h>		// for strerror(), strlen(), strcpy()
#include <stdio.h>		// for sprintf()
#include <yaz/diagbib1.h>
#include "zoom.h"


namespace ZOOM {
    exception::exception(int errcode) {
	code = errcode;
    }

    int exception::errcode() const {
	return code;
    }

    const char *exception::errmsg() const {
	static char buf[40];
	sprintf(buf, "error #%d", code);
	return buf;
    }



    systemException::systemException() : exception(errno){
	code = errno;
    }

    const char *systemException::errmsg() const {
	return strerror(code);
    }


    
    bib1Exception::bib1Exception(int errcode, const char *addinfo) :
	exception(errcode) {
	info = new char[strlen(addinfo)+1];
	strcpy((char*) info, addinfo);
	//fprintf(stderr, "made new bib1Exception 0x%lx (%d, 0x%lx=%s)\n",
		//(long) this, code, (long) info, info);
    }

#if 0
    bib1Exception::bib1Exception(bib1Exception& src) :
	exception(src) {
        code = src.code;
	info = new char[strlen(src.info)+1];
	strcpy((char*) info, src.info);
	//fprintf(stderr, "copied bib1Exception 0x%lx to 0x%lx (%d, 0x%lx=%s)\n",
		//(long) &src, (long) this, code, (long) info, info);
    }
#endif

    bib1Exception::~bib1Exception() {
	//fprintf(stderr, "deleting bib1Exception 0x%lx (%d, 0x%lx=%s)\n",
		//(long) this, code, (long) info, info);
	//delete info;
	//  ###	Don't actually do the deletion for now.  Exception
	//  reference semantics are too weird for me to grok so I'm
	//  doing The Wrong Thing in the knowledge that it will more
	//  or less work -- it just leaks memory.
    }

    const char *bib1Exception::errmsg() const {
	return diagbib1_str(code);
    }

    const char *bib1Exception::addinfo() const {
	return info;
    }



    queryException::queryException(int qtype, const char *source) :
	exception(qtype) {
	q = new char[strlen(source)+1];
	strcpy((char*) q, source);
    }

    queryException::~queryException() {
	//delete q; // ### see comment on bib1Exception destructor
    }

    const char *queryException::errmsg() const {
	switch (code) {
	case PREFIX: return "bad prefix search";
	case CCL: return "bad CCL search";
	default: break;
	}
	return "bad search (unknown type)";
    }

    const char *queryException::addinfo() const {
	return q;
    }
}
