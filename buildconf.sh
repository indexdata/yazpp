#!/bin/sh
# $Id: buildconf.sh,v 1.4 2000-12-07 13:27:14 adam Exp $
aclocal -I .
libtoolize --force
automake -a
autoconf
