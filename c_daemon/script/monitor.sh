#!/bin/sh
clear
while true ; do
	#statements
	echo "==================== PS INFO ======================"
	 ps -A -ocomm,pid,ppid,pcpu | egrep "dev_watchdog|app[0-9]"

	echo "================= WATCHDOG INFO ==================="
	echo ">>>>>>>WARNING LEVEL LOG"
	tail /sdcard/bussale/logs/watchdog/DeviceWatchdog.warn
	echo ">>>>>>>ERROR LEVEL LOG"
	tail /sdcard/bussale/logs/watchdog/DeviceWatchdog.err
	echo "==================================================="

	#check for when more apps will be started error is occour
	apps_num=`ps -A -ocomm,pid,ppid,pcpu | egrep "dev_watchdog|app[0-9]" | wc -l`
	echo "apps_num=$apps_num"
	if [[ $apps_num -gt 11 ]]; then
		killall dev_watchdog
		exit
	fi
	sleep 1
	clear
done
