#!/bin/sh
# $Id: yaz-proxy-ka.sh,v 1.3 2003-10-24 12:35:37 adam Exp $
#
# YAZ proxy keepalive wrapper, use this when testing the proxy.
#
# Allow core dumps when testing.
ulimit -c 200000
#
LOGFILE=/var/log/yaz-proxy-ka.log
#
touch $LOGFILE || exit 1
i=1
while test $i -lt 100; do
	date >>$LOGFILE
	echo "Starting proxy iteration=$i" >>$LOGFILE
	yaz-proxy $*
	code=$?
	date >>$LOGFILE
	echo "Proxy Stopped. Exit code=$code" >>$LOGFILE
	if test "$code" = "143"; then
		echo "Got TERM. Exiting" >>$LOGFILE
		exit 0
	fi
	if test "$code" = "129"; then
		echo "Got HUP. Exiting" >>$LOGFILE
		exit 0
	fi
	if test "$code" = "137"; then
		echo "Got KILL. Exiting" >>$LOGFILE
		exit 0
	fi
	if test "$code" = "0"; then
		echo "Exit 0. Exiting" >>$LOGFILE
		exit 0
	fi
	if test -f core; then
		echo "Saving core file" >>$LOGFILE
		mv -f core core.`date +%Y%m%d%k%M`
	fi
	sleep 1
	i=`expr $i + 1`
done	
