#!/bin/sh
# $Id: buildconf.sh,v 1.7 2002-04-15 09:43:30 adam Exp $
dir=`aclocal --print-ac-dir`
if [ -f $dir/yaz.m4 ]; then
	aclocal
else
	aclocal -I . 
fi
libtoolize --force 
automake -a 
autoconf || exit 4
if [ -f config.cache ]; then
	rm config.cache
fi
