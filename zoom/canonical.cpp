/* g++ -g -o canonical canonical.cpp -lyaz++ -lyaz -lxml2 */

#include <iostream>
#include <yaz++/zoom.h>

using namespace ZOOM;

int main(int argc, char **argv)
{
    connection conn("lx2.loc.gov", 210);
    conn.option("databaseName", "LCDB");
    conn.option("preferredRecordSyntax", "USMARC");
    resultSet rs(conn, prefixQuery("@attr 1=7 0253333490"));
    const record rec(rs, 0);
    std::cout << rec.render() << std::endl;
}
/*
 * Local variables:
 * c-basic-offset: 4
 * c-file-style: "Stroustrup"
 * indent-tabs-mode: nil
 * End:
 * vim: shiftwidth=4 tabstop=8 expandtab
 */

