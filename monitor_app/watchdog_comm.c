#include "watchdog_comm.h"
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>
#include <fcntl.h>
#include <signal.h>

#define UDP_IP_SER                  "127.0.0.1"
#define UDP_REC_TIMEOUT_SEC         1
#define UDP_SND_TIMEOUT_SEC         UDP_REC_TIMEOUT_SEC

static int app_socket_fd;
static struct sockaddr_in addr_server;
static struct feed_thread_configure feed_configure;

void recvfrom_watchdog_callback(int cmd, int error_code)
{
    // TODO: call JAVA callback function
    MITLog_DetPrintf(MITLOG_LEVEL_COMMON, "Get watchdog return: [cmd:%d  error_code:%d]", cmd, error_code);
}

MITFuncRetValue wd_send_register_pg(int fd)
{
    // read from watchdog port file to get port
    FILE *fp = fopen(CONF_PATH_WATCHD F_NAME_COMM_PORT, "r");
    if (fp == NULL) {
        return MIT_RETV_FAIL;
    }
    int wd_port = 0;
    int scan_num = fscanf(fp, "%d", &wd_port);
    fclose(fp);
    if (scan_num <= 0) {
        MITLog_DetErrPrintf("fscanf() failed");
        return MIT_RETV_FAIL;
    }
    MITLog_DetPrintf(MITLOG_LEVEL_COMMON, "Get Watchdog port:%d", wd_port);
    if (wd_port > 0) {
        memset(&addr_server, 0, sizeof(addr_server));
        addr_server.sin_family      = AF_INET;
        addr_server.sin_port        = htons(wd_port);
        addr_server.sin_addr.s_addr = inet_addr(UDP_IP_SER);

        int pg_len = 0;
        void *pg_reg = wd_pg_register_new(&pg_len, &feed_configure);
        MITLog_DetPrintf(MITLOG_LEVEL_COMMON, "register pg len:%d", pg_len);
        if (pg_reg > 0) {
            ssize_t ret = sendto(fd, pg_reg,
                                 (size_t)pg_len, 0,
                                 (struct sockaddr *)&addr_server,
                                 sizeof(struct sockaddr_in));
            free(pg_reg);
            if (ret < 0) {
                MITLog_DetErrPrintf("sendto() failed");
                return MIT_RETV_FAIL;
            }
            return MIT_RETV_SUCCESS;
        }
        else {
            MITLog_DetPrintf(MITLOG_LEVEL_ERROR, "wd_pg_register_new() failed");
            return MIT_RETV_FAIL;
        }
    } else {
        MITLog_DetPrintf(MITLOG_LEVEL_ERROR, "Get Watchdog port failed");
        return MIT_RETV_FAIL;
    }
}

MITFuncRetValue save_appinfo_config(pid_t monitored_pid,
                                    unsigned long feed_period,
                                    const char *app_name,
                                    const char *cmd_line,
                                    const char *version_str)
{
    if(strlen(app_name) == 0||
            strlen(cmd_line) == 0||
            strlen(version_str) == 0) {
        MITLog_DetPrintf(MITLOG_LEVEL_ERROR, "app_name/cmd_line/version_str paramater cannot be null");
        return MIT_RETV_PARAM_ERROR;
    }

    feed_configure.monitored_pid = monitored_pid;
    feed_configure.feed_period   = feed_period;
    feed_configure.app_name      = strdup(app_name);
    if(feed_configure.app_name == NULL) {
        MITLog_DetErrPrintf("strdup() failed");
        return MIT_RETV_ALLOC_MEM_FAIL;
    }
    feed_configure.cmd_line      = strdup(cmd_line);
    if(feed_configure.cmd_line == NULL) {
        free(feed_configure.app_name);
        feed_configure.app_name  = NULL;
        MITLog_DetErrPrintf("strdup() failed");
        return MIT_RETV_ALLOC_MEM_FAIL;
    }
    /* save pid info */
    char tmp_str[16] = {0};
    sprintf(tmp_str, "%d", monitored_pid);
    if(save_app_conf_info(app_name, F_NAME_COMM_PID, tmp_str) != MIT_RETV_SUCCESS) {
        MITLog_DetErrPrintf("save_app_conf_info() %s/%s failed", app_name, F_NAME_COMM_PID);
        return MIT_RETV_FAIL;
    }
    /* save verson info */
    if(save_app_conf_info(app_name, F_NAME_COMM_VERSON, version_str) != MIT_RETV_SUCCESS) {
        MITLog_DetErrPrintf("save_app_conf_info() %s/%s failed", app_name, F_NAME_COMM_VERSON);
        return MIT_RETV_FAIL;
    }
    return MIT_RETV_SUCCESS;
}

MITFuncRetValue init_udp_socket(void)
{
    app_socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (app_socket_fd < 0) {
        MITLog_DetErrPrintf("socket() failed");
        return MIT_RETV_FAIL;
    }
    int set_value = 1;
    if(setsockopt(app_socket_fd, SOL_SOCKET, SO_REUSEADDR, (void *)&set_value, sizeof(set_value)) < 0) {
        MITLog_DetErrPrintf("setsockopt() failed");
        close(app_socket_fd);
        return MIT_RETV_FAIL;
    }
    if (fcntl(app_socket_fd, F_SETFD, FD_CLOEXEC) < 0) {
        MITLog_DetErrPrintf("fcntl() failed");
        close(app_socket_fd);
        return MIT_RETV_FAIL;
    }
    /* To get the app's local port so we bind first */
    struct sockaddr_in addr_self;
    memset(&addr_self, 0, sizeof(addr_self));
    addr_self.sin_family      = AF_INET;
    addr_self.sin_port        = 0;
    addr_self.sin_addr.s_addr = INADDR_ANY;
    if (bind(app_socket_fd, (struct sockaddr *)&addr_self, sizeof(addr_self)) < 0) {
        MITLog_DetErrPrintf("bind() failed");
        close(app_socket_fd);
        return MIT_RETV_FAIL;
    }
    /* save port info */
    MITLog_DetPrintf(MITLOG_LEVEL_COMMON, "Get Port:%d", ntohs(addr_self.sin_port));
    char port_str[16] = {0};
    sprintf(port_str, "%d", ntohs(addr_self.sin_port));
    if(save_app_conf_info(feed_configure.app_name, F_NAME_COMM_PORT, port_str) != MIT_RETV_SUCCESS) {
        MITLog_DetErrPrintf("save_app_conf_info() %s/%s failed", feed_configure.app_name, F_NAME_COMM_PORT);
        close(app_socket_fd);
        return MIT_RETV_FAIL;
    }
    /* set the recieve/send timeout for 1 second */
    struct timeval socket_tv;
    socket_tv.tv_sec = UDP_REC_TIMEOUT_SEC;
    socket_tv.tv_usec = 0;
    if(setsockopt(app_socket_fd, SOL_SOCKET, SO_RCVTIMEO, (void *)&socket_tv, sizeof(socket_tv)) < 0) {
        MITLog_DetErrPrintf("setsockopt() recieve timeout failed");
        close(app_socket_fd);
        return MIT_RETV_FAIL;
    }
    socket_tv.tv_sec = UDP_SND_TIMEOUT_SEC;
    if(setsockopt(app_socket_fd, SOL_SOCKET, SO_SNDTIMEO, (void *)&socket_tv, sizeof(socket_tv)) < 0) {
        MITLog_DetErrPrintf("setsockopt() send timeout failed");
        close(app_socket_fd);
        return MIT_RETV_FAIL;
    }
    return MIT_RETV_SUCCESS;
}

MITFuncRetValue recvfrom_watchdog(int fd)
{
    char msg[MAX_UDP_PG_SIZE] = {0};
    struct sockaddr_in src_addr;
    socklen_t addrlen = sizeof(src_addr);
    ssize_t len = recvfrom(fd, msg, sizeof(msg)-1, MSG_WAITALL, (struct sockaddr *)&src_addr, &addrlen);
    if (len > 0) {
        MITWatchdogPgCmd cmd = wd_get_net_package_cmd(msg);
        MITLog_DetPrintf(MITLOG_LEVEL_COMMON, "Get Server CMD:%d", cmd);
        struct wd_pg_return *ret_pg = wd_pg_return_unpg(msg, (int)len);
        if (ret_pg) {
            if (cmd == WD_PG_CMD_REGISTER) {
                if (ret_pg->error != WD_PG_ERR_SUCCESS) {
                    MITLog_DetPrintf(MITLOG_LEVEL_ERROR, "send rigister package failed, so package should be sent again");
                } else {
                    MITLog_DetPrintf(MITLOG_LEVEL_COMMON, "great! rigister success");
                }
            } else if (cmd == WD_PG_CMD_FEED) {
                MITLog_DetPrintf(MITLOG_LEVEL_COMMON, "Get Server Feed Back");
                if (ret_pg->error != WD_PG_ERR_SUCCESS) {
                    MITLog_DetPrintf(MITLOG_LEVEL_ERROR, "send feed package failed:%d \
                                         and rigister package will resend", ret_pg->error);
                } else {
                    MITLog_DetPrintf(MITLOG_LEVEL_COMMON, "send feed package success");
                }
            } else if (cmd == WD_PG_CMD_UNREGISTER) {
                MITLog_DetPuts(MITLOG_LEVEL_COMMON, "Get Server Unregister Feed Back");
                /* handle unregister */
                if (ret_pg->error != WD_PG_ERR_SUCCESS) {
                    MITLog_DetPrintf(MITLOG_LEVEL_ERROR, "send unregister package failed:%d", ret_pg->error);
                } else {
                    MITLog_DetPrintf(MITLOG_LEVEL_COMMON, "send unregister package success");
                }
            }
            /* call the action interface for next action */
            // recvfrom_watchdog_callback(cmd, ret_pg->error);
            if(ret_pg->error == WD_PG_ERR_SUCCESS) {
            	free(ret_pg);
            	return MIT_RETV_SUCCESS;
            } else {
            	free(ret_pg);
            	return MIT_RETV_FAIL;
            }
        } else {
            MITLog_DetPrintf(MITLOG_LEVEL_ERROR, "wd_pg_return_unpg() failed");
            return MIT_RETV_FAIL;
        }
    } else {
        if(errno == EAGAIN ||
           errno == EWOULDBLOCK) {
               MITLog_DetErrPrintf("recvfrom() failed");
               return MIT_RETV_TIMEOUT;
        }
    }
    return MIT_RETV_FAIL;
}

MITFuncRetValue send_wd_register_package(void)
{
    MITFuncRetValue func_ret = wd_send_register_pg(app_socket_fd);
    if (func_ret == MIT_RETV_SUCCESS) {
        /* wait for the watchdog return package */
        func_ret = recvfrom_watchdog(app_socket_fd);
        if(func_ret != MIT_RETV_SUCCESS) {
        	MITLog_DetPrintf(MITLOG_LEVEL_ERROR, "recvfrom_watchdog() failed:%d", func_ret);
        }
    } else {
    	MITLog_DetPrintf(MITLOG_LEVEL_ERROR, "wd_send_register_pg() failed:%d", func_ret);
    }
    return func_ret;
}

MITFuncRetValue wd_send_action_pg(int fd, MITWatchdogPgCmd cmd, struct sockaddr_in *tar_addr)
{
    if(cmd != WD_PG_CMD_FEED &&
       cmd != WD_PG_CMD_UNREGISTER) {
           MITLog_DetPrintf(MITLOG_LEVEL_ERROR, "params illegal");
           return MIT_RETV_PARAM_ERROR;
    }
    int pg_len = 0;
    void *action_pg = wd_pg_action_new(&pg_len, cmd, feed_configure.monitored_pid);
    if (action_pg) {
        ssize_t ret = sendto(fd, action_pg,
                             (size_t)pg_len, 0,
                             (struct sockaddr *)tar_addr,
                             sizeof(*tar_addr));
        free(action_pg);
        if (ret < 0) {
            MITLog_DetErrPrintf("sendto() failed");
            return MIT_RETV_FAIL;
        }
        return MIT_RETV_SUCCESS;
    } else {
        MITLog_DetPrintf(MITLOG_LEVEL_ERROR, "wd_pg_action_new() failed");
        return MIT_RETV_FAIL;
    }
}

MITFuncRetValue send_wd_feed_package(void)
{
    MITFuncRetValue func_ret = wd_send_action_pg(app_socket_fd, WD_PG_CMD_FEED, &addr_server);
        if (func_ret == MIT_RETV_SUCCESS) {
        /* wait for the watchdog return package */
        func_ret = recvfrom_watchdog(app_socket_fd);
    }
    return func_ret;
}

MITFuncRetValue send_wd_unregister_package(void)
{
    MITFuncRetValue func_ret = wd_send_action_pg(app_socket_fd, WD_PG_CMD_UNREGISTER, &addr_server);
        if (func_ret == MIT_RETV_SUCCESS) {
        /* wait for the watchdog return package */
        func_ret = recvfrom_watchdog(app_socket_fd);
    }
    return func_ret;
}

MITFuncRetValue close_udp_socket(void)
{
    if(close(app_socket_fd)) {
        MITLog_DetErrPrintf("close() failed");
        return MIT_RETV_FAIL;
    }
    return MIT_RETV_SUCCESS;
}





















