## $Id: Makefile.am,v 1.7 2002-11-08 13:38:45 adam Exp $
docdir=$(datadir)/doc/@PACKAGE@

SUPPORTFILES = \
 yazhtml.dsl.in \
 yazphp.dsl.in \
 yazprint.dsl.in \
 tkl.xsl.in \
 xml.dcl
XMLFILES = \
 installation.xml \
 zoom.xml \
 proxy.xml \
 api.xml \
 yaz-proxy-ref.xml \
 yaz-proxy-man.sgml \
 license.xml \
 yaz++.xml.in
 
TOP=yaz++.xml
MANFILES=yaz-proxy.8
HTMLFILES = api.html installation.html license.html otherinfo-encoding.html \
 proxy-cache.html proxy.html proxy-keepalive.html proxy-optimizations.html \
 proxy-target.html proxy-usage.html yazpp.html yaz-proxy.html \
  zoom-connection.html zoom-exception.html zoom.html zoom-query.html \
 zoom-record.html zoom-resultset.html windows.html implementations.html

doc_DATA = $(HTMLFILES) yaz++.pdf

man_MANS = $(MANFILES)

EXTRA_DIST = $(SUPPORTFILES) $(XMLFILES) yaz++.pdf yaz-proxy.8

$(HTMLFILES): $(XMLFILES)
	jade -E14 -D $(srcdir) -d yazhtml.dsl -t sgml $(srcdir)/xml.dcl $(TOP)

yaz-proxy.8: yaz-proxy-man.sgml yaz-proxy-ref.xml
	docbook2man $(srcdir)/yaz-proxy-man.sgml

yazpp.php: $(XMLFILES)
	jade -E14 -D $(srcdir) -d yazphp.dsl -t sgml $(srcdir)/xml.dcl $(TOP)

yaz++.pdf: $(XMLFILES)
	jade -E14 -D $(srcdir) -d yazprint.dsl -t tex $(srcdir)/xml.dcl $(TOP)
	pdfjadetex yaz++.tex
	pdfjadetex yaz++.tex
	pdfjadetex yaz++.tex

index.tkl: $(XMLFILES) tkl.xsl
	xsltproc tkl.xsl $(TOP)

dist-hook: zoom.html
	cp *.html $(distdir)

clean-data-hook:
	rm -f [0-9]* *.bak

