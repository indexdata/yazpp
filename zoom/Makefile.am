# $Header: /home/cvsroot/yaz++/zoom/Makefile.am,v 1.15 2008-02-21 09:43:27 adam Exp $

AM_CXXFLAGS= $(YAZINC) 

if ZOOM
lib_LTLIBRARIES = libzoompp.la
noinst_PROGRAMS = zclient
endif

libzoompp_la_LDFLAGS=-version-info 6:0:0
LDADD=libzoompp.la $(YAZLALIB)

libzoompp_la_LIBADD = $(YAZLALIB)

mirk_sources = zexcept.cpp zconn.cpp zquery.cpp zrs.cpp zrec.cpp
libzoompp_la_SOURCES = $(mirk_sources) interface.h
pkginclude_HEADERS=zoom.h

zclient_SOURCES=zclient.cpp

EXTRA_DIST=master-header README

test: zclient
	./zclient bagel.indexdata.dk 210 gils '@and mineral epicenter'

$(mirk_sources): zoom.h

zoom.h: master-header
	rm -f $@
	sed 's/^*	/	/; s/^*/ /' $< > $@
	chmod -w $@

interface.h: master-header
	rm -f $@
	grep -v '^*' $< | sed 's/YAZ_EXPORT //g' > $@
	chmod -w $@
