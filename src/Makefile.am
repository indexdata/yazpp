## $Id: Makefile.am,v 1.4 2001-04-04 14:02:49 adam Exp $

INCLUDES=$(YAZINC) -I$(srcdir)/../include

lib_LTLIBRARIES = libyaz++.la

libyaz___la_SOURCES=yaz-socket-manager.cpp yaz-pdu-assoc.cpp \
	yaz-z-assoc.cpp yaz-proxy.cpp yaz-z-query.cpp yaz-ir-assoc.cpp \
	yaz-z-server.cpp yaz-pdu-assoc-thread.cpp yaz-z-server-sr.cpp \
	yaz-z-server-ill.cpp zes-ursula.c yaz-z-server-ursula.cpp

libyaz___la_LDFLAGS=-version-info 0:0:0

bin_PROGRAMS = yaz-my-client yaz-my-server yaz-proxy
bin_SCRIPTS = yaz++-config

yaz_my_client_SOURCES=yaz-my-client.cpp

yaz_my_server_SOURCES=yaz-my-server.cpp yaz-marc-sample.cpp

yaz_proxy_SOURCES=yaz-proxy-main.cpp

LDADD=libyaz++.la $(YAZLALIB)

zes-ursula.c $(top_srcdir)/include/yaz/zes-ursula.h: ursula.tcl ursula.asn
	cd $(srcdir); ../../yaz/util/yaz-comp -d $(srcdir)/../../yaz/z39.50/z.tcl -d ursula.tcl -i yaz -I ../include ursula.asn
