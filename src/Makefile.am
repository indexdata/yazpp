## $Id: Makefile.am,v 1.23 2004-03-29 22:46:51 adam Exp $

AM_CXXFLAGS = $(YAZINC) -I$(srcdir)/../include

lib_LTLIBRARIES = libyazcpp.la
libyazcpp_la_LDFLAGS=-version-info 1:0:0

libyazcpp_la_SOURCES=yaz-socket-manager.cpp yaz-pdu-assoc.cpp \
	yaz-z-assoc.cpp yaz-z-query.cpp yaz-ir-assoc.cpp \
	yaz-z-server.cpp yaz-pdu-assoc-thread.cpp yaz-z-server-sr.cpp \
	yaz-z-server-ill.cpp yaz-z-server-update.cpp yaz-z-databases.cpp \
	yaz-z-cache.cpp yaz-cql2rpn.cpp

noinst_PROGRAMS = yaz-my-server yaz-my-client
bin_SCRIPTS = yaz++-config

yaz_my_client_SOURCES=yaz-my-client.cpp

yaz_my_server_SOURCES=yaz-my-server.cpp yaz-marc-sample.cpp

LDADD=libyazcpp.la $(YAZLALIB)
