#!/bin/sh
# $Id: buildconf.sh,v 1.6 2001-03-26 14:43:49 adam Exp $
dir=`aclocal --print-ac-dir`
if [ -f $dir/yaz.m4 ]; then
	aclocal || exit 1
else
	aclocal -I . || exit 1
fi
libtoolize --force >/dev/null 2>&1 || exit 2
automake -a >/dev/null 2>&1 || exit 3
autoconf || exit 4
if [ -f config.cache ]; then
	rm config.cache
fi
