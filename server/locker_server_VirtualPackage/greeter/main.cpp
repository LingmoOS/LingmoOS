/*
SPDX-FileCopyrightText: 2011 Martin Gräßlin <mgraesslin@kde.org>

SPDX-License-Identifier: GPL-2.0-or-later
*/
#include <KLocalizedString>

#include <QCommandLineParser>
#include <QDateTime>
#include <QSessionManager>
#include <QSurfaceFormat>

#include <iostream>

#include <signal.h>

#include "greeterapp.h"

#include <config-kscreenlocker.h>
#include <kscreenlocker_greet_logging.h>

#if HAVE_SYS_PRCTL_H
#include <sys/prctl.h>
#endif
#if HAVE_SYS_PROCCTL_H
#include <sys/procctl.h>
#include <unistd.h>
#endif

#include <KSignalHandler>
#include <LayerShellQt/Shell>

static void signalHandler(int signum)
{
    ScreenLocker::UnlockApp *instance = qobject_cast<ScreenLocker::UnlockApp *>(QCoreApplication::instance());

    if (!instance) {
        return;
    }

    switch (signum) {
    case SIGTERM:
        // exit gracefully to not leave behind screensaver processes (bug#224200)
        // return exit code 1 to indicate that a valid password was not entered,
        // to prevent circumventing the password input by sending a SIGTERM

        qCDebug(KSCREENLOCKER_GREET) << "Greeter received SIGTERM. Will exit with error.";
        instance->exit(1);
        break;
    case SIGUSR1:
        qCDebug(KSCREENLOCKER_GREET) << "Greeter received SIGUSR1. Will lock immediately.";
        instance->lockImmediately();
        break;
    }
}

int main(int argc, char *argv[])
{
    LayerShellQt::Shell::useLayerShell();

    // disable ptrace on the greeter
#if HAVE_PR_SET_DUMPABLE
    prctl(PR_SET_DUMPABLE, 0);
#endif
#if HAVE_PROC_TRACE_CTL
    int mode = PROC_TRACE_CTL_DISABLE;
    procctl(P_PID, getpid(), PROC_TRACE_CTL, &mode);
#endif

    qCDebug(KSCREENLOCKER_GREET) << "Greeter is starting up.";

    KLocalizedString::setApplicationDomain("kscreenlocker_greet");

    // explicitly disable input methods on x11 as it makes it impossible to unlock, see BUG 306932
    // but explicitly set on screen keyboard such as maliit is allowed
    // on wayland, let the compositor take care of the input method
    if (!qEnvironmentVariableIsSet("WAYLAND_DISPLAY") && !qEnvironmentVariableIsSet("WAYLAND_SOCKET")
        && qgetenv("QT_IM_MODULE") != QByteArrayLiteral("maliit")) {
        qputenv("QT_IM_MODULE", QByteArrayLiteral("qtvirtualkeyboard"));
    }

    // Suppresses modal warnings about unwritable configuration files which may render the system inaccessible
    qputenv("KDE_HOME_READONLY", "1");

    auto format = QSurfaceFormat::defaultFormat();
    format.setOption(QSurfaceFormat::ResetNotification);
    QSurfaceFormat::setDefaultFormat(format);

    ScreenLocker::UnlockApp app(argc, argv);

    KSignalHandler::self()->watchSignal(SIGTERM);
    KSignalHandler::self()->watchSignal(SIGUSR1);

    // only connect signal handler once we can actual handle the signal properly
    QObject::connect(KSignalHandler::self(), &KSignalHandler::signalReceived, &app, &signalHandler);

    app.setQuitOnLastWindowClosed(false);
    QCoreApplication::setApplicationName(QStringLiteral("kscreenlocker_greet"));
    QCoreApplication::setApplicationVersion(QStringLiteral("0.1"));
    QCoreApplication::setOrganizationDomain(QStringLiteral("kde.org"));

    // disable session management for the greeter
    auto disableSessionManagement = [](QSessionManager &sm) {
        sm.setRestartHint(QSessionManager::RestartNever);
    };
    QObject::connect(&app, &QGuiApplication::commitDataRequest, disableSessionManagement);
    QObject::connect(&app, &QGuiApplication::saveStateRequest, disableSessionManagement);

    QCommandLineParser parser;
    parser.setApplicationDescription(i18n("Greeter for the KDE Plasma Workspaces Screen locker"));
    parser.addHelpOption();
    parser.addVersionOption();

    QCommandLineOption testingOption(QStringLiteral("testing"), i18n("Starts the greeter in testing mode"));

    QCommandLineOption themeOption(QStringLiteral("theme"),
                                   i18n("Starts the greeter with the selected theme (only in Testing mode)"),
                                   QStringLiteral("theme"),
                                   QStringLiteral(""));

    QCommandLineOption immediateLockOption(QStringLiteral("immediateLock"), i18n("Lock immediately, ignoring any grace time etc."));
    QCommandLineOption graceTimeOption(QStringLiteral("graceTime"),
                                       i18n("Delay till the lock user interface gets shown in milliseconds."),
                                       QStringLiteral("milliseconds"),
                                       QStringLiteral("0"));
    QCommandLineOption nolockOption(QStringLiteral("nolock"), i18n("Don't show any lock user interface."));
    QCommandLineOption switchUserOption(QStringLiteral("switchuser"), i18n("Default to the switch user UI."));

    QCommandLineOption waylandFdOption(QStringLiteral("ksldfd"), i18n("File descriptor for connecting to ksld."), QStringLiteral("fd"));

    parser.addOption(testingOption);
    parser.addOption(themeOption);
    parser.addOption(immediateLockOption);
    parser.addOption(graceTimeOption);
    parser.addOption(nolockOption);
    parser.addOption(switchUserOption);
    parser.addOption(waylandFdOption);
    parser.process(app);

    if (parser.isSet(testingOption)) {
        app.setTesting(true);
        app.setImmediateLock(true);

        // parse theme option
        const QString theme = parser.value(themeOption);
        if (!theme.isEmpty()) {
            app.setTheme(theme);
        }

        // allow ptrace if testing is enabled
#if HAVE_PR_SET_DUMPABLE
        prctl(PR_SET_DUMPABLE, 1);
#endif
#if HAVE_PROC_TRACE_CTL
        int mode = PROC_TRACE_CTL_ENABLE;
        procctl(P_PID, getpid(), PROC_TRACE_CTL, &mode);
#endif
    } else {
        app.setImmediateLock(parser.isSet(immediateLockOption));
    }
    app.setNoLock(parser.isSet(nolockOption));

    bool ok = false;
    int graceTime = parser.value(graceTimeOption).toInt(&ok);
    if (ok) {
        app.setGraceTime(graceTime);
    }

    if (parser.isSet(switchUserOption)) {
        app.setDefaultToSwitchUser(true);
    }

    if (parser.isSet(waylandFdOption)) {
        ok = false;
        const int fd = parser.value(waylandFdOption).toInt(&ok);
        if (ok) {
            app.setKsldSocket(fd);
        }
    }

    app.initialViewSetup();

    // This allow ksmserver to know when the application has actually finished setting itself up.
    // Crucial for blocking until it is ready, ensuring locking happens before sleep, e.g.
    std::cout << "Locked at " << QDateTime::currentDateTime().toSecsSinceEpoch() << std::endl;

    return app.exec();
}
