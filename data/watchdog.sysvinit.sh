#!/bin/sh
### BEGIN INIT INFO
# Provides:          watchdog
# Required-Start:    $local_fs
# Required-Stop:     
# Default-Start:     2 3 4 5
# Default-Stop:      0 1 6
# Short-Description: Start/stop watchdog daemon
### END INIT INFO

NAME=watchdog
DAEMON=/usr/local/sbin/$NAME
PIDFILE=/var/run/watchdog.pid
SCRIPTNAME=/etc/init.d/watchdog
DESC="Linux watchdog daemon"


. /lib/lsb/init-functions

test -x $DAEMON || exit

case $1 in
	start)
		log_daemon_msg "Starting $DESC" "$NAME"
		start-stop-daemon --start --quiet --oknodo --pidfile $PIDFILE --exec $DAEMON --background
		log_end_msg $?
	;;
	stop)
		log_daemon_msg "Stopping $DESC" "$NAME"
		start-stop-daemon --stop --quiet --oknodo --pidfile $PIDFILE --exec $DAEMON
		log_end_msg $?
	;;
	*)
		echo "Usage: $SCRIPTNAME {start|stop}"
		exit 1
	;;
esac