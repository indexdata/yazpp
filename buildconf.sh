#!/bin/sh
# $Id: buildconf.sh,v 1.2 2000-10-26 21:31:50 adam Exp $
aclocal
libtoolize
automake -a
autoconf
