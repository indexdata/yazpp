// $Header: /home/cvsroot/yaz++/zoom/zquery.cpp,v 1.2 2002-08-08 16:06:08 mike Exp $

// Z39.50 Query classes

#include "zoom++.h"


namespace ZOOM {
    query::~query() {
	ZOOM_query_destroy(q);
	q = 0;
    }



    prefixQuery::prefixQuery(const char *pqn) {
	q = ZOOM_query_create();
	if (ZOOM_query_prefix(q, pqn) == -1) {
	    ZOOM_query_destroy(q);
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
	ZOOM_query_destroy(q);
	q = 0;
    }



    CCLQuery::CCLQuery(const char *ccl, void *qualset) {
	throw "Oops.  No CCL support in ZOOM-C yet.  Sorry.";
    }

    CCLQuery::~CCLQuery() {
	ZOOM_query_destroy(q);
	q = 0;
    }
}
