AUTOMAKE_OPTIONS = foreign

SUBDIRS = src include zoom lib etc doc 

EXTRA_DIST= LICENSE README.txt TODO ChangeLog yaz++-config.in

dist-hook:
	cp -R $(srcdir)/win $(distdir)
