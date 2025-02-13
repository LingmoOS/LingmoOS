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

#ifndef LOGGER_H
#define LOGGER_H

#include "launcherlib.h"
#include <cstdarg>

/*!
 * \class Logger
 * \brief Logging utility class
 *
 */
class DECL_EXPORT Logger
{
public:

    /*!
     * \brief Open the log
     * \param progName Program name as it will be seen in the log.
     */
    static void openLog(const char * progName = 0);

    /*!
     * \brief Close the log
     */
    static void closeLog();

    /*!
     * \brief Log a debug to the system message logger.
     *        Effective only if Logger::setDebugMode(true) called;
     * \param format String identical to a printf format string
     * \param additionalArgs Depending on the format string, the function may expect a 
     *        sequence of additional arguments, each containing one value to be inserted
     *        in the format parameter, if any. 
     */
    static void logDebug(const char * format, ...);

    /*!
     * \brief Log an error to the system message logger
     * \param format String identical to a printf format string
     * \param additionalArgs Depending on the format string, the function may expect a 
     *        sequence of additional arguments, each containing one value to be inserted
     *        in the format parameter, if any. 
     */
    static void logError(const char * format, ...);

    /*!
     * \brief Log a warning to the system message logger
     * \param format String identical to a printf format string
     * \param additionalArgs Depending on the format string, the function may expect a 
     *        sequence of additional arguments, each containing one value to be inserted
     *        in the format parameter, if any. 
     */
    static void logWarning(const char * format, ...);

    /*!
     * \brief Log a piece of information to the system message logger
     * \param format String identical to a printf format string
     * \param additionalArgs Depending on the format string, the function may expect a 
     *        sequence of additional arguments, each containing one value to be inserted
     *        in the format parameter, if any. 
     */
    static void logInfo(const char * format, ...);

    /*!
     * \brief Forces Logger to log everything and echo to stdout if set to true.
     */
    static void setDebugMode(bool enable);

private:

    static void writeLog(const int priority, const char * format, va_list ap); 

    //! True if the log is open
    static bool m_isOpened;

    //! Echo everything including debug messages to stdout if true
    static bool m_debugMode;

#ifdef UNIT_TEST
    friend class Ut_Logger;
#endif
};

// QUARANTINE /* Allow the same logging API to be used in booster and invoker */
// QUARANTINE #define error(  FMT, ARGS...) Logger::logError(  FMT, ##ARGS)
// QUARANTINE #define warning(FMT, ARGS...) Logger::logWarning(FMT, ##ARGS)
// QUARANTINE #define info(   FMT, ARGS...) Logger::logInfo(   FMT, ##ARGS)
// QUARANTINE #define debug(  FMT, ARGS...) Logger::logDebug(  FMT, ##ARGS)

#endif // LOGGER_H

