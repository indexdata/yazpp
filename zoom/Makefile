# $Header: /home/cvsroot/yaz++/zoom/Attic/Makefile,v 1.3 2002-08-08 16:14:26 mike Exp $

CCC = g++	# ... until I figure out what the standard
		# Make macro name is for the C++ compiler.
CPPFLAGS := -Wall -g -ansi
L = libzoom.a
OBJ = $L(zexcept.o) $L(zconn.o) $L(zquery.o) $L(zrs.o) $L(zrec.o)

all: interface.h zclient

zclient: zclient.o $L
	@echo CCC = $(CCC)
	$(CCC) $(CPPFLAGS) -o zclient zclient.o $L -lyaz

test: zclient
	./zclient bagel.indexdata.dk 210 gils '@and mineral epicenter'

$L: $(OBJ)
	ranlib $L

$(OBJ): zoom++.h

zclient.o: zoom++.h

zoom++.h: master-header
	rm -f $@
	sed 's/^*	/	/; s/^*/ /' $< > $@
	chmod -w $@

interface.h: master-header
	rm -f $@
	grep -v '^*' $< > $@
	chmod -w $@

clean:
	rm -f zoom++.h interface.h zclient *.[ao] core
