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
	echo ">>>>>>>WARNING LEVEL LOG"
	tail /data/logs/watchdog/DeviceWatchdog.warn
	echo ">>>>>>>ERROR LEVEL LOG"
	tail /data/logs/watchdog/DeviceWatchdog.err
	echo "==================================================="
	sleep 1
	clear
done
