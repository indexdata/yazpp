// $Header: /home/cvsroot/yaz++/zoom/zrs.cpp,v 1.4 2002-11-30 22:33:21 mike Exp $

// Z39.50 Result Set class

#include "zoom.h"


namespace ZOOM {
    resultSet::resultSet(connection &c, const query &q) : owner(c) {
	ZOOM_connection yazc = c._getYazConnection();
	rs = ZOOM_connection_search(yazc, q._getYazQuery());
	int errcode;
	const char *errmsg;	// unused: carries same info as `errcode'
	const char *addinfo;

	if ((errcode = ZOOM_connection_error(yazc, &errmsg, &addinfo)) != 0) {
	    throw bib1Exception(errcode, addinfo);
	}
    }

    resultSet::~resultSet() {
	ZOOM_resultset_destroy(rs);
    }

    string resultSet::option(const string &key) const {
	return ZOOM_resultset_option_get(rs, key.c_str());
    }

    bool resultSet::option(const string &key, const string &val) {
      ZOOM_resultset_option_set(rs, key.c_str(), val.c_str());
	return true;
    }

    size_t resultSet::size() const {
	return ZOOM_resultset_size(rs);
    }
}
