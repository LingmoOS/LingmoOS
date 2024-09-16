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
// Local
#include "Logger.h"
#include "AbstractAppender.h"
#include "AbstractStringAppender.h"

#include <QCoreApplication>
#include <QReadWriteLock>
#include <QSemaphore>
#include <QMutex>
#include <QDateTime>
#include <QIODevice>

#include <iostream>
#include <spdlog/spdlog.h>

DLOG_CORE_BEGIN_NAMESPACE

/*!
@~english
  \headerfile <Logger.h>
  \inmodule dtkcore
  @brief A file containing the description of Logger class and and additional useful macros for logging.
 */

/*!
@~english
  \macro Dtk::Core::dlogger
  \relates Dtk::Core::Logger
  \keyword Dtk::Core::dlogger

  @brief Macro returning the current instance of Logger object

  If you haven't created a local Logger object it returns the same value as the Logger::globalInstance() functions.
  This macro is a recommended way to get an access to the Logger instance used in current class.

  Example:
  \code
  ConsoleAppender* consoleAppender = new ConsoleAppender;
  dlogger->registerAppender(consoleAppender);
  \endcode

  @sa Dtk::Core::Logger::globalInstance()
 */

/*!
@~english
  \macro Dtk::Core::dTrace
  \relates Dtk::Core::Logger
  \keyword Dtk::Core::dTrace

  @brief Writes the trace log record

  This macro is the convenient way to call Logger::write(). It uses the common preprocessor macros \c __FILE__,
  \c __LINE__ and the standard Qt \c Q_FUNC_INFO macros to automatically determine the needed parameters to call
  Logger::write().

  @note This and other (dInfo() etc...) macros uses the variadic macro arguments to give convenient usage form for
  the different versions of Logger::write() (using the QString or const char *argument or returning the QDebug class
  instance). Not all compilers will support this. Please, consider reviewing your compiler documentation to ensure
  it support __VA_ARGS__ macro.

  @sa Dtk::Core::dInfo Dtk::Core::dDebug Dtk::Core::dWarning Dtk::Core::dError
  @sa Dtk::Core::Logger::LogLevel
  @sa Dtk::Core::Logger::write()
 */

/*!
@~english
  \macro Dtk::Core::dDebug
  \relates Dtk::Core::Logger
  \keyword Dtk::Core::dDebug

  @brief Writes the debug log record

  This macro records the debug log record using the Logger::write() function. It works similar to the dTrace()
  macro.

  @sa Dtk::Core::dTrace Dtk::Core::dInfo Dtk::Core::dWarning Dtk::Core::dError
  @sa Dtk::Core::Logger::LogLevel
  @sa Dtk::Core::Logger::write()
 */

/*!
@~english
  \macro Dtk::Core::dInfo
  \relates Dtk::Core::Logger
  \keyword Dtk::Core::dInfo

  @brief Writes the info log record

  This macro records the info log record using the Logger::write() function. It works similar to the dTrace()
  macro.

  @sa Dtk::Core::dTrace Dtk::Core::dDebug Dtk::Core::dWarning Dtk::Core::dError
  @sa Dtk::Core::Logger::LogLevel
  @sa Dtk::Core::Logger::write()
 */

/*!
@~english
  \macro Dtk::Core::dWarning
  \relates Dtk::Core::Logger
  \keyword Dtk::Core::dWarning

  @brief Write the warning log record

  This macro records the warning log record using the Logger::write() function. It works similar to the dTrace()
  macro.

  @sa Dtk::Core::dTrace Dtk::Core::dInfo Dtk::Core::dDebug Dtk::Core::dError
  @sa Dtk::Core::Logger::LogLevel
  @sa Dtk::Core::Logger::write()
 */

/*!
@~english
  \macro Dtk::Core::dError
  \relates Dtk::Core::Logger
  \keyword Dtk::Core::dError

  @brief Write the error log record
  This macro records the error log record using the Logger::write() function. It works similar to the dTrace()
  macro.

  @sa Dtk::Core::dTrace
  @sa Dtk::Core::Logger::LogLevel
  @sa Dtk::Core::Logger::write()
 */

/*!
@~english
  \macro Dtk::Core::dFatal
  \relates Dtk::Core::Logger
  \keyword Dtk::Core::dFatal

  @brief Write the fatal log record

  This macro records the fatal log record using the Logger::write() function. It works similar to the dTrace()
  macro.

  @note Recording of the log record using the Logger::Fatal log level will lead to calling the STL abort()
        function, which will interrupt the running of your software and begin the writing of the core dump.

  @sa Dtk::Core::dTrace
  @sa Dtk::Core::Logger::LogLevel
  @sa Dtk::Core::Logger::write()
 */

/*!
@~english
  \macro Dtk::Core::dCTrace(category)
  \relates Dtk::Core::Logger
  \keyword Dtk::Core::dCTrace()

  @brief Writes the trace log record to the specific category

  This macro is the similar to the dTrace() macro, but has a category parameter
  to write only to the category appenders (registered using Logger::registerCategoryAppender() method).

  @param[in] category category name string

  @sa Dtk::Core::dTrace
  @sa Dtk::Core::Logger::LogLevel
  @sa Dtk::Core::Logger::registerCategoryAppender()
  @sa Dtk::Core::Logger::write()
  @sa Dtk::Core::dCategory(), Dtk::Core::dGlobalCategory()
 */

/*!
@~english
  \macro Dtk::Core::dCDebug
  \relates Dtk::Core::Logger
  \keyword Dtk::Core::dCDebug

  @brief Writes the debug log record to the specific category

  This macro records the debug log record using the Logger::write() function. It works similar to the dCTrace()
  macro.

  @sa Dtk::Core::dCTrace()
 */

/*!
@~english
  \macro Dtk::Core::dCInfo
  \relates Dtk::Core::Logger
  \keyword Dtk::Core::dCInfo

  @brief Writes the info log record to the specific category

  This macro records the info log record using the Logger::write() function. It works similar to the dCTrace()
  macro.

  @sa Dtk::Core::dCTrace()
 */

/*!
@~english
  \macro Dtk::Core::dCWarning
  \relates Dtk::Core::Logger
  \keyword Dtk::Core::dCWarning

  @brief Writes the warning log record to the specific category

  This macro records the warning log record using the Logger::write() function. It works similar to the dCTrace()
  macro.

  @sa Dtk::Core::dCTrace()
 */

/*!
@~english
  \macro Dtk::Core::dCError
  \relates Dtk::Core::Logger
  \keyword Dtk::Core::dCError

  @brief Writes the error log record to the specific category

  This macro records the error log record using the Logger::write() function. It works similar to the dCTrace()
  macro.

  @sa Dtk::Core::dCTrace()
 */

/*!
@~english
  \macro Dtk::Core::dCFatal
  \relates Dtk::Core::Logger
  \keyword Dtk::Core::dCFatal

  @brief Write the fatal log record to the specific category

  This macro records the fatal log record using the Logger::write() function. It works similar to the dCTrace()
  macro.

  @note Recording of the log record using the Logger::Fatal log level will lead to calling the STL abort()
        function, which will interrupt the running of your software and begin the writing of the core dump.

  @sa Dtk::Core::dCTrace()
 */

/*!
@~english
  \macro Dtk::Core::dCategory(category)
  \relates Dtk::Core::Logger
  \keyword Dtk::Core::dCategory()

  @brief Create dlogger instance inside your custom class to log all messages to the specified \a category

  This macro is used to pass all log messages inside your custom class to the specific \a category.
  You must include this macro inside your class declaration (similarly to the Q_OBJECT macro).
  Internally, this macro redefines loggerInstance() function, creates the local Logger object inside your class and
  sets the default category to the specified parameter.

  Thus, any call to loggerInstance() (for example, inside dTrace() macro) will return the local Logger object,
  so any logging message will be directed to the default category.

  @note This macro does not register any appender to the newly created dlogger instance. You should register
  dlogger appenders manually, inside your class.

  Usage example:
  \code
  class CustomClass : public QObject
  {
    Q_OBJECT
    dCategory("custom_category")
    ...
  };

  CustomClass::CustomClass(QObject* parent) : QObject(parent)
  {
    dlogger->registerAppender(new FileAppender("custom_category_log"));
    dTrace() << "Trace to the custom category log";
  }
  \endcode

  @sa Dtk::Core::Logger::write()
  @sa Dtk::Core::dTrace()
  @sa Dtk::Core::Logger::registerCategoryAppender()
  @sa Dtk::Core::Logger::setDefaultCategory()
 */

/*!
@~english
  \macro Dtk::Core::dGlobalCategory(category)
  \relates Dtk::Core::Logger
  \keyword Dtk::Core::dGlobalCategory()

  @brief Create dlogger instance inside your custom class to log all messages both to the specified \a category and to
  the global dlogger instance.

  This macro is similar to dCategory(), but also passes all log messages to the global dlogger instance appenders.
  It is equal to defining the local \a category dlogger using dCategory macro and calling:
  \code
  dlogger->logToGlobalInstance(dlogger->defaultCategory(), true);
  \endcode

  @sa Dtk::Core::dCategory
  @sa Dtk::Core::Logger::logToGlobalInstance()
  @sa Dtk::Core::Logger::defaultCategory()
  @sa Dtk::Core::Logger::registerCategoryAppender()
  @sa Dtk::Core::Logger::write()
 */

/*!
@~english
  \macro Dtk::Core::dAssert
  \relates Dtk::Core::Logger
  \keyword Dtk::Core::dAssert

  @brief Check the assertion

  This macro is a convenient and recommended to use way to call Logger::writeAssert() function. It uses the
  preprocessor macros (as the dDebug() does) to fill the necessary arguments of the Logger::writeAssert() call. It
  also uses undocumented but rather mature and stable \c qt_noop() function (which does nothing) when the assertion
  is true.

  Example:
  \code
  bool b = checkSomething();
  ...
  dAssert(b == true);
  \endcode

  @sa Dtk::Core::Logger::writeAssert()
 */

/*!
@~english
  \macro Dtk::Core::dTraceTime
  \relates Dtk::Core::Logger
  \keyword Dtk::Core::dTraceTime

  @brief Logs the processing time of current function / code block

  This macro automagically measures the function or code of block execution time and outputs it as a Logger::Trace
  level log record.

  Example:
  \code
  int foo()
  {
    dTraceTime();
    ... // Do some long operations
    return 0;
  } // Outputs: Function foo finished in <time> ms.
  \endcode

  If you are measuring a code of block execution time you may also add a name of block to the macro:
  \code
  int bar(bool doFoo)
  {
    dTraceTime();

    if (doFoo)
    {
      dTraceTime("Foo");
      ...
    }

    ...
  }
  // Outputs:
  // "Foo" finished in <time1> ms.
  // Function bar finished in <time2> ms.
  \endcode

  @note Macro switches to logging the seconds instead of milliseconds when the execution time reaches 10000 ms.
  @sa Dtk::Core::dDebugTime, Dtk::Core::dInfoTime
 */

/*!
  \macro Dtk::Core::dDebugTime
  \relates Dtk::Core::Logger
  \keyword Dtk::Core::dDebugTime

  @brief Logs the processing time of current function / code block

  This macro automagically measures the function or code of block execution time and outputs it as a Logger::Debug
  level log record. It works similar to dTraceTime() macro.

  @sa Dtk::Core::dTraceTime
 */

/*!
@~english
  \macro Dtk::Core::dInfoTime
  \relates Dtk::Core::Logger
  \keyword Dtk::Core::dInfoTime

  @brief Logs the processing time of current function / code block

  This macro automagically measures the function or code of block execution time and outputs it as a Logger::Info
  level log record. It works similar to dTraceTime() macro.

  @sa Dtk::Core::dTraceTime
 */

/*!
@~english
 \enum Dtk::Core::Logger::LogLevel
 \value Trace
 Can be used for mostly unneeded records used for internal code tracing.
 \value Debug
 Useful for non-necessary records used for the debugging of the software.
 \value Info
 Can be used for informational records, which may be interesting for not only developers.
 \value Warning
 May be used to log some non-fatal warnings detected by your application.
 \value Error
 May be used for a big problems making your application work wrong but not crashing.
 \value Fatal
 Used for unrecoverable errors, crashes the application right after the log record is written.
*/

/*!
@~english
  \class Dtk::Core::Logger
  \inmodule dtkcore

  @brief Very simple but rather powerful component which may be used for logging your application activities.

  Global dlogger instance created on a first access to it (e.g. registering appenders, calling a dDebug() macro
  etc.) registers itself as a Qt default message handler and captures all the qDebug/dWarning/qCritical output.

  @note Qt 4 qDebug set of macro doesn't support capturing source function name, file name or line number so we
        recommend to use dDebug() and other Logger macros instead.

  @sa Dtk::Core::dlogger
 */

class LogDevice : public QIODevice
{
public:
    LogDevice(Logger *l)
        : m_logger(l),
          m_semaphore(1)
    {
    }

    void lock(Logger::LogLevel level, const char *file, int line,
              const char *func, const char *category)
    {
        m_semaphore.acquire();

        if (!isOpen())
            open(QIODevice::WriteOnly);

        m_logLevel = level;
        m_file = file;
        m_line = line;
        m_function = func;
        m_category = category;
    }

protected:
    qint64 readData(char *, qint64)
    {
        return 0;
    }

    qint64 writeData(const char *data, qint64 maxSize)
    {
        if (maxSize > 0)
        {
            const QString &msg = QString::fromLocal8Bit(QByteArray(data, int(maxSize)));
            m_logger->write(m_logLevel, m_file, m_line,
                            m_function, m_category, msg);
        }

        m_semaphore.release();
        return maxSize;
    }

private:
    Logger *m_logger;
    QSemaphore m_semaphore;
    Logger::LogLevel m_logLevel;
    const char *m_file;
    int m_line;
    const char *m_function;
    const char *m_category;
};

/*!
@~english
  \internal

  LoggerPrivate class implements the Singleton pattern in a thread-safe way. It contains a static pointer to the
  global dlogger instance protected by QReadWriteLock
 */
class LoggerPrivate
{
public:
    static Logger *globalInstance;
    static QReadWriteLock globalInstanceLock;

    QList<AbstractAppender *> appenders;
    mutable QMutex loggerMutex;

    QMultiMap<QString, AbstractAppender *> categoryAppenders;
    QString defaultCategory;

    LogDevice *logDevice;
};

Logger *LoggerPrivate::globalInstance = nullptr;
QReadWriteLock LoggerPrivate::globalInstanceLock;

static void cleanupLoggerGlobalInstance()
{
    QWriteLocker locker(&LoggerPrivate::globalInstanceLock);

    delete LoggerPrivate::globalInstance;
    LoggerPrivate::globalInstance = nullptr;
}

#if QT_VERSION >= 0x050000
static void qtLoggerMessageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    Logger::LogLevel level = Logger::Warning;
    switch (type)
    {
    case QtDebugMsg:
        level = Logger::Debug;
        break;
#if QT_VERSION >= 0x050500
    case QtInfoMsg:
        level = Logger::Info;
        break;
#endif
    case QtWarningMsg:
        level = Logger::Warning;
        break;
    case QtCriticalMsg:
        level = Logger::Error;
        break;
    case QtFatalMsg:
        level = Logger::Fatal;
        break;
    }

    bool isDefaultCategory = QString::fromLatin1(context.category) == "default";
    Logger::globalInstance()->write(level, context.file, context.line, context.function,
                                    isDefaultCategory ? nullptr : context.category, msg);
}

#else

static void qtLoggerMessageHandler(QtMsgType type, const char *msg)
{
    switch (type)
    {
    case QtDebugMsg:
        loggerInstance()->write(Logger::Debug, "", 0, "qDebug", 0, msg);
        break;
    case QtWarningMsg:
        loggerInstance()->write(Logger::Warning, "", 0, "qDebug", 0, msg);
        break;
    case QtCriticalMsg:
        loggerInstance()->write(Logger::Error, "", 0, "qDebug", 0, msg);
        break;
    case QtFatalMsg:
        loggerInstance()->write(Logger::Fatal, "", 0, "qDebug", 0, msg);
        break;
    }
}
#endif

/*!
@~english
  @brief Construct the instance of Logger.

  If you're only using one global instance of dlogger you wouldn't probably need to use this constructor manually.
  Consider using [dlogger](@ref dlogger) macro instead to access the dlogger instance
 */
Logger::Logger()
    : d_ptr(new LoggerPrivate)
{
    Q_D(Logger);
    d->logDevice = new LogDevice(this);
}

/*!
@~english
  @brief Construct the instance of Logger and set dlogger default category.

  If you're only using one global instance of dlogger you wouldn't probably need to use this constructor manually.
  Consider using dlogger macro instead to access the dlogger instance and call setDefaultCategory method.

  @sa Logger()
  @sa setDefaultCategory()
 */
Logger::Logger(const QString &defaultCategory)
    : Logger()
{
    setDefaultCategory(defaultCategory);
}

/*!
@~english
  @brief Destroy the instance of Logger.

  You probably wouldn't need to use this function directly. Global instance of dlogger will be destroyed automatically
  at the end of your QCoreApplication execution
 */
Logger::~Logger()
{
    Q_D(Logger);

    QMutexLocker appendersLocker(&d->loggerMutex);

#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
    QSet<AbstractAppender *> appenderList{d->appenders.begin(), d->appenders.end()};
    for (const auto v : d->categoryAppenders.values())
    {
        appenderList.insert(v);
    }
#else
    QSet<AbstractAppender *> appenderList;
    appenderList += d->appenders.toSet() += d->categoryAppenders.values().toSet();
#endif
    qDeleteAll(appenderList);

    delete d->logDevice;
    appendersLocker.unlock();

    delete d_ptr;
    if (!LoggerPrivate::globalInstance ||
        LoggerPrivate::globalInstance == this)
    {
        spdlog::shutdown();
    }
}

/*!
@~english
  @brief Returns the global instance of Logger.

  In a most cases you shouldn't use this function directly. Consider using [dlogger](@ref dlogger) macro instead.

  @sa Dtk::Core::dlogger
 */
Logger *Logger::globalInstance()
{
    Logger *result = nullptr;
    {
        QReadLocker locker(&LoggerPrivate::globalInstanceLock);
        result = LoggerPrivate::globalInstance;
    }

    if (!result)
    {
        QWriteLocker locker(&LoggerPrivate::globalInstanceLock);
        LoggerPrivate::globalInstance = new Logger;

#if QT_VERSION >= 0x050000
        qInstallMessageHandler(qtLoggerMessageHandler);
#else
        qInstallMsgHandler(qtLoggerMessageHandler);
#endif
        qAddPostRoutine(cleanupLoggerGlobalInstance);
        result = LoggerPrivate::globalInstance;

        spdlog::set_automatic_registration(true);
        spdlog::set_pattern("%v");
    }

    return result;
}

/*!
@~english
  @brief Converts the LogLevel enum value to its string representation.

  \a logLevel Log level to convert

  @sa LogLevel
  @sa levelFromString()
 */
QString Logger::levelToString(Logger::LogLevel level)
{
    switch (level)
    {
    case Trace:
        return QLatin1String("Trace");
    case Debug:
        return QLatin1String("Debug");
    case Info:
        return QLatin1String("Info");
    case Warning:
        return QLatin1String("Warning");
    case Error:
        return QLatin1String("Error");
    case Fatal:
        return QLatin1String("Fatal");
    }

    return QString();
}

/*!
@~english
  @brief Converts the LogLevel string representation to enum value.

  Comparation of the strings is case independent. If the log level string provided cannot be understood
  Logger::Debug is returned.

  \a s String to be decoded

  @sa LogLevel
  @sa levelToString()
 */
Logger::LogLevel Logger::levelFromString(const QString &str)
{
    const QString &s = str.trimmed().toLower();

    LogLevel result = Debug;

    if (s == QLatin1String("trace"))
        result = Trace;
    else if (s == QLatin1String("debug"))
        result = Debug;
    else if (s == QLatin1String("info"))
        result = Info;
    else if (s == QLatin1String("warning"))
        result = Warning;
    else if (s == QLatin1String("error"))
        result = Error;
    else if (s == QLatin1String("fatal"))
        result = Fatal;

    return result;
}

/*!
@~english
  @brief Registers the appender to write the log records to.

  On the log writing call (using one of the macros or the write() function) Logger traverses through the list of
  the appenders and writes a log records to the each of them. Please, look through the AbstractAppender
  documentation to understand the concept of appenders.

  If no appenders was added to Logger, it falls back to logging into the \c std::cerr STL stream.

  \a appender Appender to register in the Logger

  @note Logger takes ownership on the appender and it will delete it on the application exit. According to this,
        appenders must be created on heap to prevent double destruction of the appender.

  @sa registerCategoryAppender
  @sa AbstractAppender
 */
void Logger::registerAppender(AbstractAppender *appender)
{
    Q_D(Logger);
    QMutexLocker locker(&d->loggerMutex);

    if (!d->appenders.contains(appender))
        d->appenders.append(appender);
    else
        std::cerr << "Trying to register appender that was already registered" << std::endl;
}

/*!
@~english
  @brief Registers the appender to write the log records to the specific category.

  Calling this method, you can link some appender with the named category.
  On the log writing call to the specific category (calling write() with category parameter directly,
  writing to the default category, or using special dCDebug(), dCWarning() etc. macros),
  Logger writes the log message only to the list of registered category appenders.

  You can call logToGlobalInstance() to pass all category log messages to the global dlogger instance appenders
  (registered using registerAppender()).
  If no category appenders with specific name was registered to the Logger,
  it falls back to logging into the \c std::cerr STL stream, both with simple warning message.

  @param[in] category Category name
  @param[in] appender Appender to register in the Logger

  @note Logger takes ownership on the appender and it will delete it on the application exit. According to this,
        appenders must be created on heap to prevent double destruction of the appender.

  @sa registerAppender
  @sa Dtk::Core::dCTrace(), Dtk::Core::dCDebug(), Dtk::Core::dCInfo(), Dtk::Core::dCWarning(), Dtk::Core::dCError(), Dtk::Core::dCFatal()
  @sa Dtk::Core::dCategory(), Dtk::Core::dGlobalCategory()
  @sa logToGlobalInstance()
  @sa setDefaultCategory()
 */
void Logger::registerCategoryAppender(const QString &category, AbstractAppender *appender)
{
    Q_D(Logger);
    QMutexLocker locker(&d->loggerMutex);

    if (!d->categoryAppenders.contains(category, appender))
        d->categoryAppenders.insert(category, appender);
    else
        std::cerr << "Trying to register category [" << qPrintable(category) << "] appender that was already registered" << std::endl;
}

/*!
@~english
  @brief Links some logging category with the global dlogger instance appenders.

  If set to true, all log messages to the specified category appenders will also be written to the global dlogger instance appenders,
  registered using registerAppender().

  By default, all messages to the specific category are written only to the specific category appenders
  (registered using registerCategoryAppender()).

  @param category Category name
  @param logToGlobal Link or onlink the category from global dlogger instance appender

  @sa globalInstance
  @sa registerAppender
  @sa registerCategoryAppender
 */
void Logger::logToGlobalInstance(const QString &category, bool logToGlobal)
{
    Q_UNUSED(category)
    Q_UNUSED(logToGlobal)
    std::cerr << "DEPRECATED! no longer take effect" << std::endl;
}

/*!
@~english
  @brief Sets default logging category.

  All log messages to this category appenders will also be written to general dlogger instance appenders (registered
  using [registerAppender](@ref registerAppender) method), and vice versa.
  In particular, any calls to the dDebug() macro will be treated as category logging,
  so you needn't to specify category name using dCDebug() macro.

  To unset the default category, pass a null string as a parameter.

  @param[in] category Category name

  @note "category" format marker will be set to the category name for all of these messages
  (see [AbstractStringAppender::setFormat](@ref AbstractStringAppender::setFormat)).

  @sa defaultCategory()
  @sa registerCategoryAppender()
  @sa logToGlobalInstance()
 */
void Logger::setDefaultCategory(const QString &category)
{
    Q_D(Logger);
    QMutexLocker locker(&d->loggerMutex);
    d->defaultCategory = category;
}

/*!
@~english
  @sa setDefaultCategory
  @brief Returns default logging category name
 */
QString Logger::defaultCategory() const
{
    Q_D(const Logger);
    QMutexLocker locker(&d->loggerMutex);
    return d->defaultCategory;
}

/*!
@~english
  @brief Writes the log record.

  Writes the log records with the supplied arguments to all the registered appenders.

  @note It is not recommended to call this function directly. Instead of this you can just call one of the macros
        (dTrace, dTrac, dInfo, dWarning, dError, dFatal) that will supply all the needed
        information to this function.

  @param[in] time - the time stamp of the record
  @param[in] logLevel - the log level of the record
  @param[in] file - the name of the source file that requested the log record
  @param[in] line - the line of the code of source file that requested the log record
  @param[in] function - name of the function that requested the log record
  @param[in] category - logging category (0 for default category)
  @param[in] message - log message

  @note Recording of the log record using the Logger::Fatal log level will lead to calling the STL abort()
        function, which will interrupt the running of your software and begin the writing of the core dump.

  @sa LogLevel
  @sa Dtk::Core::dTrace(), Dtk::Core::dDebug(), Dtk::Core::dInfo(), Dtk::Core::dWarning(), Dtk::Core::dError(), Dtk::Core::dFatal()
  @sa AbstractAppender
 */
void Logger::write(const QDateTime &time, Logger::LogLevel level, const char *file, int line,
                   const char *func, const char *category, const QString &msg)
{
    write(time, level, file, line, func, category, msg, /* fromLocalInstance = */ false);
}

/*!
@~english
  This is the overloaded function provided for the convenience. It behaves similar to the above function.

  This function uses the current timestamp obtained with @sa QDateTime::currentDateTime().

  @sa write()
 */
void Logger::write(Logger::LogLevel level, const char *file, int line,
                   const char *func, const char *category, const QString &msg)
{
    write(QDateTime::currentDateTime(), level, file, line, func, category, msg);
}

/*!
@~english
  This is the overloaded function provided for the convenience. It behaves similar to the above function.

  This function doesn't accept any log message as argument. It returns the \c QDebug object that can be written
  using the stream functions. For example, you may like to write:
  \code
  dDebug() << "This is the size" << size << "of the element" << elementName;
  \endcode
  instead of writing
  \code
  dDebug(QString(QLatin1String("This is the size %1x%2 of the element %3"))
            .arg(size.x()).arg(size.y()).arg(elementName));
  \endcode

  Please consider reading the Qt Reference Documentation for the description of the QDebug class usage syntax.

  @note This overload is definitely more pleasant to use than the first write() overload, but it behaves definitely
        slower than all the above overloads.

  @sa write()
 */
QDebug Logger::write(Logger::LogLevel level, const char *file, int line,
                     const char *func, const char *category)
{
    Q_D(Logger);

    d->logDevice->lock(level, file, line, func, category);
    return QDebug(d->logDevice);
}

/*!
@~english
  @brief Writes the assertion.

  This function writes the assertion record using the write() function.

  The assertion record is always written using the Logger::Fatal log level which leads to the abortation of the
  program and generation of the core dump (if supported).

  The message written to the appenders will be identical to the \a condition argument prefixed with the
  <tt>ASSERT:</tt> notification.

  The @param[in] file parameter is the current file name.
  The @param[in] line parameter indicates the number of lines to output.
  The @param[in] func parameter indicates the function name to output.

  @note It is not recommended to call this function directly. Instead of this you can just call the LOG_ASSERT
        macro that will supply all the needed information to this function.

  @sa write()
 */
void Logger::writeAssert(const char *file, int line,
                         const char *func, const char *condition)
{
    write(Logger::Fatal, file, line, func, nullptr, QString("ASSERT: \"%1\"").arg(condition));
}

void Logger::write(const QDateTime &time, Logger::LogLevel level, const char *file, int line,
                   const char *func, const char *category, const QString &msg, bool fromLocalInstance)
{
    Q_D(Logger);
    QMutexLocker locker(&d->loggerMutex);

    QString logCategory = QString::fromLatin1(category);
    if (logCategory.isNull() && !d->defaultCategory.isNull())
        logCategory = d->defaultCategory;

    bool wasWritten = false;
    bool isGlobalInstance = this == globalInstance();

    if (!logCategory.isNull())
    {
        QList<AbstractAppender *> appenders = d->categoryAppenders.values(logCategory);

        for (AbstractAppender *appender : appenders)
            appender->write(time, level, file, line, func, logCategory, msg);
        if (!appenders.isEmpty())
            wasWritten = true;
    }

    // if we can't find the corresponding category, log will be appended to all Appenders.
    if (!wasWritten)
    {
        if (!d->appenders.isEmpty())
        {
            for (AbstractAppender *appender : d->appenders)
                appender->write(time, level, file, line, func, logCategory, msg);

            wasWritten = true;
        }
        else
        {
            static bool noAppendersWarningShown = false;
            if (!noAppendersWarningShown)
            {
                std::cerr << "No appenders registered with dlogger" << std::endl;
                noAppendersWarningShown = true;
            }
        }
    }

    // local dlogger instances send category messages to the global instance
    if (!logCategory.isNull() && !isGlobalInstance)
        globalInstance()->write(time, level, file, line, func, logCategory.toLatin1(), msg, true);

    if (!wasWritten && !fromLocalInstance)
    {
        QString result = QString(QLatin1String("[%1] <%2> %3")).arg(levelToString(level), -7).arg(AbstractStringAppender::stripFunctionName(func)).arg(msg);
        std::cerr << qPrintable(result) << std::endl;
    }

    if (level == Logger::Fatal)
    {
        std::cerr << "fatal level error occured, the program will abort!";
        abort();
    }
}

Logger *loggerInstance()
{
    return Logger::globalInstance();
}

class DLOG_DECL_DEPRECATED CuteMessageLogger
{
    Q_DISABLE_COPY(CuteMessageLogger)
public:
    Q_DECL_CONSTEXPR CuteMessageLogger(Logger *, Logger::LogLevel,
                                       const char *, int, const char *)
    {
    }

    Q_DECL_CONSTEXPR CuteMessageLogger(Logger *l, Logger::LogLevel, const char *,
                                       int, const char *, const char *)
    {
    }
    void write(const char *msg, ...) const;
    void write(const QString &msg) const;
    QDebug write() const;
};

void CuteMessageLogger::write(const char * /*msg*/, ...) const
{
    write();
}

void CuteMessageLogger::write(const QString & /*msg*/) const
{
    write();
}

QDebug CuteMessageLogger::write() const
{
    return QDebug(QtWarningMsg) << "DEPRECATED! rebuild your application with lastest DtkCore";
}

DLOG_CORE_END_NAMESPACE
#include "Logger.moc"
