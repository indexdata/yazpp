#!/bin/sh
# $Id: buildconf.sh,v 1.3 2000-11-20 11:27:33 adam Exp $
aclocal
libtoolize --force
automake -a
autoconf
