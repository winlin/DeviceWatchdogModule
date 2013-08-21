#!/bin/bash
for i in {1..10} 
do
    echo "push $i ..."
    adb push app$i /sdcard/bussale/apps/
    rm app$i
done

