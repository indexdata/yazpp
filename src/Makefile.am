## $Id: Makefile.am,v 1.22 2004-02-11 10:01:54 adam Exp $

AM_CXXFLAGS = $(YAZINC) -I$(srcdir)/../include $(XSLT_CFLAGS)

lib_LTLIBRARIES = libyazcpp.la
libyazcpp_la_LDFLAGS=-version-info 1:0:0

libyazcpp_la_SOURCES=yaz-socket-manager.cpp yaz-pdu-assoc.cpp \
	yaz-z-assoc.cpp yaz-proxy.cpp yaz-z-query.cpp yaz-ir-assoc.cpp \
	yaz-z-server.cpp yaz-pdu-assoc-thread.cpp yaz-z-server-sr.cpp \
	yaz-z-server-ill.cpp yaz-z-server-update.cpp yaz-z-databases.cpp \
	yaz-z-cache.cpp yaz-proxy-config.cpp yaz-bw.cpp yaz-cql2rpn.cpp

bin_PROGRAMS = yaz-proxy
noinst_PROGRAMS = yaz-my-server yaz-my-client
bin_SCRIPTS = yaz++-config

yaz_my_client_SOURCES=yaz-my-client.cpp

yaz_my_server_SOURCES=yaz-my-server.cpp yaz-marc-sample.cpp

yaz_proxy_SOURCES=yaz-proxy-main.cpp

LDADD=libyazcpp.la $(YAZLALIB) $(XSLT_LIBS)
