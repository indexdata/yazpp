#!/bin/sh
# $Id: buildconf.sh,v 1.9 2002-09-10 12:00:20 adam Exp $
set -x
dir=`aclocal --print-ac-dir`
if [ -f $dir/yaz.m4 ]; then
	aclocal
else
	aclocal -I . 
fi
libtoolize --force 
automake -a 
autoconf
if [ -f config.cache ]; then
	rm config.cache
fi
