## $Id: Makefile.am,v 1.1 2000-10-11 11:58:16 adam Exp $

INCLUDES=$(YAZINC) -I../include

lib_LTLIBRARIES = libyaz++.la

libyaz___la_SOURCES=yaz-socket-manager.cpp yaz-pdu-assoc.cpp \
	yaz-z-assoc.cpp yaz-proxy.cpp yaz-z-query.cpp yaz-ir-assoc.cpp \
	yaz-z-server.cpp

libyaz___la_LDFLAGS=-version-info 0:0:0

bin_PROGRAMS = yaz-client++ yaz-server++ yaz-proxy
bin_SCRIPTS = yaz++-config

yaz_client___SOURCES=yaz-client.cpp

yaz_server___SOURCES=yaz-server.cpp

yaz_proxy_SOURCES=yaz-proxy-main.cpp

LDADD=libyaz++.la $(YAZLALIB) -lpthread
