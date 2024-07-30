/*
    SPDX-FileCopyrightText: 2015 Samuel Gaist <samuel.gaist@edeltech.ch

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "pam_darwin.h"

#include <stdlib.h>
#include <stdio.h>
#include <sys/syslog.h>

#include <security/pam_modules.h>
#include <security/pam_appl.h>

void pam_vsyslog(const pam_handle_t *ph, int priority, const char *fmt, va_list args)
{
    char *msg = NULL;
    const char *service = NULL;
    int retval;

    retval = pam_get_item(ph, PAM_SERVICE, (const void **) &service);
    if (retval != PAM_SUCCESS)
        service = NULL;
    if (vasprintf(&msg, fmt, args) < 0) {
        syslog(LOG_CRIT | LOG_AUTHPRIV, "cannot allocate memory in vasprintf: %m");
        return;
    }
    syslog(priority | LOG_AUTHPRIV, "%s%s%s: %s",
           (service == NULL) ? "" : "(",
           (service == NULL) ? "" : service,
           (service == NULL) ? "" : ")", msg);
    free(msg);
}

void pam_syslog(const pam_handle_t *ph, int priority, const char *fmt, ...)
{
    va_list args;

    va_start(args, fmt);
    pam_vsyslog(ph, priority, fmt, args);
    va_end(args);
}
