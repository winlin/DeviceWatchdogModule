#!/bin/sh
clear
while true ; do
	#statements
	echo "==================== PS INFO ======================"
	 ps -eo pid,ppid,comm,pcpu | egrep "watchdog|app[0-9]"
	echo "==================================================="

	echo "================= WATCHDOG INFO ==================="
	echo ">>>>>>>COMM LEVEL LOG"
	tail /tmp/logs/watchdog/DeviceWatchdog.comm
	echo ">>>>>>>WARNING LEVEL LOG"
	tail /tmp/logs/watchdog/DeviceWatchdog.warn
	echo ">>>>>>>ERROR LEVEL LOG"
	tail /tmp/logs/watchdog/DeviceWatchdog.err
	echo "==================================================="
	sleep 1
	clear
done