// Z39.50 Record class

#if HAVE_CONFIG_H
#include <config.h>
#endif
#include "zoom.h"
#include <yaz/yaz-util.h>       // for yaz_matchstr()


namespace ZOOM {
    record::syntax::syntax (value rs): val(rs) {}

    record::syntax::operator std::string() const {
        switch (val) {
        case GRS1:   return "grs1";
        case SUTRS:  return "sutrs";
        case USMARC: return "usmarc";
        case UKMARC: return "ukmarc";
        case XML:    return "xml";
        default: break;
        }
        return "unknown";
    }

    bool record::syntax::operator==(const record::syntax &s) const {
        return s.val == val;
    }

    bool record::syntax::operator==(record::syntax::value rs) const {
        return rs == val;
    }

    record::syntax::operator record::syntax::value() const {
        return val;
    }


    record::record(resultSet &rs, size_t i): owner(rs) {
        if ((r = ZOOM_resultset_record(rs._getYazResultSet(), i)) == 0) {
            const char *errmsg; // unused: carries same info as `errcode'
            const char *addinfo;
            int errcode = ZOOM_connection_error(rs._getYazConnection(),
                                                &errmsg, &addinfo);
            throw bib1Exception(errcode, addinfo);
        }

        // Memory management is odd here.  The ZOOM-C record we've
        // just fetched (`r') is owned by the ZOOM-C result-set we
        // fetched it from (`rs.rs'), so the underlying (ZOOM-C)
        // record is _not_ destroyed when this object is destroyed:
        // it's done when the underlying result-set is deleted.
    }

    record::~record() {
        // Nothing to do -- see comment in constructor
    }

    // It's tempting to modify this method just to return either the
    // string that ZOOM_record_get("syntax") gives us, or the VAL_*
    // value from Yaz's OID database, but we'd break the nominal
    // plug-compatibility of competing C++ binding implementations
    // if we did that.
    //
    record::syntax record::recsyn() const {
        const char *syn = ZOOM_record_get(r, "syntax", 0);

        // These string constants are from yaz/util/oid.c
        if (!yaz_matchstr(syn, "xml"))
            return syntax::XML;
        else if (!yaz_matchstr(syn, "GRS-1"))
            return syntax::GRS1;
        else if (!yaz_matchstr(syn, "SUTRS"))
            return syntax::SUTRS;
        else if (!yaz_matchstr(syn, "USmarc"))
            return syntax::USMARC;
        else if (!yaz_matchstr(syn, "UKmarc"))
            return syntax::UKMARC;
        else if (!yaz_matchstr(syn, "XML") ||
                 !yaz_matchstr(syn, "text-XML") ||
                 !yaz_matchstr(syn, "application-XML"))
            return syntax::XML;

        return syntax::UNKNOWN;
    }

    std::string record::render() const {
        int len;
        const char* data = ZOOM_record_get(r, "render", &len);
        return std::string(data, len);
    }

    std::string record::rawdata() const {
        int len;
        const char* data = ZOOM_record_get(r, "raw", &len);
        return std::string(data, len);
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

