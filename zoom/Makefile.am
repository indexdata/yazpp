# $Header: /home/cvsroot/yaz++/zoom/Makefile.am,v 1.2 2002-10-08 23:53:55 mike Exp $

AM_CXXFLAGS= $(YAZINC) 

lib_LTLIBRARIES = libzoomcpp.la
LDADD=libzoomcpp.la $(YAZLALIB)

mirk_XXX_SOURCES = zexcept.cpp zconn.cpp zquery.cpp zrs.cpp zrec.cpp
libzoomcpp_la_SOURCES = $(mirk_XXX_SOURCES) zoom++.h interface.h

noinst_PROGRAMS = zclient

zclient_SOURCES=zclient.cpp

EXTRA_DIST=master-header Changes README

all: interface.h zclient

test: zclient
	./zclient bagel.indexdata.dk 210 gils '@and mineral epicenter'

$(mirk_XXX_SOURCES): zoom++.h

zoom++.h: master-header
	rm -f $@
	sed 's/^*	/	/; s/^*/ /' $< > $@
	chmod -w $@

interface.h: master-header
	rm -f $@
	grep -v '^*' $< > $@
	chmod -w $@
