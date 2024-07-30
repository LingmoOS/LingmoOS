/*
    SPDX-FileCopyrightText: 2009 George Kiagiadakis <gkiagia@users.sourceforge.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include <config-drkonqi.h>

#include "crashedapplication.h"

#if HAVE_STRSIGNAL && defined(Q_OS_UNIX)
#include <clocale>
#include <cstdlib>
#include <cstring>
#else
#if defined(Q_OS_UNIX)
#include <signal.h>
#endif
#endif

#include <QDir>

#include <KIO/CommandLauncherJob>

#include "drkonqi_debug.h"

CrashedApplication::CrashedApplication(int pid,
                                       int thread,
                                       int signalNumber,
                                       const QFileInfo &executable,
                                       const QString &version,
                                       const BugReportAddress &reportAddress,
                                       const QString &name,
                                       const QString &productName,
                                       const QDateTime &datetime,
                                       bool restarted,
                                       bool hasDeletedFiles,
                                       bool applicationNotResponding,
                                       const QString &fakeBaseName,
                                       QObject *parent)

    : QObject(parent)
    , m_pid(pid)
    , m_signalNumber(signalNumber)
    , m_name(name)
    , m_executable(executable)
    , m_fakeBaseName(fakeBaseName)
    , m_version(version)
    , m_reportAddress(reportAddress)
    , m_productName(productName)
    , m_restarted(restarted)
    , m_thread(thread)
    , m_datetime(datetime)
    , m_hasDeletedFiles(hasDeletedFiles)
    , m_applicationNotResponding(applicationNotResponding)
{
}

CrashedApplication::~CrashedApplication()
{
    if (!m_coreFile.isEmpty()) {
        const auto path = QFileInfo(m_coreFile).path();
        if (!path.isEmpty() && QFile::exists(path)) {
            qCDebug(DRKONQI_LOG) << "Cleaning up" << path;
            QDir(path).removeRecursively();
        }
    }
};

QString CrashedApplication::name() const
{
    return m_name.isEmpty() ? fakeExecutableBaseName() : m_name;
}

QFileInfo CrashedApplication::executable() const
{
    return m_executable;
}

QString CrashedApplication::fakeExecutableBaseName() const
{
    if (!m_fakeBaseName.isEmpty()) {
        return m_fakeBaseName;
    } else {
        return m_executable.baseName();
    }
}

QString CrashedApplication::version() const
{
    return m_version;
}

BugReportAddress CrashedApplication::bugReportAddress() const
{
    return m_reportAddress;
}

QString CrashedApplication::productName() const
{
    return m_productName;
}

int CrashedApplication::pid() const
{
    return m_pid;
}

int CrashedApplication::signalNumber() const
{
    return m_signalNumber;
}

QString CrashedApplication::signalName() const
{
#if HAVE_STRSIGNAL && defined(Q_OS_UNIX)
    const QByteArray originalLocale(std::setlocale(LC_MESSAGES, nullptr));
    std::setlocale(LC_MESSAGES, "C");
    const char *name = strsignal(m_signalNumber);
    std::setlocale(LC_MESSAGES, originalLocale.data() /* empty string if we got a nullptr */);
    return QString::fromLocal8Bit(name ? name : "Unknown");
#else
    switch (m_signalNumber) {
#if defined(Q_OS_UNIX)
    case SIGILL:
        return QLatin1String("SIGILL");
    case SIGABRT:
        return QLatin1String("SIGABRT");
    case SIGFPE:
        return QLatin1String("SIGFPE");
    case SIGSEGV:
        return QLatin1String("SIGSEGV");
    case SIGBUS:
        return QLatin1String("SIGBUS");
#else
    case EXCEPTION_ACCESS_VIOLATION:
        return QLatin1String("EXCEPTION_ACCESS_VIOLATION");
    case EXCEPTION_DATATYPE_MISALIGNMENT:
        return QLatin1String("EXCEPTION_DATATYPE_MISALIGNMENT");
    case EXCEPTION_BREAKPOINT:
        return QLatin1String("EXCEPTION_BREAKPOINT");
    case EXCEPTION_SINGLE_STEP:
        return QLatin1String("EXCEPTION_SINGLE_STEP");
    case EXCEPTION_ARRAY_BOUNDS_EXCEEDED:
        return QLatin1String("EXCEPTION_ARRAY_BOUNDS_EXCEEDED");
    case EXCEPTION_FLT_DENORMAL_OPERAND:
        return QLatin1String("EXCEPTION_FLT_DENORMAL_OPERAND");
    case EXCEPTION_FLT_DIVIDE_BY_ZERO:
        return QLatin1String("EXCEPTION_FLT_DIVIDE_BY_ZERO");
    case EXCEPTION_FLT_INEXACT_RESULT:
        return QLatin1String("EXCEPTION_FLT_INEXACT_RESULT");
    case EXCEPTION_FLT_INVALID_OPERATION:
        return QLatin1String("EXCEPTION_FLT_INVALID_OPERATION");
    case EXCEPTION_FLT_OVERFLOW:
        return QLatin1String("EXCEPTION_FLT_OVERFLOW");
    case EXCEPTION_FLT_STACK_CHECK:
        return QLatin1String("EXCEPTION_FLT_STACK_CHECK");
    case EXCEPTION_FLT_UNDERFLOW:
        return QLatin1String("EXCEPTION_FLT_UNDERFLOW");
    case EXCEPTION_INT_DIVIDE_BY_ZERO:
        return QLatin1String("EXCEPTION_INT_DIVIDE_BY_ZERO");
    case EXCEPTION_INT_OVERFLOW:
        return QLatin1String("EXCEPTION_INT_OVERFLOW");
    case EXCEPTION_PRIV_INSTRUCTION:
        return QLatin1String("EXCEPTION_PRIV_INSTRUCTION");
    case EXCEPTION_IN_PAGE_ERROR:
        return QLatin1String("EXCEPTION_IN_PAGE_ERROR");
    case EXCEPTION_ILLEGAL_INSTRUCTION:
        return QLatin1String("EXCEPTION_ILLEGAL_INSTRUCTION");
    case EXCEPTION_NONCONTINUABLE_EXCEPTION:
        return QLatin1String("EXCEPTION_NONCONTINUABLE_EXCEPTION");
    case EXCEPTION_STACK_OVERFLOW:
        return QLatin1String("EXCEPTION_STACK_OVERFLOW");
    case EXCEPTION_INVALID_DISPOSITION:
        return QLatin1String("EXCEPTION_INVALID_DISPOSITION");
#endif
    default:
        return QLatin1String("Unknown");
    }
#endif
}

bool CrashedApplication::hasBeenRestarted() const
{
    return m_restarted;
}

int CrashedApplication::thread() const
{
    return m_thread;
}

const QDateTime &CrashedApplication::datetime() const
{
    return m_datetime;
}

bool CrashedApplication::hasDeletedFiles() const
{
    return m_hasDeletedFiles;
}

void CrashedApplication::restart()
{
    if (m_restarted) {
        return;
    }

    // start the application via CommandLauncherJob so it runs in a new cgroup if possible.
    // if m_fakeBaseName is set, this means m_executable is the path to kdeinit4
    // so we need to use the fakeBaseName to restart the app
    auto job = new KIO::CommandLauncherJob(!m_fakeBaseName.isEmpty() ? m_fakeBaseName : m_executable.absoluteFilePath());
    if (const QString &id = DrKonqi::startupId(); !id.isEmpty()) {
        job->setStartupId(id.toUtf8());
    }
    connect(job, &KIO::CommandLauncherJob::result, this, [job, this] {
        m_restarted = (job->error() == KJob::NoError);
        if (!m_restarted) {
            qCWarning(DRKONQI_LOG) << "Failed to restart:" << job->error() << job->errorString();
        }
        Q_EMIT restarted(m_restarted);
    });
    job->start();
}

QString getSuggestedKCrashFilename(const CrashedApplication *app)
{
    QString filename =
        app->fakeExecutableBaseName() + QLatin1Char('-') + app->datetime().toString(QStringLiteral("yyyyMMdd-hhmmss")) + QStringLiteral(".kcrash");

    if (filename.contains(QLatin1Char('/'))) {
        filename = filename.mid(filename.lastIndexOf(QLatin1Char('/')) + 1);
    }

    return filename;
}

bool CrashedApplication::wasNotResponding() const
{
    return m_applicationNotResponding;
}

#include "moc_crashedapplication.cpp"
