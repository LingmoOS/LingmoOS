/*
    SPDX-FileCopyrightText: 2008 Nicola Gigante <nicola.gigante@gmail.com>
    SPDX-FileCopyrightText: 2009 Dario Freddi <drf@kde.org>
    SPDX-FileCopyrightText: 2020 Harald Sitter <sitter@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "helpersupport.h"

#include <cstdlib>

#ifndef Q_OS_WIN
#include <pwd.h>
#include <sys/types.h>
#include <syslog.h>
#include <unistd.h>
#else
// Quick hack to replace syslog (just write to stderr)
// TODO: should probably use ReportEvent
#define LOG_ERR 3
#define LOG_WARNING 4
#define LOG_DEBUG 7
#define LOG_INFO 6
#define LOG_USER (1 << 3)
static inline void openlog(const char *, int, int)
{
}
static inline void closelog()
{
}
#define syslog(level, ...) fprintf(stderr, __VA_ARGS__)

#endif

#include <QCoreApplication>
#include <QTimer>

#include "BackendsManager.h"

namespace KAuth
{
namespace HelperSupport
{
void helperDebugHandler(QtMsgType type, const QMessageLogContext &context, const QString &msgStr);
}

static bool remote_dbg = false;

#ifdef Q_OS_UNIX
static void fixEnvironment()
{
    // try correct HOME
    const char *home = "HOME";
    if (getenv(home) == nullptr) {
        struct passwd *pw = getpwuid(getuid());

        if (pw != nullptr) {
            int overwrite = 0;
            setenv(home, pw->pw_dir, overwrite);
        }
    }
}
#endif

int HelperSupport::helperMain(int argc, char **argv, const char *id, QObject *responder)
{
#ifdef Q_OS_UNIX
    fixEnvironment();
#endif

#ifdef Q_OS_OSX
    openlog(id, LOG_CONS | LOG_PID, LOG_USER);
    int logLevel = LOG_WARNING;
#else
    openlog(id, 0, LOG_USER);
    int logLevel = LOG_DEBUG;
#endif
    qInstallMessageHandler(&HelperSupport::helperDebugHandler);

    // NOTE: The helper proxy might use dbus, and we should have the qapp
    //       before using dbus.
    QCoreApplication app(argc, argv);

    if (!BackendsManager::helperProxy()->initHelper(QString::fromLatin1(id))) {
        syslog(logLevel, "Helper initialization failed");
        return -1;
    }

    // closelog();
    remote_dbg = true;

    BackendsManager::helperProxy()->setHelperResponder(responder);

    // Attach the timer
    QTimer *timer = new QTimer(nullptr);
    responder->setProperty("__KAuth_Helper_Shutdown_Timer", QVariant::fromValue(timer));
    timer->setInterval(10000);
    timer->start();
    QObject::connect(timer, &QTimer::timeout, &app, &QCoreApplication::quit);
    app.exec(); // krazy:exclude=crashy

    return 0;
}

void HelperSupport::helperDebugHandler(QtMsgType type, const QMessageLogContext &context, const QString &msgStr)
{
    Q_UNUSED(context); // can be used to find out about file, line, function name
    QByteArray msg = msgStr.toLocal8Bit();
    if (!remote_dbg) {
        int level = LOG_DEBUG;
        switch (type) {
        case QtDebugMsg:
            level = LOG_DEBUG;
            break;
        case QtWarningMsg:
            level = LOG_WARNING;
            break;
        case QtCriticalMsg:
        case QtFatalMsg:
            level = LOG_ERR;
            break;
        case QtInfoMsg:
            level = LOG_INFO;
            break;
        }
        syslog(level, "%s", msg.constData());
    } else {
        BackendsManager::helperProxy()->sendDebugMessage(type, msg.constData());
    }

    // Anyway I should follow the rule:
    if (type == QtFatalMsg) {
        exit(-1);
    }
}

void HelperSupport::progressStep(int step)
{
    BackendsManager::helperProxy()->sendProgressStep(step);
}

void HelperSupport::progressStep(const QVariantMap &data)
{
    BackendsManager::helperProxy()->sendProgressStepData(data);
}

bool HelperSupport::isStopped()
{
    return BackendsManager::helperProxy()->hasToStopAction();
}

int HelperSupport::callerUid()
{
    return BackendsManager::helperProxy()->callerUid();
}

} // namespace Auth
