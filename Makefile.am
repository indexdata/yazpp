AUTOMAKE_OPTIONS = foreign

SUBDIRS = src include

EXTRA_DIST= LICENSE README.txt ChangeLog yaz++-config.in

dist-hook:
	cp -R $(srcdir)/win $(distdir)
