AUTOMAKE_OPTIONS = foreign

SUBDIRS = src include zlint zoom doc 

ACLOCAL_AMFLAGS = -I m4

aclocaldir=$(datadir)/aclocal
aclocal_DATA = m4/yazpp.m4

SPEC_FILE=$(PACKAGE).spec
EXTRA_DIST= $(SPEC_FILE) IDMETA README LICENSE NEWS m4/id-config.sh \
	yazpp-config.in m4/yaz.m4 m4/yazpp.m4 buildconf.sh Doxyfile.in

dist-hook:
	if test -x /usr/bin/git -a -d .git; then git log >ChangeLog ; cp ChangeLog $(distdir); fi
	mkdir $(distdir)/win
	cp $(srcdir)/win/makefile $(distdir)/win/


# doxygen target
.PHONY:dox
dox:
	doxygen

