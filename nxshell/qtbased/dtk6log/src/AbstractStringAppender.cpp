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

#include "AbstractStringAppender.h"

#include <QRegularExpression>
#include <QCoreApplication>
#include <QThread>

DLOG_CORE_BEGIN_NAMESPACE

inline static QString formattedLevelWithColor(Logger::LogLevel level, QString &msg)
{
    switch (level)
    {
    case Logger::Trace:
        return QString("\x1b[94m%1\x1b[0m").arg(msg);
    case Logger::Debug:
        return QString("\x1b[36m%1\x1b[0m").arg(msg);
    case Logger::Info:
        return QString("\x1b[32m%1\x1b[0m").arg(msg);
    case Logger::Warning:
        return QString("\x1b[33m%1\x1b[0m").arg(msg);
    case Logger::Error:
        return QString("\x1b[31m%1\x1b[0m").arg(msg);
    case Logger::Fatal:
        return QString("\x1b[35m%1\x1b[0m").arg(msg);
    default:
        break;
    }
    return msg;
}

/*!
@~english
  \class Dtk::Core::AbstractStringAppender
  \inmodule dtkcore

  \brief The AbstractStringAppender class provides a convenient base for appenders working with plain text formatted
         logs.

  AbstractSringAppender is the simple extension of the AbstractAppender class providing the convenient way to create
  custom log appenders working with a plain text formatted log targets.

  It have the formattedString() protected function that formats the logging arguments according to a format set with
  setFormat().

  This class can not be directly instantiated because it contains pure virtual function inherited from AbstractAppender
  class.

  For more detailed description of customizing the log output format see the documentation on the setFormat() function.
 */

const char formattingMarker = '%';

/*!
@~english
    \brief Constructs a new string appender object.
 */
AbstractStringAppender::AbstractStringAppender()
    : m_format(QLatin1String("%{time}{yyyy-MM-ddTHH:mm:ss.zzz} [%{type:-7}] <%{function}> %{message}\n"))
{
}

/*!
@~english
  \brief Returns the current log format string.

  The default format is set to "%{time}{yyyy-MM-ddTHH:mm:ss.zzz} [%{type:-7}] <%{function}> %{message}\n". You can set a different log record
  format using the setFormat() function.

  \sa setFormat(const QString&)
 */
QString AbstractStringAppender::format() const
{
    QReadLocker locker(&m_formatLock);
    return m_format;
}

/*!
@~english
  \brief Sets the logging format for writing strings to the log target with this appender.

  The string format seems to be very common to those developers who have used a standard sprintf function.

  Log output format is a simple QString with the special markers (starting with % sign) which will be replaced with
  it's internal meaning when writing a log record.

  Controlling marker begins with the percent sign (%) which is followed by the command inside {} brackets
  (the command describes, what will be put to log record instead of marker).
  Optional field width argument may be specified right after the command (through the colon symbol before the closing bracket)
  Some commands requires an additional formatting argument (in the second {} brackets).

  Field width argument works almost identically to the QString::arg() fieldWidth argument (and uses it
  internally). For example, "%{type:-7}" will be replaced with the left padded debug level of the message
  ("Debug  ") or something. For the more detailed description of it you may consider to look to the Qt
  Reference Documentation.

  Supported marker commands are:
    \list
    \li %{time} - timestamp. You may specify your custom timestamp \a format using the second {} brackets after the marker,
    \li   timestamp \a format here will be similar to those used in QDateTime::toString() function. For example,
    \li   "%{time}{dd-MM-yyyy, HH:mm}" may be replaced with "17-12-2010, 20:17" depending on current date and time.
    \li   The default \a format used here is "HH:mm:ss.zzz".
    \li %{type} - Log level. Possible log levels are shown in the Logger::LogLevel enumerator.
    \li %{Type} - Uppercased log level.
    \li %{typeOne} - One letter log level.
    \li %{TypeOne} - One uppercase letter log level.
    \li %{File} - Full source file name (with path) of the file that requested log recording. Uses the __FILE__ preprocessor macro.
    \li %{file} - Short file name (with stripped path).
    \li %{line} - Line number in the source file. Uses the __LINE__ preprocessor macro.
    \li %{Function} - Name of function that called on of the LOG_* macros. Uses the Q_FUNC_INFO macro provided with Qt.
    \li %{function} - Similar to the %{Function}, but the function name is stripped using stripFunctionName
    \li %{message} - The log message sent by the caller.
    \li %{category} - The log category.
    \li %{appname} - Application name (returned by QCoreApplication::applicationName() function).
    \li %{pid} - Application pid (returned by QCoreApplication::applicationPid() function).
    \li %{threadid} - ID of current thread.
    \li %% - Convinient marker that is replaced with the single % mark.
    \endlist

  \note Format doesn't add '\\n' to the end of the \a format line. Please consider adding it manually.

  \sa format()
  \sa stripFunctionName()
  \sa Logger::LogLevel
 */
void AbstractStringAppender::setFormat(const QString &format)
{
    QWriteLocker locker(&m_formatLock);
    m_format = format;
}

/*!
@~english
  \brief Strips the long function signature (as added by Q_FUNC_INFO macro).

  The string processing drops the returning type, arguments and template parameters of function. It is definitely
  useful for enchancing the log output readability.

  The \a name parameter is the function name.

  \return stripped function name
 */
QString AbstractStringAppender::stripFunctionName(const char *name)
{
    return QString::fromLatin1(qCleanupFuncinfo(name));
}

// The function was backported from Qt5 sources (qlogging.h)
QByteArray AbstractStringAppender::qCleanupFuncinfo(const char *name)
{
    QByteArray info(name);

    // Strip the function info down to the base function name
    // note that this throws away the template definitions,
    // the parameter types (overloads) and any const/volatile qualifiers.
    if (info.isEmpty())
        return info;

    int pos;

    // skip trailing [with XXX] for templates (gcc)
    pos = info.size() - 1;
    if (info.endsWith(']'))
    {
        while (--pos)
        {
            if (info.at(pos) == '[')
                info.truncate(pos);
        }
    }

    bool hasLambda = false;
    QRegularExpression lambdaRegex("::<lambda\\(.*\\)>");
    QRegularExpressionMatch match = lambdaRegex.match(QString::fromLatin1(info));
    if (match.hasMatch())
    {
        hasLambda = true;
        info.remove(match.capturedStart(), match.capturedLength());
    }

    // operator names with '(', ')', '<', '>' in it
    static const char operator_call[] = "operator()";
    static const char operator_lessThan[] = "operator<";
    static const char operator_greaterThan[] = "operator>";
    static const char operator_lessThanEqual[] = "operator<=";
    static const char operator_greaterThanEqual[] = "operator>=";

    // canonize operator names
    info.replace("operator ", "operator");

    // remove argument list
    Q_FOREVER
    {
        int parencount = 0;
        pos = info.lastIndexOf(')');
        if (pos == -1)
        {
            // Don't know how to parse this function name
            return info;
        }

        // find the beginning of the argument list
        --pos;
        ++parencount;
        while (pos && parencount)
        {
            if (info.at(pos) == ')')
                ++parencount;
            else if (info.at(pos) == '(')
                --parencount;
            --pos;
        }
        if (parencount != 0)
            return info;

        info.truncate(++pos);

        if (info.at(pos - 1) == ')')
        {
            if (info.indexOf(operator_call) == pos - (int)strlen(operator_call))
                break;

            // this function returns a pointer to a function
            // and we matched the arguments of the return type's parameter list
            // try again
            info.remove(0, info.indexOf('('));
            info.chop(1);
            continue;
        }
        else
        {
            break;
        }
    }

    if (hasLambda)
        info.append("::lambda");

    // find the beginning of the function name
    int parencount = 0;
    int templatecount = 0;
    --pos;

    // make sure special characters in operator names are kept
    if (pos > -1)
    {
        switch (info.at(pos))
        {
        case ')':
            if (info.indexOf(operator_call) == pos - (int)strlen(operator_call) + 1)
                pos -= 2;
            break;
        case '<':
            if (info.indexOf(operator_lessThan) == pos - (int)strlen(operator_lessThan) + 1)
                --pos;
            break;
        case '>':
            if (info.indexOf(operator_greaterThan) == pos - (int)strlen(operator_greaterThan) + 1)
                --pos;
            break;
        case '=':
        {
            int operatorLength = (int)strlen(operator_lessThanEqual);
            if (info.indexOf(operator_lessThanEqual) == pos - operatorLength + 1)
                pos -= 2;
            else if (info.indexOf(operator_greaterThanEqual) == pos - operatorLength + 1)
                pos -= 2;
            break;
        }
        default:
            break;
        }
    }

    while (pos > -1)
    {
        if (parencount < 0 || templatecount < 0)
            return info;

        char c = info.at(pos);
        if (c == ')')
            ++parencount;
        else if (c == '(')
            --parencount;
        else if (c == '>')
            ++templatecount;
        else if (c == '<')
            --templatecount;
        else if (c == ' ' && templatecount == 0 && parencount == 0)
            break;

        --pos;
    }
    info = info.mid(pos + 1);

    // remove trailing '*', '&' that are part of the return argument
    while ((info.at(0) == '*') || (info.at(0) == '&'))
        info = info.mid(1);

    // we have the full function name now.
    // clean up the templates
    while ((pos = info.lastIndexOf('>')) != -1)
    {
        if (!info.contains('<'))
            break;

        // find the matching close
        int end = pos;
        templatecount = 1;
        --pos;
        while (pos && templatecount)
        {
            const char c = info.at(pos);
            if (c == '>')
                ++templatecount;
            else if (c == '<')
                --templatecount;
            --pos;
        }
        ++pos;
        info.remove(pos, end - pos + 1);
    }

    return info;
}

QString AbstractStringAppender::formattedString(const QDateTime &time, Logger::LogLevel level,
                                                const char *file, int line, const char *func,
                                                const QString &category, const QString &msg) const
{
    return formattedString(time, level, file, line, func, category, msg, false);
}

/*!
@~english
  \brief Returns the string to record to the logging target, formatted according to the format().

  \a time The time stamp.
  The \a level parameter describes the LogLevel, and the \a file parameter is the current file name,
  and the \a line parameter indicates the number of lines to output.
  The \a func parameter indicates the function name to output.
  The \a category parameter indicates the log category.
  The \a msg parameter indicates the output message.
  The \a withcolor parameter indicates wether to add color to output

  \sa format()
  \sa setFormat(const QString&)
 */
QString AbstractStringAppender::formattedString(const QDateTime &time, Logger::LogLevel level,
                                                const char *file, int line, const char *func,
                                                const QString &category, const QString &msg,
                                                bool withcolor) const
{
    QString f = format();

    // dtkcore无法正确解析Qt的日志格式，dtk默认的日志格式并未和Qt统一，解析方式需要兼容两种不同的格式。
    if (f.contains(QLatin1String("time ")))
        f.replace(f.indexOf(' ', f.indexOf(QLatin1String("time")) + QLatin1String("time").size()), 1, QLatin1String("}{"));

    const int size = f.size();

    QString result;

    int i = 0;
    while (i < f.size())
    {
        QChar c = f.at(i);

        // We will silently ignore the broken % marker at the end of string
        if (c != QLatin1Char(formattingMarker) || (i + 2) >= size)
        {
            result.append(c);
        }
        else
        {
            i += 2;
            QChar currentChar = f.at(i);
            QString command;
            int fieldWidth = 0;

            if (currentChar.isLetter())
            {
                command.append(currentChar);
                int j = 1;
                while ((i + j) < size && f.at(i + j).isLetter())
                {
                    command.append(f.at(i + j));
                    j++;
                }

                i += j;
                currentChar = f.at(i);

                // Check for the padding instruction
                if (currentChar == QLatin1Char(':'))
                {
                    currentChar = f.at(++i);
                    if (currentChar.isDigit() || currentChar.category() == QChar::Punctuation_Dash)
                    {
                        int j = 1;
                        while ((i + j) < size && f.at(i + j).isDigit())
                            j++;

                        fieldWidth = f.mid(i, j).toInt();
                        i += j;
                    }
                }
            }

            // Log record chunk to insert instead of formatting instruction
            QString chunk;

            // Time stamp
            if (command == QLatin1String("time"))
            {
                if (f.at(i + 1) == QLatin1Char('{'))
                {
                    int j = 1;
                    while ((i + 2 + j) < size && f.at(i + 2 + j) != QLatin1Char('}'))
                        j++;

                    if ((i + 2 + j) < size)
                    {
                        chunk = time.toString(f.mid(i + 2, j));

                        i += j;
                        i += 2;
                    }
                }

                if (chunk.isNull())
                    chunk = time.toString(QLatin1String("HH:mm:ss.zzz"));
            }
            else if (command == QLatin1String("type"))
            {
                // Log level
                chunk = Logger::levelToString(level);
                if (withcolor)
                {
                    chunk = formattedLevelWithColor(level, chunk);
                }
            }
            else if (command == QLatin1String("Type"))
            {
                // Uppercased log level
                chunk = Logger::levelToString(level).toUpper();
                if (withcolor)
                {
                    chunk = formattedLevelWithColor(level, chunk);
                }
            }
            else if (command == QLatin1String("typeOne"))
            {
                // One letter log level
                chunk = Logger::levelToString(level).left(1).toLower();
                if (withcolor)
                {
                    chunk = formattedLevelWithColor(level, chunk);
                }
            }
            else if (command == QLatin1String("TypeOne"))
            {
                // One uppercase letter log level
                chunk = Logger::levelToString(level).left(1).toUpper();
                if (withcolor)
                {
                    chunk = formattedLevelWithColor(level, chunk);
                }
            }
            else if (command == QLatin1String("File"))
            {
                // Filename
                chunk = QLatin1String(file);
            }
            else if (command == QLatin1String("file"))
            {
                // Filename without a path
                chunk = QString(QLatin1String(file)).section('/', -1);
            }
            else if (command == QLatin1String("line"))
            {
                // Source line number
                chunk = QString::number(line);
            }
            else if (command == QLatin1String("Function"))
            {
                // Function name, as returned by Q_FUNC_INFO
                chunk = QString::fromLatin1(func);
            }
            else if (command == QLatin1String("function"))
            {
                // Stripped function name
                chunk = stripFunctionName(func);
            }
            else if (command == QLatin1String("message"))
            {
                // Log message
                chunk = msg;
            }
            else if (command == QLatin1String("category"))
            {
                // Log message
                chunk = category;
            }
            else if (command == QLatin1String("pid"))
            {
                // Application pid
                chunk = QString::number(QCoreApplication::applicationPid());
            }
            else if (command == QLatin1String("appname"))
            {
                // Application name
                chunk = QCoreApplication::applicationName();
            }
            else if (command == QLatin1String("threadid"))
            {
                // Thread ID (duplicates Qt5 threadid debbuging way)
                chunk = QLatin1String("0x") + QString::number(qlonglong(QThread::currentThread()->currentThread()), 16);
            }
            else if (command == QString(formattingMarker))
            {
                // We simply replace the double formatting marker (%) with one
                chunk = QLatin1Char(formattingMarker);
            }
            else
            {
                // Do not process any unknown commands
                chunk = QString(formattingMarker);
                chunk.append(command);
            }

            if (!chunk.isEmpty() && chunk != "0")
            {
                result.append(QString(QLatin1String("%1")).arg(chunk, fieldWidth));
            }
        }
        ++i;
    }

    return result;
}

DLOG_CORE_END_NAMESPACE
