/* g++ -g -o canonical canonical.cpp -lyaz++ -lyaz -lxml2 */

#include <iostream>
#include <yaz++/zoom.h>

using namespace ZOOM;

int main(int argc, char **argv)
{
    connection conn("z3950.loc.gov", 7090);
    conn.option("databaseName", "Voyager");
    conn.option("preferredRecordSyntax", "USMARC");
    resultSet rs(conn, prefixQuery("@attr 1=7 0253333490"));
    const record rec(rs, 0);
    std::cout << rec.render() << std::endl;
}
