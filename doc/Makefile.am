SUBDIRS = common

XMLFILES = book.xml local.ent

MANFILES = yazpp-config.1

REFFILES = yazpp-config-man.xml

HTMLFILES = index.html

doc_DATA = $(HTMLFILES)

man_MANS = $(MANFILES)

EXTRA_DIST = $(XMLFILES) $(doc_DATA) $(man_MANS) $(REFFILES) \
	entities.ent

$(HTMLFILES): $(XMLFILES)
	rm -f *.html
	$(HTML_COMPILE) book.xml

yazpp-config.1: yazpp-config-man.xml
	$(MAN_COMPILE) $(srcdir)/yazpp-config-man.xml

$(MANFILES): local.ent

yazpp.pdf: $(XMLFILES)
	if test ! -f common/id.png ; then cp $(srcdir)/common/id.png common/; fi
	$(PDF_COMPILE) book.xml
	mv book.pdf yazpp.pdf

clean-data-hook:
	rm -f [0-9]* *.bak

dist-hook:
	if test -f index.html; then d=.; else d="$(srcdir)"; fi; \
	for p in $$d/*.html; do \
		cp $$p $(distdir); \
	done

doc-clean:
	rm -f *.html *.[0-9] *.pdf toc.hhc htmlhelp.hhp local.ent

install-data-hook:
	if test -f index.html; then d=.; else d="$(srcdir)"; fi; \
	for p in $$d/*.html; do \
		$(INSTALL_DATA) $$p $(DESTDIR)$(docdir); \
	done

uninstall-hook:
	rm -r $(DESTDIR)$(docdir)

distclean-local: doc-clean
