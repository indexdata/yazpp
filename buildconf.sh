#!/bin/sh
# $Id: buildconf.sh,v 1.5 2001-02-21 11:25:07 adam Exp $
dir=`aclocal --print-ac-dir`
if [ -f $dir/yaz.m4 ]; then
	aclocal
else
	aclocal -I .
fi
libtoolize --force
automake -a
autoconf
