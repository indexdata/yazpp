#! /bin/sh
#
# skeleton	example file to build /etc/init.d/ scripts.
#		This file should be used to construct scripts for /etc/init.d.
#
#		Written by Miquel van Smoorenburg <miquels@cistron.nl>.
#		Modified for Debian GNU/Linux
#		by Ian Murdock <imurdock@gnu.ai.mit.edu>.
#
# Version:	@(#)skeleton  1.8  03-Mar-1998  miquels@cistron.nl
#
PATH=/usr/local/bin:/bin:/usr/bin
export PATH

RUNAS=nobody
LOGFILE=/var/log/proxy.log

if test `whoami` != $RUNAS; then
	touch $LOGFILE
	chown $RUNAS $LOGFILE
	su -c "$0 $*" $RUNAS
	exit 0
fi

# Proxy CWD is here. Should be writable by it.
DIR=/var/proxy
# Proxy Path
DAEMON=/usr/local/bin/yaz-proxy
# Proxy PIDFILE. Must be writable by it.
PIDFILE=$DIR/proxy.pid
# Port
PORT=9000
# Extra args . Config file _WITH_ option
ARGS="-c config.xml"

# Name, Description (not essential)
NAME=yaz-proxy
DESC="YAZ proxy"

test -d $DIR || exit 0
test -f $DAEMON || exit 0

set -e

case "$1" in
  start)
	echo -n "Starting $DESC: "
	cd $DIR
	$DAEMON -l $LOGFILE -p $PIDFILE $ARGS @:$PORT &
	echo "$NAME."
	;;
  stop)
	echo -n "Stopping $DESC: "

	if test -f $PIDFILE; then
		kill `cat $PIDFILE`
		echo "$NAME."
	else
		echo "No PID $PIDFILE"
	fi
	;;
  reload)
	if test -f $PIDFILE; then
		kill -INT `cat $PIDFILE`
	fi
  ;;
  restart|force-reload)
	echo -n "Restarting $DESC: "
	if test -f $PIDFILE; then
		kill `cat $PIDFILE`
	fi
	sleep 1
	cd $DIR
	$DAEMON -l $LOGFILE -p $PIDFILE $ARGS @:$PORT &
	echo "$NAME."
	;;
  *)
	N=/etc/init.d/$NAME
	# echo "Usage: $N {start|stop|restart|reload|force-reload}" >&2
	echo "Usage: $N {start|stop|restart|force-reload}" >&2
	exit 1
	;;
esac

exit 0
