#!/bin/sh
# $Id: buildconf.sh,v 1.1 2000-10-11 12:21:50 adam Exp $
libtoolize
automake -a
autoconf
