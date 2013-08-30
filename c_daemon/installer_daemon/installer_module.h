//
//  installer_module.h
//
//  Created by gtliu on 7/19/13.
//  Copyright (c) 2013 GT. All rights reserved.
//

#ifndef INSTALLER_DAEMON_H
#define INSTALLER_DAEMON_H

#include "../include/mit_log_module.h"
#include "../include/mit_data_define.h"
#include <sys/types.h>
#define APP_NAME_INSTALLERD         "installer_daemon"
#define VERSION_INSTALLERD          "v1.0.1"

typedef enum WDConfirmedUnregFlag {
    WD_CONF_REG_NOT_CONF        = 0,
    WD_CONF_REG_HAS_CONF        = 1
} WDConfirmedUnregFlag;


#endif
