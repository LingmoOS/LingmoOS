/*
  Copyright (c) 2010 Boris Moiseev (cyberbobs at gmail dot com)

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU Lesser General Public License version 2.1
  as published by the Free Software Foundation and appearing in the file
  LICENSE.LGPL included in the packaging of this file.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU Lesser General Public License for more details.
*/
#ifndef ABSTRACTAPPENDER_H
#define ABSTRACTAPPENDER_H

#include "dlog_global.h"
#include "Logger.h"

#include <QMutex>

DLOG_CORE_BEGIN_NAMESPACE

class LIBDLOG_SHARED_EXPORT AbstractAppender
{
public:
  AbstractAppender();
  virtual ~AbstractAppender();

  Logger::LogLevel detailsLevel() const;
  void setDetailsLevel(Logger::LogLevel level);
  void setDetailsLevel(const QString &level);

  void write(const QDateTime &time, Logger::LogLevel level, const char *file, int line,
             const char *func, const QString &category, const QString &msg);

protected:
  virtual void append(const QDateTime &time, Logger::LogLevel level, const char *file, int line,
                      const char *func, const QString &category, const QString &msg) = 0;

private:
  QMutex m_writeMutex;

  Logger::LogLevel m_detailsLevel;
  mutable QMutex m_detailsLevelMutex;
};

DLOG_CORE_END_NAMESPACE
#endif // ABSTRACTAPPENDER_H
