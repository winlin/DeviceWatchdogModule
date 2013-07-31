#!/bin/sh
sudo mkdir -p /data/watchdog
sudo mkdir -p /data/logs
sudo mkdir -p /data/apps

sudo chmod a+wrx -R /data

clear
while true ; do
	#statements
	echo "==================== PS INFO ======================"
	 ps -A -ocomm,pid,ppid,pcpu | egrep "dev_watchdog|app[0-9]"

	echo "================= WATCHDOG INFO ==================="
	echo ">>>>>>>COMM LEVEL LOG"
	tail -n 25 /data/logs/watchdog/DeviceWatchdog.comm
	#echo ">>>>>>>WARNING LEVEL LOG"
	#tail /tmp/logs/watchdog/DeviceWatchdog.warn
	#echo ">>>>>>>ERROR LEVEL LOG"
	#tail /tmp/logs/watchdog/DeviceWatchdog.err
	echo "==================================================="
	sleep 1
	clear
done
