# $Header: /home/cvsroot/yaz++/zoom/Makefile.am,v 1.12 2006-03-27 15:10:42 adam Exp $

AM_CXXFLAGS= $(YAZINC) 

if ZOOM
lib_LTLIBRARIES = libzoomcpp.la
noinst_PROGRAMS = zclient
endif

libzoomcpp_la_LDFLAGS=-version-info 2:0:0
LDADD=libzoomcpp.la $(YAZLALIB)

mirk_sources = zexcept.cpp zconn.cpp zquery.cpp zrs.cpp zrec.cpp
libzoomcpp_la_SOURCES = $(mirk_sources) interface.h
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
