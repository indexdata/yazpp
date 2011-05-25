// Z39.50 Query classes

#if HAVE_CONFIG_H
#include <config.h>
#endif
#include "zoom.h"


namespace ZOOM {
    query::query() : q(ZOOM_query_create()) {
    }
    query::~query() {
        ZOOM_query_destroy(q);
    }

    prefixQuery::prefixQuery(const std::string &pqn) {
        if (ZOOM_query_prefix(q, pqn.c_str()) == -1) {
            throw queryException(queryException::PREFIX, pqn);
        }
    }

    // The binding specification says we have to have destructors for
    // the query subclasses, so in they go -- even though they don't
    // actually _do_ anything that inheriting the base query type's
    // destructor wouldn't do.  It's an irritant of C++ that the
    // declaration of a subclass has to express explicitly the
    // implementation detail of whether destruction is implemented
    // by a specific destructor or by inheritance.  Oh well.
    //
    // ### Not sure whether I need to do nothing at all, and the
    // superclass destructor gets called anyway (I think that only
    // works when you _don't_ define a destructor so that the default
    // one pertains) or whether I need to duplicate the functionality
    // of that destructor.  Let's play safe by assuming the latter and
    // zeroing what we free so that we get bitten if we're wrong.
    //
    prefixQuery::~prefixQuery() {
    }



    CCLQuery::CCLQuery(const std::string &, void *) {
        throw "Oops.  No CCL support in ZOOM-C yet.  Sorry.";
    }

    CCLQuery::~CCLQuery() {
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

