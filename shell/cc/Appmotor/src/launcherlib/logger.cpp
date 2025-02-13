/***************************************************************************
**
** Copyright (c) 2010 Nokia Corporation and/or its subsidiary(-ies).
** Copyright (c) 2013 - 2021 Jolla Ltd.
** Copyright (c) 2021 Open Mobile Platform LLC.
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

#include "logger.h"
#include <cstdlib>
#include <syslog.h>
#include <cstdarg>
#include <cstdio>
#include <unistd.h>
#include <ctype.h>

#include "coverage.h"
#include "report.h"

bool Logger::m_isOpened  = false;
bool Logger::m_debugMode = false;

static bool useSyslog()
{
    return report_get_output() == report_syslog;
}

void Logger::openLog(const char * progName)
{
    if (!progName)
        progName = "mapplauncherd";

    if (useSyslog()) {
        if (Logger::m_isOpened)
            Logger::closeLog();
        openlog(progName, LOG_PID, LOG_DAEMON);
        Logger::m_isOpened = true;
    }
}

void Logger::closeLog()
{
    if (useSyslog()) {
        if (Logger::m_isOpened)
            closelog();
        Logger::m_isOpened = false;
    }
}

void Logger::writeLog(const int priority, const char *format, va_list va)
{
    vreport((enum report_type)priority, format, va);
}

void Logger::logDebug(const char * format, ...)
{
    va_list va;
    va_start(va, format);
    Logger::writeLog(report_debug, format, va);
    va_end(va);
}

void Logger::logInfo(const char * format, ...)
{
    va_list va;
    va_start(va, format);
    Logger::writeLog(report_info, format, va);
    va_end(va);
}

void Logger::logWarning(const char * format, ...)
{
    va_list va;
    va_start(va, format);
    Logger::writeLog(report_warning, format, va);
    va_end(va);
}

void Logger::logError(const char * format, ...)
{
    va_list va;
    va_start(va, format);
    Logger::writeLog(report_error, format, va);
    va_end(va);
}

void Logger::setDebugMode(bool enable)
{
    if ((Logger::m_debugMode = enable))
        report_set_type(report_debug);
    else
        report_set_type(report_warning);
}

