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

/*!
 * \brief The RollingFileAppender class extends FileAppender so that the underlying file is rolled over at a user chosen frequency.
 *
 * The class is based on Log4Qt.DailyRollingFileAppender class (http://log4qt.sourceforge.net/)
 * and has the same date pattern format.
 *
 * For example, if the fileName is set to /foo/bar and the DatePattern set to the daily rollover ('.'yyyy-MM-dd'.log'), on 2014-02-16 at midnight,
 * the logging file /foo/bar.log will be copied to /foo/bar.2014-02-16.log and logging for 2014-02-17 will continue in /foo/bar
 * until it rolls over the next day.
 *
 * The logFilesLimit parameter is used to automatically delete the oldest log files in the directory during rollover
 * (so no more than logFilesLimit recent log files exist in the directory at any moment).
 * \sa setDatePattern(DatePattern), setLogFilesLimit(int)
 */

#ifndef ROLLINGFILEAPPENDER_H
#define ROLLINGFILEAPPENDER_H

#include <QDateTime>

#include "FileAppender.h"

DLOG_CORE_BEGIN_NAMESPACE

class LIBDLOG_SHARED_EXPORT RollingFileAppender : public FileAppender
{
public:
  /*!
  @~english
  The enum DatePattern defines constants for date patterns.
  \sa setDatePattern(DatePattern)
   */
  enum DatePattern
  {

    MinutelyRollover = 0, /*!<@~english The minutely date pattern string is "'.'yyyy-MM-dd-hh-mm". */
    HourlyRollover,       /*!<@~english The hourly date pattern string is "yyyy-MM-dd-hh". */
    HalfDailyRollover,    /*!<@~english The half-daily date pattern string is "'.'yyyy-MM-dd-a". */
    DailyRollover,        /*!<@~english The daily date pattern string is "'.'yyyy-MM-dd". */
    WeeklyRollover,       /*!<@~english The weekly date pattern string is "'.'yyyy-ww". */
    MonthlyRollover       /*!<@~english The monthly date pattern string is "'.'yyyy-MM". */
  };

  RollingFileAppender(const QString &fileName = QString());

  DatePattern datePattern() const;
  void setDatePattern(DatePattern datePattern);
  QT_DEPRECATED_X("use setDatePattern(DatePattern)")
  void setDatePattern(const QString &datePattern);

  QT_DEPRECATED_X("use datePattern(DatePattern)")
  QString datePatternString() const;

  void setLogFilesLimit(int limit);
  int logFilesLimit() const;

  void setLogSizeLimit(int limit);
  qint64 logSizeLimit() const;

protected:
  virtual void append(const QDateTime &time, Logger::LogLevel level, const char *file, int line,
                      const char *func, const QString &category, const QString &msg);

private:
  void rollOver();
  void computeRollOverTime();
  void computeFrequency();
  void removeOldFiles();
  void setDatePatternString(const QString &datePatternString);

  QString m_datePatternString;
  DatePattern m_frequency;

  QDateTime m_rollOverTime;
  QString m_rollOverSuffix;
  int m_logFilesLimit;
  qint64 m_logSizeLimit;
  mutable QMutex m_rollingMutex;
};

DLOG_CORE_END_NAMESPACE

#endif // ROLLINGFILEAPPENDER_H
