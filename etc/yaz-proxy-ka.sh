#!/bin/sh
# $Id: yaz-proxy-ka.sh,v 1.1 2003-10-24 10:33:01 adam Exp $
i=1
while test $i -lt 20; do
	$*
	test $? && exit 0
	if test -f core; then
		mv -f core core.`date +%Y%m%d%k%M`
	fi
	sleep 1
	i=`expr $i + 1`
done	
