#ifndef WATCHDOG_COMM_H
#define WATCHDOG_COMM_H

#include "../include/mit_log_module.h"
#include "../include/mit_data_define.h"
#include <sys/types.h>

/**
 * Recieve App's infomation used for feeding watchdog
 * and save pid and version into config files.
 *
 */
 MITFuncRetValue save_appinfo_config(pid_t monitored_pid,
                                     unsigned long feed_period,
                                     const char *app_name,
                                     const char *cmd_line,
                                     const char *version_str);

/**
 * Initialize the UDP socket.
 */
MITFuncRetValue init_udp_socket(void);

/**
 * The register package will be sent.
 */
MITFuncRetValue send_wd_register_package(void);

/**
 * After register to watchdog, you should call this function
 * to send feed package periodically.
 */
MITFuncRetValue send_wd_feed_package(void);

/**
 * Send the un-register package to the watchdog.
 */
MITFuncRetValue send_wd_unregister_package(void);

/**
 * Close the UDP socket.
 */
MITFuncRetValue close_udp_socket(void);

#endif
