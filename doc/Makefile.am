## $Id: Makefile.am,v 1.1 2002-10-08 11:55:57 adam Exp $
docdir=$(datadir)/doc/@PACKAGE@

SUPPORTFILES = \
 yazhtml.dsl.in \
 yazphp.dsl.in \
 yazprint.dsl.in \
 xml.dcl
XMLFILES = \
 installation.xml \
 zoom.xml \
 proxy.xml \
 yaz++.xml.in
 
TOP=yaz++.xml

HTMLFILES = yazpp.html installation.html zoom.html proxy.html

doc_DATA = $(HTMLFILES) yaz++.pdf

EXTRA_DIST = $(SUPPORTFILES) $(XMLFILES) yaz++.pdf

$(HTMLFILES): $(XMLFILES)
	jade -E14 -D $(srcdir) -d yazhtml.dsl -t sgml $(srcdir)/xml.dcl $(TOP)

yaz++.php: $(XMLFILES)
	jade -E14 -D $(srcdir) -d yazphp.dsl -t sgml $(srcdir)/xml.dcl $(TOP)

yaz++.pdf: $(XMLFILES)
	jade -E14 -D $(srcdir) -d yazprint.dsl -t tex $(srcdir)/xml.dcl $(TOP)
	pdfjadetex yaz++.tex
	pdfjadetex yaz++.tex
	pdfjadetex yaz++.tex

dist-hook: yaz++.html
	cp *.html $(distdir)

clean-data-hook:
	rm -f [0-9]* *.bak

