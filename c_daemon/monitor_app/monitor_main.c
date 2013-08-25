//
//  app_main.c
//
//  Created by gtliu on 7/19/13.
//  Copyright (c) 2013 GT. All rights reserved.
//

#include "../include/mit_data_define.h"
#include "watchdog_comm.h"
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <pthread.h>

/* the app name */
#define MONITOR_APP_NAME               "app4"
/* app's version number */
#define VERSION_MONITOR_APP            "v1.0.3"


static void *thread_start(void *arg)
{
    // init the udp
    int thread_socket_id = init_udp_socket();
    if (thread_socket_id <= 0) {
        MITLog_DetPrintf(MITLOG_LEVEL_ERROR, "init_udp_socket() failed");
        return NULL;
    }
    // send the register package
    int main_period = 8;
    pthread_t thread_id   = pthread_self();
    MITLog_DetPrintf(MITLOG_LEVEL_COMMON, "child thread start with id=%llu", thread_id);
    MITFuncRetValue func_ret;
    while (send_wd_register_package(thread_socket_id, main_period, thread_socket_id) != MIT_RETV_SUCCESS) {
        MITLog_DetPuts(MITLOG_LEVEL_COMMON, "send the register package one time...");
        sleep(2);
    }

    // periodically send the feed package
    int i = 0;
    while(i++ < 10) {
        if((func_ret=send_wd_feed_package(thread_socket_id, main_period, thread_socket_id)) != MIT_RETV_SUCCESS) {
            MITLog_DetPrintf(MITLOG_LEVEL_ERROR, "send_wd_feed_package() failed");
            if(func_ret == MIT_RETV_TIMEOUT) {
                MITLog_DetErrPrintf("Send feed package failed so the register package will be re-sent");
                while (send_wd_register_package(thread_socket_id, main_period, thread_socket_id) != MIT_RETV_SUCCESS) {
                    MITLog_DetPuts(MITLOG_LEVEL_COMMON, "send the register package one time...");
                    sleep(1);
                }
            }
        }
        sleep(4);
    }

    // un-register the monitored app
    while(send_wd_unregister_package(thread_socket_id, thread_socket_id) != MIT_RETV_SUCCESS) {
        MITLog_DetPrintf(MITLOG_LEVEL_ERROR, "send_wd_unregister_package() faild");
    }
    // close the socket
    close_udp_socket(thread_socket_id);
    return NULL;
}

int main(int argc, const char * argv[])
{
    MITLogOpen("UDPClient", LOG_FILE_PATH MONITOR_APP_NAME, _IOLBF);
    int ret = 0;
    char dir[1024];
    char *cwd_char = getcwd(dir, sizeof(dir));
    if (cwd_char == NULL) {
        MITLog_DetErrPrintf("getcwd() failed");
    }
    MITLog_DetPrintf(MITLOG_LEVEL_COMMON, "%s", dir);
    // save the appinfo
    MITFuncRetValue func_ret = save_appinfo_config(getpid(),
                                                   MONITOR_APP_NAME,
                                                   APP_EXEC_FILE_PATH MONITOR_APP_NAME,
                                                   VERSION_MONITOR_APP);
    //start two thread
    pthread_t thread_one, thread_two;
    pthread_create(&thread_one, NULL, thread_start, NULL);
    pthread_create(&thread_two, NULL, thread_start, NULL);

    if(func_ret != MIT_RETV_SUCCESS) {
        MITLog_DetPrintf(MITLOG_LEVEL_ERROR, "save_appinfo_config() failed");
        return -1;
    }
    // init the udp
    int main_socket_id = init_udp_socket();
    if (main_socket_id <= 0) {
        MITLog_DetPrintf(MITLOG_LEVEL_ERROR, "init_udp_socket() failed");
        return -1;
    }
    // send the register package
    int main_period = 5;
    pthread_t thread_id   = pthread_self();
    MITLog_DetPrintf(MITLOG_LEVEL_COMMON, "main thread start with id=%llu", thread_id);
    while (send_wd_register_package(main_socket_id, main_period, main_socket_id) != MIT_RETV_SUCCESS) {
        MITLog_DetPuts(MITLOG_LEVEL_COMMON, "send the register package one time...");
        sleep(2);
    }

    // periodically send the feed package
    int i = 0;
    while(i++ < 10) {
        if((func_ret=send_wd_feed_package(main_socket_id, main_period, main_socket_id)) != MIT_RETV_SUCCESS) {
            MITLog_DetPrintf(MITLOG_LEVEL_ERROR, "send_wd_feed_package() failed");
            if(func_ret == MIT_RETV_TIMEOUT) {
                MITLog_DetErrPrintf("Send feed package failed so the register package will be re-sent");
                while (send_wd_register_package(main_socket_id, main_period, main_socket_id) != MIT_RETV_SUCCESS) {
                    MITLog_DetPuts(MITLOG_LEVEL_COMMON, "send the register package one time...");
                    sleep(1);
                }
            }
        }
        sleep(4);
    }

    // un-register the monitored app
    while(send_wd_unregister_package(main_socket_id, main_socket_id) != MIT_RETV_SUCCESS) {
        MITLog_DetPrintf(MITLOG_LEVEL_ERROR, "send_wd_unregister_package() faild");
    }

    // close the socket
    close_udp_socket(main_socket_id);
    pthread_join(thread_one, NULL);
    pthread_join(thread_two, NULL);
    // free the app configure infomation
    free_appinfo_config();
    MITLogClose();
    return ret;
}

