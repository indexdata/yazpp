## $Id: Makefile.am,v 1.15 2004-04-22 18:44:33 adam Exp $
docdir=$(datadir)/doc/@PACKAGE@

SUPPORTFILES = \
 yazhtml.dsl.in \
 yazphp.dsl.in \
 yazprint.dsl.in \
 tkl.xsl.in \
 xml.dcl
XMLFILES = \
 introduction.xml \
 installation.xml \
 zoom.xml \
 api.xml \
 license.xml \
 yaz++.xml.in
 
TOP=yaz++.xml
MANFILES=
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

doc_DATA = $(HTMLFILES) yaz++.pdf id.png yaz.css

man_MANS = $(MANFILES)

EXTRA_DIST = $(SUPPORTFILES) $(XMLFILES) $(doc_DATA) $(man_MANS)

$(HTMLFILES): $(XMLFILES)
	jade -E14 -D $(srcdir) -d yazhtml.dsl -t sgml $(srcdir)/xml.dcl $(TOP)

yazpp.php: $(XMLFILES)
	jade -E14 -D $(srcdir) -d yazphp.dsl -t sgml $(srcdir)/xml.dcl $(TOP)

yaz++.pdf: $(XMLFILES)
	if test ! -f id.png ; then ln -s $(srcdir)/id.png .; fi
	jade -E14 -D $(srcdir) -d yazprint.dsl -t tex $(srcdir)/xml.dcl $(TOP)
	pdfjadetex yaz++.tex >pdfjadetex.log 2>&1
	pdfjadetex yaz++.tex >pdfjadetex.log 2>&1
	pdfjadetex yaz++.tex >pdfjadetex.log 2>&1

index.tkl: $(XMLFILES) tkl.xsl
	xsltproc tkl.xsl $(TOP)

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

