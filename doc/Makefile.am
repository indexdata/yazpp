## $Id: Makefile.am,v 1.12 2004-03-30 22:26:16 adam Exp $
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
HTMLFILES = \
 api.html \
 implementations.html \
 installation.html \
 license.html \
 other-optimizations.html \
 otherinfo-encoding.html \
 proxy-config-file.html \
 proxy-keepalive.html \
 proxy-target.html \
 proxy-usage.html \
 proxy.html \
 query-cache.html \
 query-validation.html \
 record-cache.html \
 record-validation.html \
 windows.html \
 yaz-proxy.html \
 yazpp.html \
 zoom-connection.html \
 zoom-exception.html \
 zoom-query.html \
 zoom-record.html \
 zoom-resultset.html \
 zoom.html

doc_DATA = $(HTMLFILES) yaz++.pdf yaz.css

man_MANS = $(MANFILES)

EXTRA_DIST = $(SUPPORTFILES) $(XMLFILES) $(doc_DATA) $(man_MANS)

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

clean-data-hook:
	rm -f [0-9]* *.bak

