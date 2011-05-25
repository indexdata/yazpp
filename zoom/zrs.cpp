// Z39.50 Result Set class

#if HAVE_CONFIG_H
#include <config.h>
#endif
#include "zoom.h"


namespace ZOOM {
    resultSet::resultSet(connection &c, const query &q) : owner(c) {
        ZOOM_connection yazc = c._getYazConnection();
        rs = ZOOM_connection_search(yazc, q._getYazQuery());
        int errcode;
        const char *errmsg;     // unused: carries same info as `errcode'
        const char *addinfo;

        if ((errcode = ZOOM_connection_error(yazc, &errmsg, &addinfo)) != 0) {
            ZOOM_resultset_destroy(rs);
            throw bib1Exception(errcode, addinfo);
        }
    }

    resultSet::~resultSet() {
        ZOOM_resultset_destroy(rs);
    }

    std::string resultSet::option(const std::string &key) const {
        return ZOOM_resultset_option_get(rs, key.c_str());
    }

    bool resultSet::option(const std::string &key, const std::string &val) {
      ZOOM_resultset_option_set(rs, key.c_str(), val.c_str());
        return true;
    }

    size_t resultSet::size() const {
        return ZOOM_resultset_size(rs);
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

