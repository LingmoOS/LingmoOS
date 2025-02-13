/***************************************************************************
**
** Copyright (c) 2010 Nokia Corporation and/or its subsidiary(-ies).
** Copyright (c) 2021 Open Mobile Platform LLC.
** Copyright (c) 2021 Jolla Ltd.
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

#ifndef REPORT_H
#define REPORT_H

#include <syslog.h>
#include <stdarg.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __GNUC__
#define ATTR_NORET __attribute__((noreturn))
#else
#define ATTR_NORET
#endif

enum report_output {
  report_guess,
  report_console,
  report_syslog,
  report_none,
};

enum report_type {
  report_fatal = LOG_CRIT,
  report_error = LOG_ERR,
  report_warning = LOG_WARNING,
  report_notice = LOG_NOTICE,
  report_info = LOG_INFO,
  report_debug = LOG_DEBUG,
  report_default = report_warning,
  report_minimum = report_fatal,
  report_maximum = report_debug,
};

extern enum report_output report_get_output(void);
extern void report_set_output(enum report_output new_output);
extern enum report_type report_get_type(void);
extern void report_set_type(enum report_type type);
extern void report(enum report_type type, const char *msg, ...) __attribute__((format(printf, 2, 3)));
extern void vreport(enum report_type type, const char *msg, va_list arg);
extern void ATTR_NORET die(int status, const char *msg, ...);

#define error(msg, ...) report(report_error, msg, ##__VA_ARGS__)
#define warning(msg, ...) report(report_warning, msg, ##__VA_ARGS__)
#define notice(msg, ...) report(report_notice, msg, ##__VA_ARGS__)
#define info(msg, ...) report(report_info, msg, ##__VA_ARGS__)
#define debug(msg, ...) report(report_debug, msg, ##__VA_ARGS__)

#define HERE warning("%s:%d: %s() ...", __FILE__, __LINE__, __func__);

#ifdef __cplusplus
};
#endif

#endif // REPORT_H
