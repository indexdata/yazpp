// $Header: /home/cvsroot/yaz++/zoom/zexcept.cpp,v 1.8 2003-07-02 10:25:13 adam Exp $

// Z39.50 Exception classes

#include <iostream>
#include <errno.h>
#include <string.h>		// for strerror(), strlen(), strcpy()
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
	static char buf[40];
	sprintf(buf, "error #%d", code);
	return buf;
    }



    systemException::systemException() : exception(errno){
	code = errno;
    }

    std::string systemException::errmsg() const {
	return strerror(code);
    }


    
    bib1Exception::bib1Exception(int errcode, const std::string &addinfo) :
	exception(errcode), info(addinfo) {
	std::cerr << "WARNING: made bib1Exception(" << errcode << "=" <<
	    ZOOM_diag_str(errcode) << ", '" << addinfo << "')\n";
    }

    bib1Exception::~bib1Exception() {
	//fprintf(stderr, "deleting bib1Exception 0x%lx (%d, 0x%lx=%s)\n",
		//(long) this, code, (long) info, info);
	//delete info;
	//  ###	Don't actually do the deletion for now.  Exception
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
