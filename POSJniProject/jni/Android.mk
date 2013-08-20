LOCAL_PATH := $(call my-dir)
 
include $(CLEAR_VARS)
 
LOCAL_MODULE    := native_utilities
LOCAL_SRC_FILES := native_utilities.c watchdog_comm.c src/mit_data_define.c src/mit_log_module.c 
LOCAL_LDLIBS := -llog

MY_LIBEVENT_PATH=/home/gtliu/AndroidSDK/android_libevent
LOCAL_CFLAGS := -I$(MY_LIBEVENT_PATH)/include/  -L$(MY_LIBEVENT_PATH)/lib/ -levent -Wall -O2 -std=gnu99 

include $(BUILD_SHARED_LIBRARY)