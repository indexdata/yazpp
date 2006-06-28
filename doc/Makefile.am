## $Id: Makefile.am,v 1.22 2006-06-28 21:43:45 adam Exp $
docdir=$(datadir)/doc/@PACKAGE@

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

HTMLFILES = \
 api.html \
 implementations.html \
 index.html \
 installation.html \
 introduction.html \
 license.html \
 windows.html \
 zoom-connection.html \
 zoom-exception.html \
 zoom-query.html \
 zoom-record.html \
 zoom-resultset.html \
 zoom.html

doc_DATA = $(HTMLFILES) 

man_MANS = $(MANFILES)

EXTRA_DIST = $(XMLFILES) $(doc_DATA) $(man_MANS) $(REFFILES) \
	entities.ent local.ent.in

$(HTMLFILES): $(XMLFILES)
	$(HTML_COMPILE) $(TOP)

index.tkl: $(XMLFILES) common/tkl.xsl
	$(TKL_COMPILE) $(TOP)

yazpp-config.8: yazpp-config-man.xml
	$(MAN_COMPILE) $(srcdir)/yazpp-config-man.xml

yazpp.pdf: $(XMLFILES)
	if test ! -f common/id.png ; then cp $(srcdir)/common/id.png common/; fi
	jade -E14 -D $(srcdir) -d common/print.dsl -t tex $(srcdir)/common/xml.dcl $(TOP)
	pdfjadetex yazpp.tex >pdfjadetex.log 2>&1
	pdfjadetex yazpp.tex >pdfjadetex.log 2>&1
	pdfjadetex yazpp.tex >pdfjadetex.log 2>&1


clean-data-hook:
	rm -f [0-9]* *.bak

dist-hook:
	if test -f index.html; then for f in *.html; do \
		found=0; \
		b=`basename $$f`; \
		for h in $(HTMLFILES); do \
			if test "$$h" = "$$b"; then \
				found=1; \
			fi \
		done; \
		if test "$$found" = "0"; then \
			echo "$$h not found in HTMLFILES"; \
			exit 1; \
		fi \
	done; fi

