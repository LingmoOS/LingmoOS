/*
    SPDX-FileCopyrightText: 2009 George Kiagiadakis <gkiagia@users.sourceforge.net>
    SPDX-FileCopyrightText: 2021 Harald Sitter <sitter@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/
#include "drkonqibackends.h"

#include <cerrno>
#include <chrono>
#include <cstdlib>
#include <signal.h>
#include <sys/types.h>

#include <QDir>
#include <QRegularExpression>
#include <QTimer>

#include "drkonqi_debug.h"
#include <KConfig>
#include <KConfigGroup>
#include <KCrash>
#include <QStandardPaths>

#include "backtracegenerator.h"
#include "coredump/metadata.h"
#include "crashedapplication.h"
#include "debugger.h"
#include "debuggermanager.h"
#include "drkonqi.h"
#include "linuxprocmapsparser.h"

#ifdef Q_OS_MACOS
#include <AvailabilityMacros.h>
#endif

using namespace std::chrono_literals;

AbstractDrKonqiBackend::~AbstractDrKonqiBackend() = default;

bool AbstractDrKonqiBackend::init()
{
    m_crashedApplication = constructCrashedApplication();
    m_debuggerManager = constructDebuggerManager();
    return true;
}

void AbstractDrKonqiBackend::prepareForDebugger()
{
    Q_EMIT preparedForDebugger();
}

QString AbstractDrKonqiBackend::metadataPath()
{
    static QString path = [] {
        const QString envPath = qEnvironmentVariable("DRKONQI_METADATA_FILE");
        qunsetenv("DRKONQI_METADATA_FILE");
        Q_ASSERT(!envPath.isEmpty());
        return envPath;
    }();
    return path;
}

KCrashBackend::~KCrashBackend()
{
    continueAttachedProcess();
}

bool KCrashBackend::init()
{
    AbstractDrKonqiBackend::init();

    // check whether the attached process exists and whether we have permissions to inspect it
    if (crashedApplication()->pid() <= 0) {
        qCWarning(DRKONQI_LOG) << "Invalid pid specified";
        return false;
    }

#if !defined(Q_OS_WIN32)
    if (::kill(crashedApplication()->pid(), 0) < 0) {
        switch (errno) {
        case EPERM:
            qCWarning(DRKONQI_LOG) << "DrKonqi doesn't have permissions to inspect the specified process";
            break;
        case ESRCH:
            qCWarning(DRKONQI_LOG) << "The specified process does not exist.";
            break;
        default:
            break;
        }
        return false;
    }

    //--keeprunning means: generate backtrace instantly and let the process continue execution
    if (DrKonqi::isKeepRunning()) {
        stopAttachedProcess();
        debuggerManager()->backtraceGenerator()->start();
        connect(debuggerManager(), &DebuggerManager::debuggerFinished, this, &KCrashBackend::continueAttachedProcess);
    } else {
        connect(debuggerManager(), &DebuggerManager::debuggerStarting, this, &KCrashBackend::onDebuggerStarting);
        connect(debuggerManager(), &DebuggerManager::debuggerFinished, this, &KCrashBackend::onDebuggerFinished);

        // stop the process to avoid high cpu usage by other threads (bug 175362).
        // if the process was started by kdeinit, we need to wait a bit for KCrash
        // to reach the alarm(0); call in kdeui/util/kcrash.cpp line 406 or else
        // if we stop it before this call, pending alarm signals will kill the
        // process when we try to continue it.
        QTimer::singleShot(2s, this, &KCrashBackend::stopAttachedProcess);
    }
#endif

    // Handle drkonqi crashes
    s_pid = crashedApplication()->pid(); // copy pid for use by the crash handler, so that it is safer
    KCrash::setEmergencySaveFunction(emergencySaveFunction);

    return true;
}

CrashedApplication *KCrashBackend::constructCrashedApplication()
{
    const auto pid = DrKonqi::pid();
    QFileInfo executable;
    QString fakeBaseName;
    bool hasDeletedFiles = false;

    // try to determine the executable that crashed
    const QString procPath(QStringLiteral("/proc/%1").arg(pid));
    const QString exeProcPath(procPath + QStringLiteral("/exe"));
    if (QFileInfo::exists(exeProcPath)) {
        // on linux, the fastest and most reliable way is to get the path from /proc
        qCDebug(DRKONQI_LOG) << "Using /proc to determine executable path";
        const QString exePath = QFile::symLinkTarget(exeProcPath);

        executable.setFile(exePath);
        if (DrKonqi::isKdeinit() || executable.fileName().startsWith(QLatin1String("python"))) {
            fakeBaseName = DrKonqi::appName();
        }

        const QString mapsPath = procPath + QStringLiteral("/maps");
        QFile mapsFile(mapsPath);
        if (mapsFile.open(QFile::ReadOnly)) {
            hasDeletedFiles = LinuxProc::hasMapsDeletedFiles(exePath, mapsFile.readAll(), LinuxProc::Check::DeletedMarker);
        } else {
            qCWarning(DRKONQI_LOG) << "failed to open maps file" << mapsPath;
        }

        qCDebug(DRKONQI_LOG) << "exe" << exePath << "has deleted files:" << hasDeletedFiles;
    } else {
        if (DrKonqi::isKdeinit()) {
            executable = QFileInfo(QStandardPaths::findExecutable(QStringLiteral("kdeinit5")));
            fakeBaseName = DrKonqi::appName();
        } else {
            QFileInfo execPath(DrKonqi::appName());
            if (execPath.isAbsolute()) {
                executable = execPath;
            } else if (!DrKonqi::appPath().isEmpty()) {
                QDir execDir(DrKonqi::appPath());
                executable = QFileInfo(execDir.absoluteFilePath(execPath.fileName()));
            } else {
                executable = QFileInfo(QStandardPaths::findExecutable(execPath.fileName()));
            }
        }
    }

    qCDebug(DRKONQI_LOG) << "Executable is:" << executable.absoluteFilePath();
    qCDebug(DRKONQI_LOG) << "Executable exists:" << executable.exists();

    // We could support ANR by assuming time==now and bootid==current, but this backend is a bit dated anyway, so probably not
    // worth it => always assume it wasn't ANR.
    static constexpr auto applicationNotResponding = false;
    return new CrashedApplication(pid,
                                  DrKonqi::thread(),
                                  DrKonqi::signal(),
                                  executable,
                                  DrKonqi::appVersion(),
                                  BugReportAddress(DrKonqi::bugAddress()),
                                  DrKonqi::programName(),
                                  DrKonqi::productName(),
                                  QDateTime::currentDateTime(),
                                  DrKonqi::isRestarted(),
                                  hasDeletedFiles,
                                  applicationNotResponding,
                                  fakeBaseName,
                                  this);
}

DebuggerManager *KCrashBackend::constructDebuggerManager()
{
    KConfigGroup config(KSharedConfig::openConfig(), QStringLiteral("DrKonqi"));
#if defined(__MAC_OS_X_VERSION_MAX_ALLOWED) && __MAC_OS_X_VERSION_MAX_ALLOWED > 1070
    QString defaultDebuggerName = config.readEntry("Debugger", QStringLiteral("lldb"));
#elif !defined(Q_OS_WIN)
    QString defaultDebuggerName = config.readEntry("Debugger", QStringLiteral("gdb"));
#else
    QString defaultDebuggerName = config.readEntry("Debugger", QStringLiteral("cdb"));
#endif

    const QList<Debugger> internalDebuggers = Debugger::availableInternalDebuggers(QStringLiteral("KCrash"));
    const QList<Debugger> externalDebuggers = Debugger::availableExternalDebuggers(QStringLiteral("KCrash"));

    const Debugger preferredDebugger(Debugger::findDebugger(internalDebuggers, defaultDebuggerName));
    qCDebug(DRKONQI_LOG) << "Using debugger:" << preferredDebugger.codeName();
    return new DebuggerManager(preferredDebugger, externalDebuggers, this);
}

void KCrashBackend::stopAttachedProcess()
{
    if (m_state == ProcessRunning) {
        qCDebug(DRKONQI_LOG) << "Sending SIGSTOP to process";
        ::kill(crashedApplication()->pid(), SIGSTOP);
        m_state = ProcessStopped;
    }
}

void KCrashBackend::continueAttachedProcess()
{
    if (m_state == ProcessStopped) {
        qCDebug(DRKONQI_LOG) << "Sending SIGCONT to process";
        ::kill(crashedApplication()->pid(), SIGCONT);
        m_state = ProcessRunning;
    }
}

void KCrashBackend::onDebuggerStarting()
{
    continueAttachedProcess();
    m_state = DebuggerRunning;
}

void KCrashBackend::onDebuggerFinished()
{
    m_state = ProcessRunning;
    stopAttachedProcess();
}

// static
qint64 KCrashBackend::s_pid = 0;

// static
void KCrashBackend::emergencySaveFunction(int signal)
{
    // In case drkonqi itself crashes, we need to get rid of the process being debugged,
    // so we kill it, no matter what its state was.
    Q_UNUSED(signal);
    ::kill(s_pid, SIGKILL);
}

#include "moc_drkonqibackends.cpp"
