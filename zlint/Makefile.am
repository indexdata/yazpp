## $Id: Makefile.am,v 1.2 2004-02-24 20:55:57 adam Exp $

AM_CXXFLAGS = $(YAZINC) -I$(srcdir)/../include $(XSLT_CFLAGS)

zlint_SOURCES=zlint.cpp

noinst_PROGRAMS = zlint

LDADD=../src/libyazcpp.la $(YAZLALIB) $(XSLT_LIBS)
