//
//  installer_daemon.c
//
//  Created by gtliu on 8/2/13.
//  Copyright (c) 2013 GT. All rights reserved.
//
#include "../include/mit_log_module.h"
#include "../include/mit_data_define.h"
#include "installer_module.h"
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <time.h>
#include <fcntl.h>
#include <signal.h>
#include <event2/event.h>
#include <event2/event_struct.h>
#include <event2/event-config.h>
#include <event2/util.h>

static int app_socket_fd;
static int main_thread_period = 5;
static struct sockaddr_in addr_server;
static struct feed_thread_configure th_conf;
static WDConfirmedUnregFlag wd_confirmed_register;

static int max_missed_feed_time;

void socket_ev_r_cb(evutil_socket_t fd, short ev_type, void *data)
{
    //TODO: recieve from the watchdog and db_sync
    MITLog_DetPuts(MITLOG_LEVEL_COMMON, "There is a package recieved");
    if (ev_type & EV_READ) {
        struct event_base *ev_base = ((struct event *)data)->ev_base;
        char msg[MAX_UDP_PG_SIZE] = {0};
        struct sockaddr_in src_addr;
        socklen_t addrlen = sizeof(src_addr);
        ssize_t len = recvfrom(fd, msg, sizeof(msg)-1, 0, (struct sockaddr *)&src_addr, &addrlen);
        if (len > 0) {
            MITWatchdogPgCmd cmd = wd_get_net_package_cmd(msg);
            MITLog_DetPrintf(MITLOG_LEVEL_COMMON, "Get Server CMD:%d", cmd);

            struct wd_pg_return *ret_pg = wd_pg_return_unpg(msg, (int)len);
            if (ret_pg) {
                if (cmd == WD_PG_CMD_REGISTER) {
                    if (ret_pg->error != WD_PG_ERR_SUCCESS) {
                        MITLog_DetPrintf(MITLOG_LEVEL_ERROR, "send rigister package failed, so package will be sent again");
                        monapp_send_register_pg(fd,
                                                main_thread_period,
                                                fd,
                                                &addr_server,
                                                &th_conf);
                    } else {
                        MITLog_DetPrintf(MITLOG_LEVEL_COMMON, "great! rigister success");
                        wd_confirmed_register = WD_CONF_REG_HAS_CONF;
                    }
                } else if (cmd == WD_PG_CMD_FEED) {
                    MITLog_DetPrintf(MITLOG_LEVEL_COMMON, "Get Server Feed Back");
                    if (ret_pg->error != WD_PG_ERR_SUCCESS) {
                        MITLog_DetPrintf(MITLOG_LEVEL_ERROR,
                                         "send feed package failed:%d and rigister package will resend",
                                         ret_pg->error);
                        wd_confirmed_register = WD_CONF_REG_NOT_CONF;
                    } else {
                        MITLog_DetPrintf(MITLOG_LEVEL_COMMON, "send feed package success");
                        max_missed_feed_time = 0;
                    }
                } else if (cmd == WD_PG_CMD_UNREGISTER) {
                    MITLog_DetPuts(MITLOG_LEVEL_COMMON, "Get Server Unregister Feed Back");
                    /* handle unregister */
                    if (ret_pg->error != WD_PG_ERR_SUCCESS) {
                        MITLog_DetPrintf(MITLOG_LEVEL_ERROR, "send unregister package failed:%d", ret_pg->error);
                        monapp_send_action_pg(fd,
                                              0,
                                              th_conf.monitored_pid,
                                              fd,
                                              WD_PG_CMD_UNREGISTER,
                                              &addr_server);
                    } else {
                        MITLog_DetPrintf(MITLOG_LEVEL_COMMON, "send unregister package success");
                        if(event_base_loopbreak(ev_base) < 0) {
                            MITLog_DetPrintf(MITLOG_LEVEL_ERROR, "event_base_loopbreak() failed.");
                        }
                    }
                } else {
                    MITLog_DetPrintf(MITLOG_LEVEL_ERROR, "Unknown cmd recieved:%d", cmd);
                }
                free(ret_pg);
            } else {
                MITLog_DetPrintf(MITLOG_LEVEL_ERROR, "wd_pg_return_unpg() failed");
            }
        }
    }
}

void timeout_cb(int fd, short ev_type, void* data)
{
    //TODO: register and feed to the watchdog
    if (wd_confirmed_register == WD_CONF_REG_NOT_CONF) {
        monapp_send_register_pg(app_socket_fd,
                                main_thread_period,
                                app_socket_fd,
                                &addr_server,
                                &th_conf);
    } else if (wd_confirmed_register == WD_CONF_REG_HAS_CONF) {
        if(max_missed_feed_time++ < MAX_MISS_FEEDBACK_TIMES) {
            monapp_send_action_pg(app_socket_fd,
                                  main_thread_period,
                                  th_conf.monitored_pid,
                                  app_socket_fd,
                                  WD_PG_CMD_FEED,
                                  &addr_server);
        } else {
            wd_confirmed_register = WD_CONF_REG_NOT_CONF;
            max_missed_feed_time = 0;
        }
    }
}

MITFuncRetValue start_listen_and_feed(int main_th_period)
{
    wd_confirmed_register = WD_CONF_REG_NOT_CONF;
    max_missed_feed_time = 0;

    MITFuncRetValue func_ret = MIT_RETV_SUCCESS;
    app_socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (app_socket_fd < 0) {
        MITLog_DetErrPrintf("socket() failed");
        func_ret = MIT_RETV_OPEN_FILE_FAIL;
        goto FUNC_EXIT_TAG;
    }

    int set_value = 1;
    if(setsockopt(app_socket_fd, SOL_SOCKET, SO_REUSEADDR, (void *)&set_value, sizeof(set_value)) < 0) {
        MITLog_DetErrPrintf("setsockopt() failed");
    }
    if (fcntl(app_socket_fd, F_SETFD, FD_CLOEXEC) < 0) {
        MITLog_DetErrPrintf("fcntl() failed");
    }

    struct sockaddr_in addr_self;
    memset(&addr_self, 0, sizeof(addr_self));
    addr_self.sin_family        = AF_INET;
    addr_self.sin_port          = 0;
    /* any local network card is ok */
    addr_self.sin_addr.s_addr   = INADDR_ANY;

    if (bind(app_socket_fd, (struct sockaddr*)&addr_self, sizeof(addr_self)) < 0) {
        MITLog_DetErrPrintf("bind() failed");
        func_ret = MIT_RETV_FAIL;
        goto CLOSE_FD_TAG;
    }
    socklen_t addr_len = sizeof(addr_self);
    if (getsockname(app_socket_fd, (struct sockaddr *)&addr_self, &addr_len) < 0) {
        MITLog_DetErrPrintf("getsockname() failed");
        func_ret = MIT_RETV_FAIL;
        goto CLOSE_FD_TAG;
    }
    /* save port info */
    MITLog_DetPrintf(MITLOG_LEVEL_COMMON, "Get Port:%d", ntohs(addr_self.sin_port));
    char port_str[16] = {0};
    sprintf(port_str, "%d", ntohs(addr_self.sin_port));
    if(save_app_conf_info(APP_NAME_INSTALLERD, F_NAME_COMM_PORT, port_str) != MIT_RETV_SUCCESS) {
        MITLog_DetErrPrintf("save_app_conf_info() %s/%s failed", APP_NAME_INSTALLERD, F_NAME_COMM_PORT);
        func_ret = MIT_RETV_FAIL;
        goto CLOSE_FD_TAG;
    }
    struct event_base *ev_base = event_base_new();
    if (!ev_base) {
        MITLog_DetErrPrintf("event_base_new() failed");
        func_ret = MIT_RETV_FAIL;
        goto CLOSE_FD_TAG;
    }

    /* Add UDP server event */
    struct event socket_ev_r;
    event_assign(&socket_ev_r, ev_base, app_socket_fd, EV_READ|EV_PERSIST, socket_ev_r_cb, &socket_ev_r);
    if (event_add(&socket_ev_r, NULL) < 0) {
        MITLog_DetPrintf(MITLOG_LEVEL_ERROR, "couldn't add an event");
        func_ret = MIT_RETV_FAIL;
        goto EVENT_BASE_FREE_TAG;
    }
    /* Add timer event */
    struct event timeout;
    struct timeval tv;
    event_assign(&timeout, ev_base, -1, EV_PERSIST, timeout_cb, &timeout);
    evutil_timerclear(&tv);
    tv.tv_sec = main_th_period - 1;
    event_add(&timeout, &tv);

    event_base_dispatch(ev_base);

EVENT_BASE_FREE_TAG:
    event_base_free(ev_base);
CLOSE_FD_TAG:
    close(app_socket_fd);
FUNC_EXIT_TAG:
    MITLog_DetLogExit
    return func_ret;
}

int main(int argc, const char * argv[])
{
    /*
     * 1. convert self into a daemon
     *    don't change process's working directory to '/'
     *    don't redirect stdin/stdou/stderr
     */
    int ret = 0;
#ifndef MITLOG_DEBUG_ENABLE
	ret = daemon(1, 1);
	if(ret == -1) {
		perror("call daemon() failed!");
	}
#endif
    MITLogOpen("InstallerDaemon", LOG_FILE_PATH APP_NAME_INSTALLERD, _IOLBF);

    MITLog_DetPrintf(MITLOG_LEVEL_COMMON, "daemon ppid:%d pid:%d",  getppid(), getpid());

    char dir[1024];
    char *cwd_char = getcwd(dir, sizeof(dir));
    if (cwd_char == NULL) {
        MITLog_DetErrPrintf("getcwd() failed");
    }
    MITLog_DetPrintf(MITLOG_LEVEL_COMMON, "%s", dir);

    /* save pid and version info */
    char tmp_str[16] = {0};
    sprintf(tmp_str, "%d", getpid());
    if(save_app_pid_ver_info(APP_NAME_INSTALLERD, getpid(), VERSION_INSTALLERD) != MIT_RETV_SUCCESS) {
        MITLog_DetErrPrintf("save_app_pid_ver_info() %s failed", APP_NAME_INSTALLERD);
        ret = -1;
        goto CLOSE_LOG_TAG;
    }

    th_conf.cmd_line = "/data/bussale/apps/" APP_NAME_INSTALLERD;
    th_conf.app_name = APP_NAME_INSTALLERD;
    th_conf.monitored_pid = getpid();

    MITFuncRetValue f_ret = start_listen_and_feed(main_thread_period);
    if(f_ret != MIT_RETV_SUCCESS) {
        MITLog_DetPrintf(MITLOG_LEVEL_ERROR, "start_listen_and_feed() failed");
    }

CLOSE_LOG_TAG:
    MITLogClose();
    return ret;
}

