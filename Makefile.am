AUTOMAKE_OPTIONS = foreign

SUBDIRS = src include zoom lib doc

EXTRA_DIST= LICENSE README.txt ChangeLog yaz++-config.in

dist-hook:
	cp -R $(srcdir)/win $(distdir)
