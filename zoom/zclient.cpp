// $Header: /home/cvsroot/yaz++/zoom/zclient.cpp,v 1.4 2002-10-09 09:07:10 mike Exp $

// Simple sample client

#include <stdlib.h>		// for atoi()
#include <iostream.h>
#include "zoom.h"


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
    } catch(ZOOM::bib1Exception& err) {
	cerr << argv[0] << ": connect: bib1Exception " <<
	    err.errmsg() << " (" << err.addinfo() << ")\n";
	return 2;
    } catch(ZOOM::exception& err) {
	cerr << argv[0] << ": connect: exception " <<
	    err.errmsg() << "\n";
	return 2;
    }

    conn->option("databaseName", dbname);
    ZOOM::prefixQuery pq(searchSpec);
    ZOOM::resultSet *rs;
    try {
	rs = new ZOOM::resultSet(*conn, pq);
    } catch(ZOOM::bib1Exception err) {
	//fprintf(stderr, "caught exception 0x%lx\n", (long) &err);
	cerr << argv[0] << ": search: " <<
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
