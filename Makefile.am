AUTOMAKE_OPTIONS = foreign

SUBDIRS = src include zlint zoom doc 

aclocaldir=$(datadir)/aclocal
aclocal_DATA = yazpp.m4

EXTRA_DIST= LICENSE TODO yazpp-config.in yazpp.m4 buildconf.sh

dist-hook:
	mkdir $(distdir)/win
	cp $(srcdir)/win/makefile $(distdir)/win/
	mkdir $(distdir)/debian
	cp $(srcdir)/debian/control $(distdir)/debian
	cp $(srcdir)/debian/changelog $(distdir)/debian
	cp $(srcdir)/debian/rules $(distdir)/debian
	cp $(srcdir)/debian/postinst $(distdir)/debian
	cp $(srcdir)/debian/compat $(distdir)/debian
	cp $(srcdir)/debian/*.install $(distdir)/debian
