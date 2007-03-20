dnl YAZ++ Toolkit, Index Data 1994-2006
dnl See the file LICENSE for details.
dnl $Id: configure.ac,v 1.15 2007-03-20 16:21:58 adam Exp $
AC_PREREQ(2.59)
AC_INIT([yazpp],[1.0.3],[adam@indexdata.dk])
AC_CONFIG_SRCDIR(configure.ac)
AC_CONFIG_AUX_DIR([config])
AM_INIT_AUTOMAKE([1.8])

AC_PROG_CC
AC_PROG_CPP
AC_PROG_CXX
AC_HEADER_STDC
AM_PROG_LIBTOOL

YAZ_INIT(threads,2.1.50)
if test -z "$YAZLIB"; then
	AC_MSG_ERROR([YAZ development libraries missing])
fi
YAZ_DOC
AC_CHECK_HEADERS(unistd.h sys/stat.h sys/time.h sys/types.h)

AC_ARG_ENABLE(zoom,[  --disable-zoom          disable ZOOM (for old C++ compilers)],[enable_zoom=$enableval],[enable_zoom=yes])
AM_CONDITIONAL(ZOOM, test $enable_zoom = "yes")

AC_SUBST(YAZPP_SRC_ROOT)
AC_SUBST(YAZPP_BUILD_ROOT)
YAZPP_SRC_ROOT=`cd ${srcdir}; pwd`
YAZPP_BUILD_ROOT=`pwd`

AC_OUTPUT([
	Doxyfile
	Makefile
	src/Makefile
	include/Makefile
	include/yazpp/Makefile
	zlint/Makefile
	yazpp-config
	zoom/Makefile
	doc/Makefile
	doc/local.ent
	doc/common/Makefile
	doc/common/print.dsl
],[sed s%yaz_echo_source=yes%yaz_echo_source=no%g < yazpp-config >src/yazpp-config && chmod +x yazpp-config src/yazpp-config])


echo \
"------------------------------------------------------------------------

  YAZPP Package:              ${PACKAGE}
  YAZPP Version:              ${VERSION}
  Source code location:       ${srcdir}
  CPP Preprocessor:           ${CPP}
  CPP Preprocessor flags:     ${CPPFLAGS}
  CXX Compiler:               ${CXX}
  CXX Compiler flags:         ${CXXFLAGS}
  Linker flags:               ${LDFLAGS}
  Linked libs:                ${LIBS}
  Host System Type:           ${host}
  Install path:               ${prefix}
  Automake:                   ${AUTOMAKE}
  Archiver:                   ${AR}
  Ranlib:                     ${RANLIB}
  YAZ Version:                ${YAZVERSION}
  YAZ Include:                ${YAZINC}
  YAZ La Lib:                 ${YAZLALIB}
  YAZ Lib:                    ${YAZLIB}
  Bugreport:                  ${PACKAGE_BUGREPORT}

------------------------------------------------------------------------"
