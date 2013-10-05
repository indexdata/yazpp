
check_PROGRAMS = test_query test_gdu
noinst_PROGRAMS = yaz-my-server yaz-my-client
bin_SCRIPTS = yazpp-config

TESTS = $(check_PROGRAMS)

AM_CXXFLAGS = -I$(srcdir)/../include $(YAZINC)

lib_LTLIBRARIES = libyazpp.la
libyazpp_la_LDFLAGS=-version-info 6:0:0

DISTCLEANFILES = yazpp-config

libyazpp_la_SOURCES=socket-observer.cpp pdu-observer.cpp query.cpp \
	z-server.cpp \
	yaz-socket-manager.cpp yaz-pdu-assoc.cpp \
	yaz-z-assoc.cpp yaz-z-query.cpp yaz-ir-assoc.cpp \
	yaz-z-server.cpp yaz-pdu-assoc-thread.cpp yaz-z-server-sr.cpp \
	yaz-z-server-ill.cpp yaz-z-server-update.cpp yaz-z-databases.cpp \
	yaz-z-cache.cpp yaz-cql2rpn.cpp gdu.cpp gduqueue.cpp \
	timestat.cpp limit-connect.cpp

libyazpp_la_LIBADD = $(YAZLALIB)

yaz_my_client_SOURCES=yaz-my-client.cpp

yaz_my_server_SOURCES=yaz-my-server.cpp yaz-marc-sample.cpp

test_query_SOURCES=test_query.cpp
test_gdu_SOURCES=test_gdu.cpp

LDADD=libyazpp.la $(YAZLALIB)
