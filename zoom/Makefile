# $Header: /home/cvsroot/yaz++/zoom/Attic/Makefile,v 1.4 2002-08-09 09:03:37 mike Exp $

CPPFLAGS := -Wall -g -ansi
L = libzoom.a
OBJ = $L(zexcept.o) $L(zconn.o) $L(zquery.o) $L(zrs.o) $L(zrec.o)

all: interface.h zclient

zclient: zclient.o $L
	$(CXX) $(CPPFLAGS) -o zclient zclient.o $L -lyaz

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
