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

/** the app name */
#define MONITOR_APP_NAME               "app1"
/** app's version number */
#define VERSION_MONITOR_APP            "v1.0.3"

int main(int argc, const char * argv[])
{
    MITLogOpen("UDPClient", LOG_FILE_PATH MONITOR_APP_NAME);
    int ret = 0;
    char dir[1024];
    char *cwd_char = getcwd(dir, sizeof(dir));
    if (cwd_char == NULL) {
        MITLog_DetErrPrintf("getcwd() failed");
    }
    MITLog_DetPrintf(MITLOG_LEVEL_COMMON, "%s", dir);
    // save the appinfo
    MITFuncRetValue func_ret = save_appinfo_config(getpid(), 5, MONITOR_APP_NAME, APP_EXEC_FILE_PATH MONITOR_APP_NAME, VERSION_MONITOR_APP);
    if(func_ret != MIT_RETV_SUCCESS) {
        MITLog_DetPrintf(MITLOG_LEVEL_ERROR, "save_appinfo_config() failed");
        return -1;
    }
    // init the udp
    func_ret = init_udp_socket();
    if (func_ret != MIT_RETV_SUCCESS) {
        MITLog_DetPrintf(MITLOG_LEVEL_ERROR, "init_udp_socket() failed");
        return -1;
    }
    // send the register package
    while (send_wd_register_package() != MIT_RETV_SUCCESS) {
        MITLog_DetPuts(MITLOG_LEVEL_COMMON, "send the register package one time...");
        sleep(2);
    }

    // periodically send the feed package
    int i = 0;
    while(i++ < 10) {
        if((func_ret=send_wd_feed_package()) != MIT_RETV_SUCCESS) {
            MITLog_DetPrintf(MITLOG_LEVEL_ERROR, "send_wd_feed_package() failed");
            if(func_ret == MIT_RETV_TIMEOUT) {
                MITLog_DetErrPrintf("Send feed package failed so the register package will be re-sent");
                while (send_wd_register_package() != MIT_RETV_SUCCESS) {
                    MITLog_DetPuts(MITLOG_LEVEL_COMMON, "send the register package one time...");
                    sleep(1);
                }
            }
        }
        sleep(4);
    }

    // un-register the monitored app
    if(send_wd_unregister_package() != MIT_RETV_SUCCESS) {
        MITLog_DetPrintf(MITLOG_LEVEL_ERROR, "send_wd_unregister_package() faild");
    }

    // close the socket
    close_udp_socket();

    MITLogClose();
    return ret;
}

