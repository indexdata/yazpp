// Z39.50 Connection class

#if HAVE_CONFIG_H
#include <config.h>
#endif
#include "zoom.h"


namespace ZOOM {
    connection::connection() {
        ZOOM_options o = ZOOM_options_create();
        c = ZOOM_connection_create(o);
    }

    void connection::connect(const std::string &hostname, int portnum) {
        const char *line_printer_size_hostname = hostname.c_str();
        //###cerr << "opening " << hostname << ":" << portnum << "\n";
        ZOOM_connection_connect(c, line_printer_size_hostname, portnum);
        //###cerr << "opened\n";

        int errcode;
        const char *errmsg;     // unused: carries same info as `errcode'
        const char *addinfo;
        if ((errcode = ZOOM_connection_error(c, &errmsg, &addinfo)) != 0) {
            //###cerr << "oops: no connect, errcode=" << errcode << "\n";
            throw bib1Exception(errcode, addinfo);
        }
    }

    connection::connection(const std::string &hostname, int portnum) {
        ZOOM_options o = ZOOM_options_create();
        c = ZOOM_connection_create(o);
        connect(hostname, portnum);
    }

    std::string connection::option(const std::string &key) const {
        const char* val = ZOOM_connection_option_get(c, key.c_str());
        return (val) ? val : std::string();
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
/*
 * Local variables:
 * c-basic-offset: 4
 * c-file-style: "Stroustrup"
 * indent-tabs-mode: nil
 * End:
 * vim: shiftwidth=4 tabstop=8 expandtab
 */

