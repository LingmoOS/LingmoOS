/*
  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU Lesser General Public License version 2.1
  as published by the Free Software Foundation and appearing in the file
  LICENSE.LGPL included in the packaging of this file.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU Lesser General Public License for more details.
*/

#include <QDateTime>
#include <QDir>
#include <QFileInfo>

#include "RollingFileAppender.h"

#include "rollingfilesink_p.h"

DLOG_CORE_BEGIN_NAMESPACE
extern std::string loggerName(const QFile &logFile);
/*!
@~english
  @class Dtk::Core::RollingFileAppender
  @ingroup dlog
  @brief The RollingFileAppender class extends FileAppender so that the underlying file is rolled over at a user chosen frequency.

  The class is based on Log4Qt.DailyRollingFileAppender class (http://log4qt.sourceforge.net/)
  and has the same date pattern format.

  For example, if the fileName is set to /foo/bar and the DatePattern set to the daily rollover ('.'yyyy-MM-dd'.log'), on 2014-02-16 at midnight,
  the logging file /foo/bar.log will be copied to /foo/bar.2014-02-16.log and logging for 2014-02-17 will continue in /foo/bar
  until it rolls over the next day.

  The logFilesLimit parameter is used to automatically delete the oldest log files in the directory during rollover
  (so no more than logFilesLimit recent log files exist in the directory at any moment).
 */

RollingFileAppender::RollingFileAppender(const QString &fileName)
    : FileAppender(fileName)
{
    setLogFilesLimit(1);
    setLogSizeLimit(1024 * 1024 * 20);
}

void RollingFileAppender::append(const QDateTime &time, Logger::LogLevel level, const char *file, int line,
                                 const char *func, const QString &category, const QString &msg)
{
    FileAppender::append(time, level, file, line, func, category, msg);
}

RollingFileAppender::DatePattern RollingFileAppender::datePattern() const
{
    QMutexLocker locker(&m_rollingMutex);
    return m_frequency;
}

QString RollingFileAppender::datePatternString() const
{
    QMutexLocker locker(&m_rollingMutex);
    return m_datePatternString;
}

void RollingFileAppender::setDatePattern(DatePattern datePattern)
{
    QMutexLocker locker(&m_rollingMutex);
    m_frequency = datePattern;

    computeRollOverTime();
}

void RollingFileAppender::setDatePattern(const QString & /*datePattern*/)
{
}

void RollingFileAppender::setDatePatternString(const QString & /*datePatternString*/)
{
}

void RollingFileAppender::computeFrequency()
{
}

void RollingFileAppender::removeOldFiles()
{
}

void RollingFileAppender::computeRollOverTime()
{
    if (auto *fs = get_sink<rolling_file_sink_mt>(loggerName(fileName())))
    {
        return fs->set_interval(RollingInterval(m_frequency));
    }
}

void RollingFileAppender::rollOver()
{
}

void RollingFileAppender::setLogFilesLimit(int limit)
{
    QMutexLocker locker(&m_rollingMutex);
    m_logFilesLimit = limit;

    if (auto *fs = get_sink<rolling_file_sink_mt>(loggerName(fileName())))
    {
        return fs->set_max_files(std::size_t(limit));
    }
}

int RollingFileAppender::logFilesLimit() const
{
    QMutexLocker locker(&m_rollingMutex);
    return m_logFilesLimit;
}

void RollingFileAppender::setLogSizeLimit(int limit)
{
    QMutexLocker locker(&m_rollingMutex);
    m_logSizeLimit = limit;

    if (auto *fs = get_sink<rolling_file_sink_mt>(loggerName(fileName())))
    {
        return fs->set_max_size(std::size_t(limit));
    }
}

qint64 RollingFileAppender::logSizeLimit() const
{
    QMutexLocker locker(&m_rollingMutex);
    return m_logSizeLimit;
}

DLOG_CORE_END_NAMESPACE
