## $Id: Makefile.am,v 1.3 2004-02-25 08:17:52 adam Exp $

AM_CXXFLAGS = $(YAZINC) -I$(srcdir)/../include $(XSLT_CFLAGS)

zlint_SOURCES=zlint.cpp

bin_PROGRAMS = zlint

LDADD=../src/libyazcpp.la $(YAZLALIB) $(XSLT_LIBS)
