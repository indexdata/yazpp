AUTOMAKE_OPTIONS = foreign

SUBDIRS = src include zlint zoom etc doc 

aclocaldir=$(datadir)/aclocal
aclocal_DATA = yazpp.m4

EXTRA_DIST= LICENSE README.txt TODO ChangeLog yaz++-config.in yazpp.m4

dist-hook:
	cp -R $(srcdir)/win $(distdir)
	mkdir $(distdir)/debian
	cp $(srcdir)/debian/control $(distdir)/debian
	cp $(srcdir)/debian/changelog $(distdir)/debian
	cp $(srcdir)/debian/rules $(distdir)/debian
	cp $(srcdir)/debian/postinst $(distdir)/debian
	cp $(srcdir)/debian/compat $(distdir)/debian
	cp $(srcdir)/debian/*.install $(distdir)/debian
