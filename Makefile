WD_PROGNAME=dev_watchdog
UPAPP_PROGNAME=update_apps_daemon
APP_PROGNAME=app10

CC=gcc
#CC=arm-linux-androideabi-gcc

CROSS_INCLUDE_PATH=/home/gtliu/AndroidSDK/android_libevent

ifeq ($(CC),gcc)
	INCLUDES= -I.
	LIBS= -lpthread -levent
else 
	INCLUDES= -I$(CROSS_INCLUDE_PATH)/include/
	LIBS= -L$(CROSS_INCLUDE_PATH)/lib/ -levent
endif

rm= /bin/rm -rf
INSTALLDIR= /data/apps/

DEFS= -D_GNU_SOURCE
DEFINES= $(INCLUDES) $(defs) -DSYS_UNIX=1
CFLAGS= $(DEFINES) -Wall -O2 -std=gnu99 

DATA_DEFINE_M_NAME= mit_data_define.o
LOG_MODULE_M_NAME= mit_log_module.o
COMM_FILE_TAR= comm_file_tar

MAKE= make

export


all: $(COMM_FILE_TAR) $(WD_PROGNAME) $(APP_PROGNAME) $(UPAPP_PROGNAME)

$(COMM_FILE_TAR):
	$(MAKE) -C src

$(WD_PROGNAME): $(COMM_FILE_TAR)
	$(MAKE) -C device_watchdog

$(APP_PROGNAME): $(COMM_FILE_TAR)
	$(MAKE) -C monitor_app

$(UPAPP_PROGNAME): $(COMM_FILE_TAR)
	$(MAKE) -C update_app

install:
	$(MAKE) install -C monitor_app
	$(MAKE) install -C device_watchdog
	$(MAKE) install -C update_app
clean:
	 $(MAKE) clean -C src
	 $(MAKE) clean -C monitor_app
	 $(MAKE) clean -C device_watchdog
	 $(MAKE) clean -C update_app

