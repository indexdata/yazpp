// $Header: /home/cvsroot/yaz++/zoom/zconn.cpp,v 1.1 2002-08-08 13:31:54 mike Exp $

// Z39.50 Connection class

#include "zoom++.h"


namespace ZOOM {
    connection::connection(const char *hostname, int portnum) {
	c = ZOOM_connection_new(hostname, portnum);

	int errcode;
	const char *errmsg;	// unused: carries same info as `errcode'
	const char *addinfo;
	if ((errcode = ZOOM_connection_error(c, &errmsg, &addinfo)) != 0) {
	    throw bib1Error(errcode, addinfo);
	}
    }

    const char *connection::option(const char *key) const {
	return ZOOM_connection_option_get(c, key);
    }

    const char *connection::option(const char *key, const char *val) {
	// ### There may be memory-management issues here.
	const char *old = ZOOM_connection_option_get(c, key);
	ZOOM_connection_option_set(c, key, val);
	return old;
    }

    connection::~connection() {
	ZOOM_connection_destroy(c);
    }
}
