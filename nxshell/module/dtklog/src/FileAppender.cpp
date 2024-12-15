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

#include "FileAppender.h"

#include <QFileInfo>

#include "rollingfilesink_p.h"

#include <iostream>

DLOG_CORE_BEGIN_NAMESPACE

std::string loggerName(const QFile &logFile)
{
    return QFileInfo(logFile).fileName().toStdString();
}
/*!
@~english
  @class Dtk::Core::FileAppender
  @ingroup dtkcore

  @brief Simple appender that writes the log records to the plain text file.
 */

/*!
@~english
    @brief Constructs the new file appender assigned to file with the given \a fileName.
 */
FileAppender::FileAppender(const QString &fileName)
{
    setFileName(fileName);
}

FileAppender::~FileAppender()
{
    closeFile();
}

/*!
@~english
  \brief Returns the name set by setFileName() or to the FileAppender constructor.

  \sa setFileName()
 */
QString FileAppender::fileName() const
{
    QMutexLocker locker(&m_logFileMutex);
    return m_logFile.fileName();
}

/*!
  \brief Sets the \a s name of the file. The name can have no path, a relative path, or an absolute path.

  \sa fileName()
 */
void FileAppender::setFileName(const QString &s)
{
    QMutexLocker locker(&m_logFileMutex);

    if (s == m_logFile.fileName())
        return;

    closeFile();

    m_logFile.setFileName(s);

    if (!spdlog::get(loggerName(s)))
        rolling_logger_mt(loggerName(s),
                          m_logFile.fileName().toStdString(),
                          1024 * 1024 * 20, 0);
}

qint64 FileAppender::size() const
{
    QMutexLocker locker(&m_logFileMutex);

    if (auto *bs = get_sink<rolling_file_sink_mt>(loggerName(m_logFile)))
    {
        return qint64(bs->filesize());
    }

    return m_logFile.size();
}

bool FileAppender::openFile()
{
    auto fl = spdlog::get(loggerName(m_logFile));

    return fl.get();
}

/*!
@~english
  \brief Write the log record to the file.
  \reimp

  The \a time parameter indicates the time stamp.
  The \a level parameter describes the LogLevel.
  The \a file parameter is the current file name.
  The \a line parameter indicates the number of lines to output.
  The \a func parameter indicates the func name to output.
  The \a category parameter indicates the log category.
  The \a msg parameter indicates the output message.

  \sa fileName()
  \sa AbstractStringAppender::format()
 */
void FileAppender::append(const QDateTime &time, Logger::LogLevel level, const char *file, int line,
                          const char *func, const QString &category, const QString &msg)
{

    if (!openFile())
        return;

    auto fl = spdlog::get(loggerName(m_logFile));
    if (Q_UNLIKELY(!fl))
        return;
    fl->set_level(spdlog::level::level_enum(detailsLevel()));

    const auto &formatted = formattedString(time, level, file, line, func, category, msg);
    fl->log(spdlog::level::level_enum(level), formatted.toStdString());
    fl->flush();
}

void FileAppender::closeFile()
{
    spdlog::drop(loggerName(m_logFile));
}

DLOG_CORE_END_NAMESPACE
