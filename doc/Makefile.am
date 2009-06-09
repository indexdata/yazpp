SUBDIRS = common

XMLFILES = \
 introduction.xml \
 installation.xml \
 zoom.xml \
 api.xml \
 license.xml \
 yazpp.xml

TOP = $(srcdir)/yazpp.xml

MANFILES = yazpp-config.8

REFFILES = yazpp-config-man.xml

HTMLFILES = index.html

doc_DATA = $(HTMLFILES) 

man_MANS = $(MANFILES)

EXTRA_DIST = $(XMLFILES) $(doc_DATA) $(man_MANS) $(REFFILES) \
	entities.ent local.ent.in

$(HTMLFILES): $(XMLFILES)
	rm -f *.html
	$(HTML_COMPILE) $(TOP)

index.tkl: $(XMLFILES)
	$(TKL_COMPILE) $(TOP)

yazpp-config.8: yazpp-config-man.xml
	$(MAN_COMPILE) $(srcdir)/yazpp-config-man.xml

yazpp.pdf: $(XMLFILES)
	if test ! -f common/id.png ; then cp $(srcdir)/common/id.png common/; fi
	$(PDF_COMPILE) $(TOP)

clean-data-hook:
	rm -f [0-9]* *.bak

dist-hook:
	if test -f index.html; then d=.; else d="$(srcdir)"; fi; \
	for p in $$d/*.html; do \
		cp $$p $(distdir); \
	done

doc-clean:
	rm -f *.html *.[0-9] *.pdf *.tkl

install-data-hook:
	if test -f index.html; then d=.; else d="$(srcdir)"; fi; \
	for p in $$d/*.html; do \
		$(docDATA_INSTALL) $$p $(DESTDIR)$(docdir); \
	done

uninstall-hook:
	rm -r $(DESTDIR)$(docdir)
