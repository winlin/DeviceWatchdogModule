#!/bin/sh
destion="-d"
if [ x$1 = x ]; then
	destion=""
fi

./script/push_shell.sh $destion -a ./device_watchdog/dev_watchdog 
./script/push_shell.sh $destion -a ./monitor_app/app1
./script/push_shell.sh $destion -a ./update_app/update_apps_daemon
 