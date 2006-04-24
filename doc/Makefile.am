## $Id: Makefile.am,v 1.19 2006-04-24 08:55:28 marc Exp $
docdir=$(datadir)/doc/@PACKAGE@

SUBDIRS = common

XMLFILES = \
 introduction.xml \
 installation.xml \
 zoom.xml \
 api.xml \
 license.xml \
 yazpp.xml.in

TOP=yazpp.xml

MANFILES = yazpp-config.8

REFFILES = yazpp-config-man.xml

HTMLFILES = \
 api.html \
 implementations.html \
 installation.html \
 introduction.html \
 license.html \
 windows.html \
 yazpp.html \
 zoom-connection.html \
 zoom-exception.html \
 zoom-query.html \
 zoom-record.html \
 zoom-resultset.html \
 zoom.html

doc_DATA = $(HTMLFILES) yazpp.pdf 

man_MANS = $(MANFILES)

EXTRA_DIST = $(XMLFILES) $(doc_DATA) $(man_MANS)  $(REFFILES) 

$(HTMLFILES): $(XMLFILES)
	jade -E14 -D $(srcdir) -d common/html.dsl -t sgml $(srcdir)/common/xml.dcl $(TOP)

yazpp.pdf: $(XMLFILES)
	if test ! -f common/id.png ; then cp $(srcdir)/common/id.png common/; fi
	jade -E14 -D $(srcdir) -d common/print.dsl -t tex $(srcdir)/common/xml.dcl $(TOP)
	pdfjadetex yazpp.tex >pdfjadetex.log 2>&1
	pdfjadetex yazpp.tex >pdfjadetex.log 2>&1
	pdfjadetex yazpp.tex >pdfjadetex.log 2>&1

index.tkl: $(XMLFILES) common/tkl.xsl
	xsltproc common/tkl.xsl $(TOP)


yazpp-config.8: yazpp-config-man.xml
	docbook2man $(srcdir)/yazpp-config-man.xml


clean-data-hook:
	rm -f [0-9]* *.bak

dist-hook:
	for f in $(srcdir)/*.html; do \
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
	done

