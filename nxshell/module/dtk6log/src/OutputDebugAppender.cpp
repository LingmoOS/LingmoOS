/*
  Copyright (c) 2010 Karl-Heinz Reichel (khreichel at googlemail dot com)

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU Lesser General Public License version 2.1
  as published by the Free Software Foundation and appearing in the file
  LICENSE.LGPL included in the packaging of this file.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU Lesser General Public License for more details.
*/
// Local
#include "win32/OutputDebugAppender.h"

#include <spdlog/sinks/msvc_sink.h>
#include <spdlog/spdlog.h>

// STL
#include <windows.h>

DLOG_CORE_BEGIN_NAMESPACE

/*!
  \class Dtk::Core::OutputDebugAppender
  \inmodule dtkcore

  \brief Appender that writes the log records to the Microsoft Debug Log.
 */

/*!
  \brief Writes the log record to the windows debug log.
  \reimp

  \brief Writes the log record to the windows debug log.

  The \a time parameter indicates the time stamp.
  The \a level parameter describes the LogLevel.
  The \a file parameter is the current file name.
  The \a line parameter indicates the number of lines to output.
  The \a func parameter indicates the function name to output.
  The \a category parameter indicates the log category.
  The \a msg parameter indicates the output message.

  \sa AbstractStringAppender::format()
 */
void OutputDebugAppender::append(const QDateTime &time,
                                 Logger::LogLevel level,
                                 const char *file,
                                 int line,
                                 const char *func,
                                 const QString &category,
                                 const QString &msg)
{
  auto msvclogger = spdlog::get("msvc");
  if (!msvclogger)
    msvclogger = spdlog::create<spdlog::sinks::msvc_sink_mt>("msvc", true);

  Q_ASSERT(msvclogger);

  msvclogger->set_level(spdlog::level::level_enum(detailsLevel()));
  const auto &formatted = formattedString(time, level, file, line, func, category, msg);
  msvclogger->log(spdlog::level::level_enum(level), formatted.toStdString());
  msvclogger->flush();
}

DLOG_CORE_END_NAMESPACE
