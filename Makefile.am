AUTOMAKE_OPTIONS = foreign

SUBDIRS = src include zlint zoom doc 

ACLOCAL_AMFLAGS = -I m4

aclocaldir=$(datadir)/aclocal
aclocal_DATA = m4/yazpp.m4

EXTRA_DIST= LICENSE TODO yazpp-config.in m4/yaz.m4 m4/yazpp.m4 buildconf.sh

dist-hook:
	if test -x /usr/bin/cvs2cl -a -d CVS; then cvs2cl ; cp ChangeLog $(distdir); fi
	mkdir $(distdir)/win
	cp $(srcdir)/win/makefile $(distdir)/win/
	mkdir $(distdir)/debian
	cp $(srcdir)/debian/control $(distdir)/debian
	cp $(srcdir)/debian/changelog $(distdir)/debian
	cp $(srcdir)/debian/rules $(distdir)/debian
	cp $(srcdir)/debian/*.install $(distdir)/debian


# doxygen target
.PHONY:dox
dox:
	doxygen

.PHONY:firefox
firefox:
	firefox  -new-window file://`pwd`/dox/html/index.html &

.PHONY:konqueror
konqueror:
	konqueror file://`pwd`/dox/html/index.html &

.PHONY:galeon
galeon:
	galeon file://`pwd`/dox/html/index.html &

.PHONY:epiphany
epiphany:
	epiphany --new-window file://`pwd`/dox/html/index.html &
