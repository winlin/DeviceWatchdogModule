//
//  up_apps_module.c
//  UpdateAppsDaemon
//
//  Created by gtliu on 8/2/13.
//  Copyright (c) 2013 GT. All rights reserved.
//

#include "../include/mit_log_module.h"
#include "../include/mit_data_define.h"
#include "up_apps_module.h"
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <event2/event.h>
#include <event2/event_struct.h>
#include <event2/util.h>
#include <event2/event-config.h>

static struct up_app_info *app_list_head;

MITFuncRetValue update_c_app(struct up_app_info *app_info)
{
    MITFuncRetValue f_ret = MIT_RETV_SUCCESS;
    /* check the verson number */
    char ver_str[30] = {0};
    get_app_version(app_info->app_name, ver_str);
    if (strlen(ver_str) == 0) {
        MITLog_DetPrintf(MITLOG_LEVEL_ERROR, "get_app_version() failed");
        f_ret = MIT_RETV_FAIL;
        goto FUNC_RET_TAG;
    }

    //TODO: compare the verson info to decside whethe to update the app

    /* create the update lock file */
    if(create_update_lock_file(app_info->app_name) !=0 ) {
        MITLog_DetErrPrintf("create_update_lock_file(%s) failed", app_info->app_name);
        f_ret = MIT_RETV_FAIL;
        goto FUNC_RET_TAG;
    }
    /* backup the app */
    if(backup_application(app_info->app_name) != 0) {
        MITLog_DetErrPrintf("backup_application(%s) failed", app_info->app_name);
        f_ret = MIT_RETV_FAIL;
        goto REMOVE_LOCK_FILE_TAG;
    }
    /* kill the app */
    long long int pid = get_pid_with_comm(app_info->app_name);
    if (pid > 0 && kill((pid_t)pid, SIGKILL) < 0) {
        MITLog_DetErrPrintf("kill() pid=%lld failed", pid);
        f_ret = MIT_RETV_FAIL;
        goto REMOVE_LOCK_FILE_TAG;
    }
    /* replace the app */
    if(replace_the_application(app_info->app_name, app_info->new_app_path) != 0) {
        MITLog_DetErrPrintf("replace_the_application():%s failed", app_info->app_name);
        f_ret = MIT_RETV_FAIL;
        goto REMOVE_LOCK_FILE_TAG;
    }

    //TODO: start the new verson app
    // if want to start the app we must have the cmd line

REMOVE_LOCK_FILE_TAG:
    /* remove the update lock file */
    if(remove_update_lock_file(app_info->app_name) != 0) {
        MITLog_DetErrPrintf("remove_update_lock_file():%s failed", app_info->app_name);
        f_ret = MIT_RETV_FAIL;
    }
FUNC_RET_TAG:
    return f_ret;
}

void timeout_cb(evutil_socket_t fd, short ev_type, void *data)
{
    MITLog_DetLogEnter
    waitpid(-1, NULL, WNOHANG);
    struct up_app_info *iter = app_list_head;
    struct up_app_info *pre_iter = NULL;
    while (iter) {
        MITFuncRetValue f_ret = MIT_RETV_FAIL;
        switch (iter->app_type) {
            case UPAPP_TYPE_C:
                //TODO: realize the update C app
                if((f_ret = update_c_app(iter)) != MIT_RETV_SUCCESS) {
                    MITLog_DetPrintf(MITLOG_LEVEL_ERROR, "update_c_app() failed");
                }
                break;
            case UPAPP_TYPE_KMODULE:
                //TODO: realize the update kernel module
                break;
            case UPAPP_TYPE_JAVA:
                //TODO: realize the update java app
                break;
            default:
                MITLog_DetPrintf(MITLOG_LEVEL_ERROR, "Unknown update app type:%d", iter->app_type);
                break;
        }
        //TODO: if success release the node
        if (f_ret == MIT_RETV_SUCCESS) {
            struct up_app_info *tmp = iter;
            iter = iter->next_node;
            if (tmp == app_list_head) {
                app_list_head = pre_iter = iter;
            } else {
                pre_iter->next_node = iter;
            }
            free(tmp->app_name);
            free(tmp->new_app_path);
            free(tmp->new_version);
            free(tmp);
        } else {
           pre_iter = iter;
           iter = iter->next_node;
        }
    }
    MITLog_DetLogExit
}

MITFuncRetValue start_app_update_func(struct up_app_info **head)
{
    MITLog_DetLogEnter
    *head = app_list_head;

    /* create a test update app */
    app_list_head = calloc(1, sizeof(struct up_app_info));
    if (app_list_head == NULL) {
        MITLog_DetErrPrintf("calloc() failed");
        return MIT_RETV_FAIL;
    }
    app_list_head->app_type = UPAPP_TYPE_C;
    app_list_head->app_name = strdup("app1");
    app_list_head->new_app_path = strdup("/sdcard/db_app/app1");
    app_list_head->new_version = strdup("v1.0.3");
    app_list_head->next_node = NULL;
    struct up_app_info *sec_node = calloc(1, sizeof(struct up_app_info));
    if (sec_node == NULL) {
        MITLog_DetErrPrintf("calloc() failed");
    } else {
        sec_node->app_type = UPAPP_TYPE_C;
        sec_node->app_name = strdup("app1");
        sec_node->new_app_path = strdup("/sdcard/db_app/app1");
        sec_node->new_version = strdup("v1.0.3");
        sec_node->next_node = NULL;
    }
    app_list_head->next_node = sec_node;

    MITFuncRetValue func_ret = MIT_RETV_SUCCESS;
    struct event_base *ev_base = event_base_new();
    if (ev_base == NULL) {
        MITLog_DetErrPrintf("event_base_new() failed");
        func_ret = MIT_RETV_FAIL;
        goto FUNC_EXIT_TAG;
    }
    /* add timer event */
    struct event timeout;
    struct timeval tv;
    event_assign(&timeout, ev_base, -1, EV_PERSIST, timeout_cb, &timeout);
    evutil_timerclear(&tv);
    tv.tv_sec = UP_APP_DAEMON_TIME_INTERVAL;
    event_add(&timeout, &tv);

    MITLog_DetPuts(MITLOG_LEVEL_COMMON, "start the event dispatch");
    event_base_dispatch(ev_base);
    MITLog_DetPuts(MITLOG_LEVEL_COMMON, "end the event dispatch");

    event_base_free(ev_base);
FUNC_EXIT_TAG:
    MITLog_DetLogExit
    return func_ret;
}

void free_up_app_list(struct up_app_info *head)
{
    MITLog_DetLogEnter
    struct up_app_info *iter = head;
    struct up_app_info *tmp = NULL;
    while (iter != NULL) {
        tmp = iter;
        iter = iter->next_node;
        free(tmp->app_name);
        free(tmp->new_app_path);
        free(tmp->new_version);
        free(tmp);
    }
    MITLog_DetLogExit
}
