// $Id: zclient.cpp,v 1.5 2002-11-30 22:33:21 mike Exp $

// Simple sample client

#include <stdlib.h>		// for atoi()
#include <iostream.h>
#include "zoom.h"


int main(int argc, char **argv)
{
    if (argc != 5) {
	cerr << "Usage: " <<
	    argv[0] << " <host> <port> <dbname> <@prefix-search>\n";
	return 1;
    }

    using namespace ZOOM;
    try {
	connection conn(argv[1], atoi(argv[2]));
	conn.option("databaseName", argv[3]);
	conn.option("preferredRecordSyntax",
		    record::syntax(record::syntax::XML));
	prefixQuery pq(argv[4]);
	resultSet rs(conn, pq);

	size_t n = rs.size();
	cout << "found " << n << " records:\n";
	for (size_t i = 0; i < n; i++) {
	    const record rec(rs, i);
	    cout << "=== record " << i+1 <<
		" (record-syntax " << (string) rec.recsyn() << ")" <<
		" ===\n" << rec.render();
	}

    } catch(bib1Exception& err) {
	cerr << argv[0] << ": bib1Exception " <<
	    err.errmsg() << " (" << err.addinfo() << ")\n";
	return 2;

    } catch(exception& err) {
	cerr << argv[0] << ": exception " <<
	    err.errmsg() << "\n";
	return 3;
    }

    return 0;
}
