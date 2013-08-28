#!/bin/bash
#for i in {1..10} 
#do
#    echo "push $i ..."
#    adb push app$i /data/apps/
#    rm app$i
#done

BS_LOCAL_PATH="/sdcard/bussale/apps/"
BS_ANDROID_PATH="/data/apps/"

dest="local"
while getopts "da:" option
do
        case "${option}"
        in
                d) dest="android";;
                a) app_name=${OPTARG};;
        esac
done


if [ $dest = "android" ]; then
        echo "Will push $app_name into $BS_ANDROID_PATH"
        adb push $app_name $BS_ANDROID_PATH

elif [[ $dest = "local" ]]; then
	echo "Will copy $app_name into $BS_LOCAL_PATH"
    cp $app_name $BS_LOCAL_PATH
fi

#rm $app_name


