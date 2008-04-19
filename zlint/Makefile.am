
AM_CXXFLAGS = $(YAZINC) -I$(srcdir)/../include

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

LDADD=../src/libyazpp.la $(YAZLALIB)
