## $Id: Makefile.am,v 1.14 2003-07-18 13:27:20 adam Exp $

AM_CXXFLAGS = $(YAZINC) -I$(srcdir)/../include

noinst_LTLIBRARIES = libyazcpp.la

libyazcpp_la_SOURCES=yaz-socket-manager.cpp yaz-pdu-assoc.cpp \
	yaz-z-assoc.cpp yaz-proxy.cpp yaz-z-query.cpp yaz-ir-assoc.cpp \
	yaz-z-server.cpp yaz-pdu-assoc-thread.cpp yaz-z-server-sr.cpp \
	yaz-z-server-ill.cpp yaz-z-server-update.cpp yaz-z-databases.cpp \
	yaz-z-cache.cpp

bin_PROGRAMS = yaz-proxy
noinst_PROGRAMS = yaz-my-server yaz-my-client
bin_SCRIPTS = yaz++-config

yaz_my_client_SOURCES=yaz-my-client.cpp

yaz_my_server_SOURCES=yaz-my-server.cpp yaz-marc-sample.cpp

yaz_proxy_SOURCES=yaz-proxy-main.cpp

LDADD=libyazcpp.la $(YAZLALIB)
