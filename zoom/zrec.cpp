// $Header: /home/cvsroot/yaz++/zoom/zrec.cpp,v 1.1 2002-08-08 13:31:54 mike Exp $

// Z39.50 Record class

#include "zoom++.h"
#include <string.h>		// for strcasecmp()


namespace ZOOM {
    record::~record() {
	if (owner == 0) {
	    // Must have been clone()d
	    ZOOM_record_destroy(r);
	}
    }

    // ### Would this operation be better expressed as a copy constructor?
    record *record::clone() const {
	// It's tempting just to replace `r' with a clone, and return
	// `this', but probably more honest to allocate a new C++
	// record object.

	record *rec = new record(0, 0);
	if ((rec->r = ZOOM_record_clone(r)) == 0) {
	    // Presumably an out-of-memory error
	    throw systemError();
	}

	return rec;
    }

    // It's tempting to modify this method just to return either the
    // string that ZOOM_record_get("syntax") gives us, or the VAL_*
    // value from Yaz's OID database, but we'd break the nominal
    // plug-compatibility of competing C++ binding implementations
    // if we did that.
    //
    record::syntax record::recsyn() const {
	const char *syn = ZOOM_record_get(r, "syntax", 0);

	// These string constants are from yaz/util/oid.c
	if (!strcasecmp(syn, "xml"))
	    return XML;
	else if (!strcasecmp(syn, "GRS-1"))
	    return GRS1;
	else if (!strcasecmp(syn, "SUTRS"))
	    return SUTRS;
	else if (!strcasecmp(syn, "USmarc"))
	    return USMARC;
	else if (!strcasecmp(syn, "UKmarc"))
	    return UKMARC;
	else if (!strcasecmp(syn, "XML") ||
		 !strcasecmp(syn, "text-XML") ||
		 !strcasecmp(syn, "application-XML"))
	    return XML;

	return UNKNOWN;
    }

    const char *record::render() const {
	int len;
	return ZOOM_record_get(r, "render", &len);
    }

    const char *record::rawdata() const {
	int len;
	return ZOOM_record_get(r, "raw", &len);
    }
}
