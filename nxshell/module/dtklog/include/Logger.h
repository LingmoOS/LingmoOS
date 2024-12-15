/*
  Copyright (c) 2012 Boris Moiseev (cyberbobs at gmail dot com)

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU Lesser General Public License version 2.1
  as published by the Free Software Foundation and appearing in the file
  LICENSE.LGPL included in the packaging of this file.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU Lesser General Public License for more details.
*/
#ifndef LOGGER_H
#define LOGGER_H

#include <QString>
#include <QDebug>
#include <QDateTime>

#include "dloggerdefs.h"

DLOG_CORE_BEGIN_NAMESPACE

class AbstractAppender;
class LoggerPrivate;
class LIBDLOG_SHARED_EXPORT Logger
{
  Q_DISABLE_COPY(Logger)
public:
  //!@~english the log levels
  enum LogLevel
  {
    Trace,   //!<@~english Trace level. Can be used for mostly unneeded records used for internal code tracing.
    Debug,   //!<@~english Debug level.for the debugging of the software.
    Info,    //!<@~english Info level. Can be used for informational records, which may be interesting for not only developers.
    Warning, //!<@~english Warning. May be used to log some non-fatal warnings detected by your application.
    Error,   //!<@~english May be used for a big problems making your application work wrong but not crashing.
    Fatal    //!<@~english Fatal. Used for unrecoverable errors, crashes the application (abort) right after the log record is written.
  };

  Logger();
  Logger(const QString &defaultCategory);
  ~Logger();

  static Logger *globalInstance();

  static QString levelToString(LogLevel level);
  static LogLevel levelFromString(const QString &str);

  void registerAppender(AbstractAppender *appender);
  void registerCategoryAppender(const QString &category, AbstractAppender *appender);

  QT_DEPRECATED_X("no longer take effect")
  void logToGlobalInstance(const QString &category, bool logToGlobal = false);

  void setDefaultCategory(const QString &category);
  QString defaultCategory() const;

  void write(const QDateTime &time, LogLevel level, const char *file, int line,
             const char *func, const char *category, const QString &msg);
  void write(LogLevel level, const char *file, int line,
             const char *func, const char *category, const QString &msg);
  QDebug write(LogLevel level, const char *file, int line,
               const char *func, const char *category);
  void writeAssert(const char *file, int line,
                   const char *func, const char *condition);

private:
  void write(const QDateTime &time, LogLevel level, const char *file, int line,
             const char *func, const char *category,
             const QString &msg, bool fromLocalInstance);
  Q_DECLARE_PRIVATE(Logger)
  LoggerPrivate *d_ptr;
};

DLOG_CORE_END_NAMESPACE
#endif // LOGGER_H
