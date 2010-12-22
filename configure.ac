dnl YAZ++ Toolkit Index Data 1998-2010
dnl See the file LICENSE for details.
AC_PREREQ(2.60)
AC_INIT([yazpp],[1.2.3],[adam@indexdata.dk])
AC_CONFIG_SRCDIR(configure.ac)
AC_CONFIG_AUX_DIR([config])
AM_INIT_AUTOMAKE([1.9])

AC_PROG_CC
AC_PROG_CPP
AC_PROG_CXX
AC_HEADER_STDC
AM_PROG_LIBTOOL

YAZ_INIT([threads],[4.0.8])
if test -z "$YAZLIB"; then
	AC_MSG_ERROR([YAZ development libraries missing])
fi
YAZ_DOC
AC_CHECK_HEADERS([unistd.h sys/stat.h sys/time.h sys/types.h fcntl.h])

AC_ARG_ENABLE(zoom,[  --disable-zoom          disable ZOOM (for old C++ compilers)],[enable_zoom=$enableval],[enable_zoom=yes])
AM_CONDITIONAL(ZOOM, test $enable_zoom = "yes")

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
],[sed s%echo_source=yes%echo_source=no%g < yazpp-config >src/yazpp-config && chmod +x yazpp-config src/yazpp-config])


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
