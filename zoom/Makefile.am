# $Header: /home/cvsroot/yaz++/zoom/Makefile.am,v 1.1 2002-10-08 12:18:47 adam Exp $

AM_CXXFLAGS= $(YAZINC) 

lib_LTLIBRARIES = libzoomcpp.la
LDADD=libzoomcpp.la $(YAZLALIB)

libzoomcpp_la_SOURCES = zexcept.cpp zconn.cpp zquery.cpp zrs.cpp zrec.cpp \
 zoom++.h interface.h
noinst_PROGRAMS = zclient

zclient_SOURCES=zclient.cpp

EXTRA_DIST=master-header Changes README

all: interface.h zclient

test: zclient
	./zclient bagel.indexdata.dk 210 gils '@and mineral epicenter'

zoom++.h: master-header
	rm -f $@
	sed 's/^*	/	/; s/^*/ /' $< > $@
	chmod -w $@

interface.h: master-header
	rm -f $@
	grep -v '^*' $< > $@
	chmod -w $@
