# $Header: /home/cvsroot/yaz++/zoom/Makefile.am,v 1.9 2003-06-10 07:18:50 adam Exp $

AM_CXXFLAGS= $(YAZINC) 

if ZOOM
noinst_LTLIBRARIES = libzoomcpp.la
noinst_PROGRAMS = zclient
endif

LDADD=libzoomcpp.la $(YAZLALIB)

mirk_sources = zexcept.cpp zconn.cpp zquery.cpp zrs.cpp zrec.cpp
libzoomcpp_la_SOURCES = $(mirk_sources) interface.h
pkginclude_HEADERS=zoom.h

zclient_SOURCES=zclient.cpp

EXTRA_DIST=master-header Changes README

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
