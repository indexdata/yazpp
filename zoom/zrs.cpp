// $Header: /home/cvsroot/yaz++/zoom/zrs.cpp,v 1.1 2002-08-08 13:31:54 mike Exp $

// Z39.50 Result Set class

#include "zoom++.h"


namespace ZOOM {
    resultSet::resultSet(connection &c, const query &q) : owner(c) {
	ZOOM_connection yazc = c._getYazConnection();
	rs = ZOOM_connection_search(yazc, q._getYazQuery());
	int errcode;
	const char *errmsg;	// unused: carries same info as `errcode'
	const char *addinfo;

	if ((errcode = ZOOM_connection_error(yazc, &errmsg, &addinfo)) != 0) {
	    throw bib1Error(errcode, addinfo);
	}
    }

    resultSet::~resultSet() {
	ZOOM_resultset_destroy(rs);
    }

    const char *resultSet::option(const char *key) const {
	return ZOOM_resultset_option_get(rs, key);
    }

    const char *resultSet::option(const char *key, const char *val) {
	// ### There may be memory-management issues here.
	const char *old = ZOOM_resultset_option_get(rs, key);
	ZOOM_resultset_option_set(rs, key, val);
	return old;
    }

    size_t resultSet::size() const {
	return ZOOM_resultset_size(rs);
    }

    const record *resultSet::getRecord(size_t i) const {
	ZOOM_record rec;
	if ((rec = ZOOM_resultset_record(rs, i)) == 0) {
	    const char *errmsg;	// unused: carries same info as `errcode'
	    const char *addinfo;
	    int errcode = ZOOM_connection_error(owner._getYazConnection(),
						&errmsg, &addinfo);
	    throw bib1Error(errcode, addinfo);
	}

	// Memory management is odd here.  The ZOOM-C record we've
	// just fetched (`rec') is owned by the ZOOM-C result-set we
	// fetched it from (`rs'), so all we need to allocate is a
	// ZOOM-C++ wrapper for it, which is destroyed at the
	// appropriate time -- but the underlying (ZOOM-C) record is
	// _not_ destroyed at that time, because it's done when the
	// underlying result-set is deleted.
	return new record(this, rec);
    }
}
