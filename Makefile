WD_PROGNAME= watchdog
APP_PROGNAME= app1

CC = gcc
DEFS= -D_GNU_SOURCE
INCLUDES= -I.
LIBS= -lpthread -levent

rm= /bin/rm -rf
INSTALLDIR= /tmp/apps/

DEFINES= $(INCLUDES) $(defs) -DSYS_UNIX=1
CFLAGS= $(DEFINES) -Wall -O2 -std=gnu99 

DATA_DEFINE_M_NAME= mit_data_define.o
LOG_MODULE_M_NAME= mit_log_module.o
COMM_FILE_TAR= comm_file_tar

MAKE= make

export


all: $(COMM_FILE_TAR) $(WD_PROGNAME) $(APP_PROGNAME)

$(COMM_FILE_TAR):
	$(MAKE) -C src

$(WD_PROGNAME): $(COMM_FILE_TAR)
	$(MAKE) -C device_watchdog

$(APP_PROGNAME): $(COMM_FILE_TAR)
	$(MAKE) -C monitor_app

install:
	$(MAKE) install -C monitor_app
	$(MAKE) install -C device_watchdog

clean:
	 $(MAKE) clean -C src
	 $(MAKE) clean -C monitor_app
	 $(MAKE) clean -C device_watchdog