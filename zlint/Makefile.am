## $Id: Makefile.am,v 1.5 2004-03-29 20:00:07 adam Exp $

AM_CXXFLAGS = $(YAZINC) -I$(srcdir)/../include $(XSLT_CFLAGS)

zlint_SOURCES=zlint.cpp zlint.h \
 test-init-01.cpp \
 test-init-02.cpp \
 test-init-03.cpp \
 test-init-04.cpp \
 test-init-05.cpp \
 test-init-06.cpp \
 test-init-07.cpp \
 test-init-08.cpp \
 test-search-01.cpp \
 test-scan-01.cpp \
 main.cpp 

noinst_PROGRAMS = zlint

LDADD=../src/libyazcpp.la $(YAZLALIB) $(XSLT_LIBS)
