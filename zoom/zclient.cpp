// Simple sample client

#include <stdlib.h>             // for atoi()
#include <iostream>
#include "zoom.h"


int main(int argc, char **argv)
{
    if (argc != 5) {
        std::cerr << "Usage: " <<
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
        std::cout << "found " << n << " records:\n";
        for (size_t i = 0; i < n; i++) {
            const record rec(rs, i);
            std::cout << "=== record " << i+1 <<
                " (record-syntax " << (std::string) rec.recsyn() << ")" <<
                " ===\n" << rec.render();
        }

    } catch(bib1Exception& err) {
        std::cerr << argv[0] << ": bib1Exception " <<
            err.errmsg() << " (" << err.addinfo() << ")\n";
        return 2;

    } catch(ZOOM::exception& err) {
        std::cerr << argv[0] << ": exception " <<
            err.errmsg() << "\n";
        return 3;
    }

    return 0;
}
/*
 * Local variables:
 * c-basic-offset: 4
 * c-file-style: "Stroustrup"
 * indent-tabs-mode: nil
 * End:
 * vim: shiftwidth=4 tabstop=8 expandtab
 */

