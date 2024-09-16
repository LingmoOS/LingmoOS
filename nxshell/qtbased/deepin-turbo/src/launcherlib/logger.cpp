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

#include "logger.h"
#include <cstdlib>
#include <syslog.h>
#include <cstdarg>
#include <cstdio>
#include <unistd.h>

#include "coverage.h"

BEGIN_NAMESPACE
bool Logger::m_isOpened  = false;
bool Logger::m_debugMode = false;

void Logger::openLog(const char * progName)
{
    if (!progName)
        progName = "deepin-turbo";

    if (Logger::m_isOpened)
    {
        Logger::closeLog();
    }
    openlog(progName, LOG_PID, LOG_DAEMON);
    Logger::m_isOpened = true;
}

void Logger::closeLog()
{
    if (Logger::m_isOpened)
    {
        // Close syslog
        closelog();
        Logger::m_isOpened = false;
    }
}

void Logger::writeLog(const int priority, const char * format, va_list ap) 
{
    // In debug mode everything is printed also to stdout
    if (m_debugMode)
    {
        vprintf(format, ap);
        printf("\n");
    }

    // Print to syslog
    if (!Logger::m_isOpened)
    {
        Logger::openLog(); //open log with default name
    }
    vsyslog(priority, format, ap);
}

void Logger::logDebug(const char * format, ...)
{
    if (m_debugMode)
    {
        va_list(ap);
        va_start(ap, format);
        writeLog(LOG_DEBUG, format, ap);
        va_end(ap);
    }
}

void Logger::logInfo(const char * format, ...)
{
    va_list(ap);
    va_start(ap, format);
    writeLog(LOG_INFO, format, ap); 
    va_end(ap);
    // To avoid extra file descriptors in forked boosters closing connection to syslog
    Logger::closeLog();
}

void Logger::logWarning(const char * format, ...)
{
    va_list(ap);
    va_start(ap, format);
    writeLog(LOG_WARNING, format, ap);
    va_end(ap);
}

void Logger::logError(const char * format, ...)
{
    va_list(ap);
    va_start(ap, format);
    writeLog(LOG_ERR, format, ap);
    va_end(ap);
}

void Logger::setDebugMode(bool enable)
{
    Logger::m_debugMode = enable;
}
END_NAMESPACE
