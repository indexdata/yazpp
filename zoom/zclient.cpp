// $Header: /home/cvsroot/yaz++/zoom/zclient.cpp,v 1.1 2002-08-08 13:31:54 mike Exp $

// Trivial sample client

#include <stdlib.h>		// for atoi()
#include <iostream.h>
#include "zoom++.h"

int main(int argc, char **argv)
{
    if (argc != 5) {
	cerr << "Usage: " << argv[0] <<
	    " <host> <port> <db> <@prefix-search>\n";
	return 1;
    }

    const char *hostname = argv[1];
    const int port = atoi(argv[2]);
    const char *dbname = argv[3];
    const char *searchSpec = argv[4];

    ZOOM::connection *conn;
    try {
	conn = new ZOOM::connection(hostname, port);
    } catch(ZOOM::bib1Error err) {
	cerr << argv[0] << ": connect: " <<
	    err.errmsg() << " (" << err.addinfo() << ")\n";
	return 2;
    } catch(ZOOM::error err) {
	cerr << argv[0] << ": connect: " << err.errmsg() << "\n";
	return 2;
    }

    conn->option("databaseName", dbname);
    ZOOM::prefixQuery pq(searchSpec);
    ZOOM::resultSet *rs;
    try {
	rs = new ZOOM::resultSet(*conn, pq);
    } catch(ZOOM::bib1Error err) {
	cerr << argv[0] << ": searchSpec: " <<
	    err.errmsg() << " (" << err.addinfo() << ")\n";
	return 3;
    }

    size_t n = rs->size();
    cout << "found " << n << " records:\n";
    for (size_t i = 0; i < n; i++) {
	const ZOOM::record *rec = rs->getRecord(i);
	cout << "=== record " << i+1 << " (recsyn " << rec->recsyn()
	     << ") ===\n" << rec->render();
    }

    return 0;
}
