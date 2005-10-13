## $Id: Makefile.am,v 1.29 2005-10-13 09:56:38 adam Exp $

check_PROGRAMS = tstquery
noinst_PROGRAMS = yaz-my-server yaz-my-client
bin_SCRIPTS = yaz++-config

TESTS = $(check_PROGRAMS)

AM_CXXFLAGS = -I$(srcdir)/../include $(YAZINC)

lib_LTLIBRARIES = libyazcpp.la
libyazcpp_la_LDFLAGS=-version-info 1:0:0

DISTCLEANFILES = yaz++-config

libyazcpp_la_SOURCES=socket-observer.cpp pdu-observer.cpp query.cpp \
	z-server.cpp \
	yaz-socket-manager.cpp yaz-pdu-assoc.cpp \
	yaz-z-assoc.cpp yaz-z-query.cpp yaz-ir-assoc.cpp \
	yaz-z-server.cpp yaz-pdu-assoc-thread.cpp yaz-z-server-sr.cpp \
	yaz-z-server-ill.cpp yaz-z-server-update.cpp yaz-z-databases.cpp \
	yaz-z-cache.cpp yaz-cql2rpn.cpp gdu.cpp gduqueue.cpp


yaz_my_client_SOURCES=yaz-my-client.cpp

yaz_my_server_SOURCES=yaz-my-server.cpp yaz-marc-sample.cpp

tstquery_SOURCES=tstquery.cpp

LDADD=libyazcpp.la $(YAZLALIB)
