//
//  create_feed_thread.h
//
//  Created by gtliu on 7/19/13.
//  Copyright (c) 2013 GT. All rights reserved.
//

#ifndef CREATE_FEED_THREAD_H
#define CREATE_FEED_THREAD_H

#include "../include/mit_log_module.h"
#include "../include/mit_data_define.h"
#include <sys/types.h>

/**
 * The app's log and configure path names 
 * must be same with the app's name.
 * ex: app's name is "dev_watchdog"
 *     app's log file path is LOG_FILE_PATH"dev_watchdog"
 */
/** The device update daemon app name */
#define APP_NAME_UPAPPSD               "update_apps_daemon"
/** Use to store update apps daemon's configure file. */
#define CONF_PATH_UPAPPSD              APP_CONF_PATH APP_NAME_UPAPPSD"/"
/** Use to store update apps daemon's log files. */
#define LOG_PATH_UPAPPSD               LOG_FILE_PATH APP_NAME_UPAPPSD"/"
/** Watchdog's version number */
#define VERSION_UPAPPSD                "v1.0.1"

/**
 * Create the feed thread.
 * !!! The function will call pthread_detach(), so it resource will be
 *     auto released by system; if pthread_detach() failed, pthread_kill(SIGKILL)
 *     will be called.
 *     Before feed thread exit please keep feed_conf must vaild.!!!
 * @param feed_conf     : the configuration of the package,
 *                        the thread is responsible to relase the object;
 * @return MITFuncRetValue
 */
MITFuncRetValue create_feed_thread(struct feed_thread_configure *feed_conf);

/**
 * Unregister the app from watchdog.
 */
MITFuncRetValue unregister_watchdog();

#endif
