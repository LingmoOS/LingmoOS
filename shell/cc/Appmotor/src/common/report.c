/***************************************************************************
**
** Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (directui@nokia.com)
**
** This file is part of applauncherd
**
** If you have questions regarding the use of this file, please contact
** Nokia at directui@nokia.com.
**
** This library is free software; you can redistribute it and/or
** modify it under the terms of the GNU Lesser General Public
** License version 2.1 as published by the Free Software Foundation
** and appearing in the file LICENSE.LGPL included in the packaging
** of this file.
**
****************************************************************************/

#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <errno.h>
#include <limits.h>

#include "report.h"

static enum report_output output = report_guess;
static enum report_type level = report_warning;

static const char *progname(void)
{
    static const char *name = NULL;
    if (!name) {
        char buff[PATH_MAX];
        char path[PATH_MAX];
        snprintf(path, sizeof path, "/proc/%d/exe", (int)getpid());
        int n = readlink(path, buff, sizeof buff);
        if (n > 0 && n < (int)sizeof buff) {
            buff[n] = 0;
            /* Note: this is intentionally never released */
            name = strdup(basename(buff));
        }
        if (!name)
            name = "unknown";
    }
    return name;
}

static char *strip(char *str)
{
    if (str) {
        char *dst = str;
        char *src = str;
        while (*src && isspace(*src))
            ++src;
        for (;;) {
            while (*src && !isspace(*src))
                *dst++ = *src++;
            while (*src && isspace(*src))
                ++src;
            if (!*src)
                break;
            *dst++ = ' ';
        }
        *dst = 0;
    }
    return str;
}

extern enum report_type report_get_type(void)
{
    return level;
}

static enum report_type normalize_type(enum report_type type)
{
    if (type < report_minimum)
        return report_minimum;
    if (type > report_maximum)
        return report_maximum;
    return type;
}

extern void report_set_type(enum report_type type)
{
    level = normalize_type(type);
}

enum report_output report_get_output(void)
{
    if (output == report_guess)
        report_set_output(isatty(STDIN_FILENO) ? report_console : report_syslog);
    return output;
}

void report_set_output(enum report_output new_output)
{
    if (output != new_output) {
        if (output == report_syslog)
            closelog();

        output = new_output;

        if (output == report_syslog)
            openlog(PROG_NAME_INVOKER, LOG_PID, LOG_DAEMON);
    }
}

void vreport(enum report_type type, const char *msg, va_list arg)
{
    /* Any errors during logging must not change errno */
    int saved = errno;

    if ((type = normalize_type(type)) > level)
        goto EXIT;

    char *str_type = "";
    switch (type) {
    case report_debug:
        str_type = "debug: ";
        break;
    case report_info:
        str_type = "info: ";
        break;
    case report_notice:
        str_type = "notice: ";
        break;
    case report_warning:
        str_type = "warning: ";
        break;
    case report_error:
        str_type = "error: ";
        break;
    case report_fatal:
        str_type = "died: ";
        break;
    default:
        break;
    }

    char str[400];
    vsnprintf(str, sizeof(str), msg, arg);

    switch (report_get_output()) {
    case report_console:
        fprintf(stderr, "%s: %s%s\n", progname(), str_type, strip(str));
        fflush(stderr);
        break;
    case report_syslog:
        syslog(type, "%s%s", str_type, str);
        break;
    default:
        break;
    }

EXIT:
    errno = saved;
}

void report(enum report_type type, const char *msg, ...)
{
    va_list arg;

    va_start(arg, msg);
    vreport(type, msg, arg);
    va_end(arg);
}

void die(int status, const char *msg, ...)
{
    va_list arg;

    va_start(arg, msg);
    vreport(report_fatal, msg, arg);
    va_end(arg);

    exit(status);
}

