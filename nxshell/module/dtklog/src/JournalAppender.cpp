// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-2.1-or-later
#include "JournalAppender.h"

#define SD_JOURNAL_SUPPRESS_LOCATION
#include <systemd/sd-journal.h>
DLOG_CORE_BEGIN_NAMESPACE

/**
 * @~english
 * @brief it can be used to send log to journal
 * @param time journal time
 * @param level log level
 * @param file
 * @param line Number of lines of code when writing to the log
 * @param func function name
 * @param category log category
 * @param msg log message
 */
void JournalAppender::append(const QDateTime &time,
                             Logger::LogLevel level,
                             const char *file,
                             int line,
                             const char *func,
                             const QString &category,
                             const QString &msg)
{
    JournalPriority logLevel = LOG_INFO;
    switch (level)
    {
    case Logger::Debug:
        logLevel = LOG_DEBUG;
        break;
    case Logger::Info:
        logLevel = LOG_INFO;
        break;
    case Logger::Warning:
        logLevel = LOG_WARNING;
        break;
    case Logger::Error:
        logLevel = LOG_ERR;
        break;
    case Logger::Fatal:
        logLevel = LOG_CRIT;
        break;
    default:
        logLevel = LOG_INFO;
        break;
    }
    sd_journal_send("MESSAGE=%s",
                    msg.toStdString().c_str(),
                    "PRIORITY=%d",
                    logLevel,
                    "DTKPRIORITTY=%d",
                    level,
                    "CODE_FILE=%s",
                    file,
                    "CODE_LINE=%d",
                    line,
                    "CODE_FUNC=%s",
                    func,
                    "CODE_CATEGORY=%s",
                    category.toStdString().c_str(),
                    NULL);
}

DLOG_CORE_END_NAMESPACE
