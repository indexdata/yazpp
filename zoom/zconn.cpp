// $Header: /home/cvsroot/yaz++/zoom/zconn.cpp,v 1.6 2003-09-22 12:25:20 mike Exp $

// Z39.50 Connection class

#include "zoom.h"


namespace ZOOM {
    connection::connection(const std::string &hostname, int portnum) {
	const char *line_printer_size_hostname = hostname.c_str();
	//###cerr << "opening " << hostname << ":" << portnum << "\n";
	c = ZOOM_connection_new(line_printer_size_hostname, portnum);
	//###cerr << "opened, got " << c << "\n";

	int errcode;
	const char *errmsg;	// unused: carries same info as `errcode'
	const char *addinfo;
	if ((errcode = ZOOM_connection_error(c, &errmsg, &addinfo)) != 0) {
	    //###cerr << "oops: no connect, errcode=" << errcode << "\n";
	    ZOOM_connection_destroy(c);
	    throw bib1Exception(errcode, addinfo);
	}
    }

    std::string connection::option(const std::string &key) const {
	return ZOOM_connection_option_get(c, key.c_str());
    }

    bool connection::option(const std::string &key, const std::string &val) {
	// No way to tell whether ZOOM_connection_option_set() accepts key
	ZOOM_connection_option_set(c, key.c_str(), val.c_str());
	return true;
    }

    connection::~connection() {
	ZOOM_connection_destroy(c);
    }
}
