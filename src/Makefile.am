## $Id: Makefile.am,v 1.3 2001-03-27 14:47:45 adam Exp $

INCLUDES=$(YAZINC) -I$(srcdir)/../include

lib_LTLIBRARIES = libyaz++.la

libyaz___la_SOURCES=yaz-socket-manager.cpp yaz-pdu-assoc.cpp \
	yaz-z-assoc.cpp yaz-proxy.cpp yaz-z-query.cpp yaz-ir-assoc.cpp \
	yaz-z-server.cpp yaz-pdu-assoc-thread.cpp yaz-z-server-sr.cpp \
	yaz-z-server-ill.cpp

libyaz___la_LDFLAGS=-version-info 0:0:0

bin_PROGRAMS = yaz-my-client yaz-my-server yaz-proxy
bin_SCRIPTS = yaz++-config

yaz_my_client_SOURCES=yaz-my-client.cpp

yaz_my_server_SOURCES=yaz-my-server.cpp yaz-marc-sample.cpp

yaz_proxy_SOURCES=yaz-proxy-main.cpp

LDADD=libyaz++.la $(YAZLALIB)
