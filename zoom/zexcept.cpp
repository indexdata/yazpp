// $Header: /home/cvsroot/yaz++/zoom/zexcept.cpp,v 1.1 2002-08-08 16:06:08 mike Exp $

// Z39.50 Exception classes

#include <errno.h>
#include <string.h>		// for strerror(), strlen(), strcpy()
#include <stdio.h>		// for sprintf()
#include <yaz/diagbib1.h>
#include "zoom++.h"


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



    systemException::systemException() : exception::exception(errno){
	code = errno;
    }

    int systemException::errcode() const {
	return code;
    }

    const char *systemException::errmsg() const {
	return strerror(code);
    }



    bib1Exception::bib1Exception(int errcode, const char *addinfo) :
	exception::exception(errcode) {
	info = new char[strlen(addinfo)+1];
	strcpy((char*) info, addinfo);
    }

    bib1Exception::~bib1Exception() {
	delete info;
    }

    int bib1Exception::errcode() const {
	return code;
    }

    const char *bib1Exception::errmsg() const {
	return diagbib1_str(code);
    }

    const char *bib1Exception::addinfo() const {
	return info;
    }



    queryException::queryException(int qtype, const char *source) :
	exception::exception(qtype) {
	q = new char[strlen(source)+1];
	strcpy((char*) q, source);
    }

    queryException::~queryException() {
	delete q;
    }

    int queryException::errcode() const {
	return code;
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
