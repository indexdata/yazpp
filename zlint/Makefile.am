## $Id: Makefile.am,v 1.1 2004-02-18 22:11:41 adam Exp $

AM_CXXFLAGS = $(YAZINC) -I$(srcdir)/../include $(XSLT_CFLAGS)

zlint_SOURCES=zlint.cpp

bin_PROGRAMS = zlint

LDADD=../src/libyazcpp.la $(YAZLALIB) $(XSLT_LIBS)
