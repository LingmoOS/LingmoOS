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

#ifndef REPORT_H
#define REPORT_H

#ifdef __GNUC__
#define ATTR_NORET __attribute__((noreturn))
#else
#define ATTR_NORET
#endif

#ifdef __cplusplus
extern "C" {
#endif

enum report_output {
  report_console,
  report_syslog,
  report_none
};

enum report_type {
  report_debug,
  report_info,
  report_warning,
  report_error,
  report_fatal
};

extern void report_set_output(enum report_output new_output);
extern void report(enum report_type type, const char *msg, ...);

#ifndef DEBUG_LOGGING_DISABLED
#define debug(msg, ...) report(report_debug, msg, ##__VA_ARGS__)
#else
#define debug(...)
#endif

#define info(msg, ...) report(report_info, msg, ##__VA_ARGS__)
#define warning(msg, ...) report(report_warning, msg, ##__VA_ARGS__)
#define error(msg, ...) report(report_error, msg, ##__VA_ARGS__)

extern void ATTR_NORET die(int status, const char *msg, ...);

#ifdef __cplusplus
}
#endif

#endif

