// $Header: /home/cvsroot/yaz++/zoom/Attic/zerr.cpp,v 1.1 2002-08-08 13:31:54 mike Exp $

// Z39.50 Error classes

#include <errno.h>
#include <string.h>		// for strerror(), strlen(), strcpy()
#include <stdio.h>		// for sprintf()
#include <yaz/diagbib1.h>
#include "zoom++.h"


namespace ZOOM {
    error::error(int errcode) {
	code = errcode;
    }

    int error::errcode() const {
	return code;
    }

    const char *error::errmsg() const {
	static char buf[40];
	sprintf(buf, "error #%d", code);
	return buf;
    }



    systemError::systemError() : error::error(errno){
	code = errno;
    }

    int systemError::errcode() const {
	return code;
    }

    const char *systemError::errmsg() const {
	return strerror(code);
    }



    bib1Error::bib1Error(int errcode, const char *addinfo) :
	error::error(errcode) {
	info = new char[strlen(addinfo)+1];
	strcpy((char*) info, addinfo);
    }

    bib1Error::~bib1Error() {
	delete info;
    }

    int bib1Error::errcode() const {
	return code;
    }

    const char *bib1Error::errmsg() const {
	return diagbib1_str(code);
    }

    const char *bib1Error::addinfo() const {
	return info;
    }



    queryError::queryError(int qtype, const char *source) :
	error::error(qtype) {
	q = new char[strlen(source)+1];
	strcpy((char*) q, source);
    }

    queryError::~queryError() {
	delete q;
    }

    int queryError::errcode() const {
	return code;
    }

    const char *queryError::errmsg() const {
	switch (code) {
	case PREFIX: return "bad prefix search";
	case CCL: return "bad CCL search";
	default: break;
	}
	return "bad search (unknown type)";
    }

    const char *queryError::addinfo() const {
	return q;
    }
}
