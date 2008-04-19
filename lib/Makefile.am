
AM_LDFLAGS=-version-info 1:0:0

lib_LTLIBRARIES = libyaz++.la

if ZOOM
libzoom=../zoom/libzoomcpp.la
else
libzoom=
endif

libyaz___la_LIBADD=$(libzoom) ../src/libyazcpp.la 
libyaz___la_SOURCES=
EXTRA_libyaz___la_SOURCES=x.c

