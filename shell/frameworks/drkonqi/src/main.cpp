/*****************************************************************
 * drkonqi - The KDE Crash Handler
 *
 * SPDX-FileCopyrightText: 2000-2003 Hans Petter Bieker <bieker@kde.org>
 * SPDX-FileCopyrightText: 2021 Harald Sitter <sitter@kde.org>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 *****************************************************************/

#include <chrono>
#include <csignal>
#include <cstdlib>
#include <unistd.h>

#include <QCommandLineParser>
#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusReply>
#include <QDebug>
#include <QFile>
#include <QIcon>
#include <QTemporaryFile>
#include <QTimer>

#include <KAboutData>
#include <KConfigGroup>
#include <KLocalizedString>
#include <KSharedConfig>
#include <KSignalHandler>
#ifdef Q_OS_MACOS
#include <KWindowSystem>
#endif

#include <config-drkonqi.h>

#include "backtracegenerator.h"
#include "bugzillaintegration/reportinterface.h"
#include "debuggermanager.h"
#include "drkonqi.h"
#include "drkonqidialog.h"
#include "statusnotifier.h"

using namespace std::chrono_literals;
using namespace Qt::StringLiterals;

static const char version[] = PROJECT_VERSION;

namespace
{

void aboutToQuit()
{
    if (ReportInterface::self()->hasCrashEventSent() || DrKonqi::debuggerManager()->backtraceGenerator()->hasAnyFailure()) {
        qApp->quit();
    } else {
        // Add a fallback timer. This timer makes sure that drkonqi will definitely quit, even if it should
        // have some sort of runtime defect that prevents reporting from finishing (and consequently not quitting).
        static QTimer timer;
        timer.setInterval(5min); // arbitrary time limit for trace+submission
        QObject::connect(&timer, &QTimer::timeout, qApp, &QCoreApplication::quit);
        QObject::connect(ReportInterface::self(), &ReportInterface::crashEventSent, qApp, &QCoreApplication::quit);
        QObject::connect(DrKonqi::debuggerManager()->backtraceGenerator(), &BacktraceGenerator::stateChanged, qApp, [] {
            if (DrKonqi::debuggerManager()->backtraceGenerator()->hasAnyFailure()) {
                qApp->quit();
            }
        });
        ReportInterface::self()->setSendWhenReady(true);
        timer.start();
    }
}

void openDrKonqiDialog(DrKonqiDialog::GoTo to = DrKonqiDialog::GoTo::Main)
{
    auto *w = new DrKonqiDialog();
    QObject::connect(qApp, &QCoreApplication::aboutToQuit, w, &QObject::deleteLater);
    QObject::connect(qApp, &QGuiApplication::lastWindowClosed, qApp, &aboutToQuit);
    w->show(to);
#ifdef Q_OS_MACOS
    KWindowSystem::forceActiveWindow(w->winId());
#endif
}

void requestDrKonqiDialog(bool restarted, bool interactionAllowed)
{
    auto activation = interactionAllowed ? StatusNotifier::Activation::Allowed : StatusNotifier::Activation::NotAllowed;
    if (ReportInterface::self()->isCrashEventSendingEnabled()) {
        activation = StatusNotifier::Activation::AlreadySubmitting;
        ReportInterface::self()->setSendWhenReady(true);
        if (DrKonqi::debuggerManager()->backtraceGenerator()->state() == BacktraceGenerator::NotLoaded) {
            DrKonqi::debuggerManager()->backtraceGenerator()->start();
        }
    }

    auto *statusNotifier = new StatusNotifier();
    if (interactionAllowed) {
        statusNotifier->show();
    }
    if (!restarted) {
        statusNotifier->notify(activation);
    }
    QObject::connect(statusNotifier, &StatusNotifier::expired, qApp, &aboutToQuit);
    QObject::connect(statusNotifier, &StatusNotifier::activated, qApp, [] {
        openDrKonqiDialog(DrKonqiDialog::GoTo::Main);
    });
    QObject::connect(statusNotifier, &StatusNotifier::sentryActivated, qApp, [] {
        qDebug() << "Sending report to sentry automatically";
        openDrKonqiDialog(DrKonqiDialog::GoTo::Sentry);
    });
}

bool isShuttingDown()
{
    if (QDBusConnection::sessionBus().isConnected()) {
        QDBusInterface remoteApp(QStringLiteral("org.kde.ksmserver"), QStringLiteral("/KSMServer"), QStringLiteral("org.kde.KSMServerInterface"));

        QDBusReply<bool> reply = remoteApp.call(QStringLiteral("isShuttingDown"));
        return reply.isValid() ? reply.value() : false;
    }
    return false;
}
}

int main(int argc, char *argv[])
{
#ifndef Q_OS_WIN // krazy:exclude=cpp
    // Drop privs.
    setgid(getgid());
    if (setuid(getuid()) < 0 && geteuid() != getuid()) {
        exit(255);
    }
#endif

    QApplication app(argc, argv);
    KLocalizedString::setApplicationDomain(QByteArrayLiteral("drkonqi"));

    // We have somewhat special close behavior because of the internal debugger instance as well as auto-submission.
    // Make sure to honor SIGINT in a timely manner.
    KSignalHandler::self()->watchSignal(SIGINT);
    QObject::connect(KSignalHandler::self(), &KSignalHandler::signalReceived, &app, [&app](int signal) {
        if (signal == SIGINT) {
            app.quit();
        }
    });

    // Prevent KApplication from setting the crash handler. We will set it later...
    setenv("KDE_DEBUG", "true", 1);
    // Session management is not needed, do not even connect in order to survive longer than ksmserver.
    unsetenv("SESSION_MANAGER");

    KAboutData aboutData(QStringLiteral("drkonqi"),
                         i18n("Crash Handler"),
                         QString::fromLatin1(version),
                         i18n("Crash Handler gives the user feedback "
                              "if a program has crashed."),
                         KAboutLicense::GPL,
                         i18n("(C) 2000-2018, The DrKonqi Authors"));
    aboutData.addAuthor(i18nc("@info:credit", "Hans Petter Bieker"), QString(), QStringLiteral("bieker@kde.org"));
    aboutData.addAuthor(i18nc("@info:credit", "Dario Andres Rodriguez"), QString(), QStringLiteral("andresbajotierra@gmail.com"));
    aboutData.addAuthor(i18nc("@info:credit", "George Kiagiadakis"), QString(), QStringLiteral("gkiagia@users.sourceforge.net"));
    aboutData.addAuthor(i18nc("@info:credit", "A. L. Spehr"), QString(), QStringLiteral("spehr@kde.org"));
    KAboutData::setApplicationData(aboutData);
    app.setWindowIcon(QIcon::fromTheme(QStringLiteral("tools-report-bug"), app.windowIcon()));

    QCommandLineParser parser;
    aboutData.setupCommandLine(&parser);

    const QCommandLineOption signalOption(QStringLiteral("signal"), i18nc("@info:shell", "The signal <number> that was caught"), QStringLiteral("number"));
    const QCommandLineOption appNameOption(QStringLiteral("appname"), i18nc("@info:shell", "<Name> of the program"), QStringLiteral("name"));
    const QCommandLineOption appPathOption(QStringLiteral("apppath"), i18nc("@info:shell", "<Path> to the executable"), QStringLiteral("path"));
    const QCommandLineOption appVersionOption(QStringLiteral("appversion"), i18nc("@info:shell", "The <version> of the program"), QStringLiteral("version"));
    const QCommandLineOption bugAddressOption(QStringLiteral("bugaddress"), i18nc("@info:shell", "The bug <address> to use"), QStringLiteral("address"));
    const QCommandLineOption programNameOption(QStringLiteral("programname"), i18nc("@info:shell", "Translated <name> of the program"), QStringLiteral("name"));
    const QCommandLineOption productNameOption(QStringLiteral("productname"), i18nc("@info:shell", "Bugzilla product name"), QStringLiteral("name"));
    const QCommandLineOption pidOption(QStringLiteral("pid"), i18nc("@info:shell", "The <PID> of the program"), QStringLiteral("pid"));
    const QCommandLineOption startupIdOption(QStringLiteral("startupid"), i18nc("@info:shell", "Startup <ID> of the program"), QStringLiteral("id"));
    const QCommandLineOption kdeinitOption(QStringLiteral("kdeinit"), i18nc("@info:shell", "The program was started by kdeinit"));
    const QCommandLineOption saferOption(QStringLiteral("safer"), i18nc("@info:shell", "Disable arbitrary disk access"));
    const QCommandLineOption restartedOption(QStringLiteral("restarted"), i18nc("@info:shell", "The program has already been restarted"));
    const QCommandLineOption keepRunningOption(QStringLiteral("keeprunning"),
                                               i18nc("@info:shell",
                                                     "Keep the program running and generate "
                                                     "the backtrace at startup"));
    const QCommandLineOption threadOption(QStringLiteral("thread"), i18nc("@info:shell", "The <thread id> of the failing thread"), QStringLiteral("threadid"));
    const QCommandLineOption dialogOption(QStringLiteral("dialog"), i18nc("@info:shell", "Do not show a notification but launch the debug dialog directly"));
    const QCommandLineOption glRendererOption(u"glrenderer"_s, u"The GL_RENDERER used by the process"_s, u"glrenderer"_s);
    const QCommandLineOption exceptionNameOption(u"exceptionname"_s, u"The exception class name if an exception was the cause"_s, u"name"_s);
    const QCommandLineOption exceptionWhatOption(u"exceptionwhat"_s, u"The exception what string if an exception was the cause"_s, u"what"_s);

    parser.addOptions({signalOption,
                       appNameOption,
                       appPathOption,
                       appVersionOption,
                       bugAddressOption,
                       programNameOption,
                       productNameOption,
                       pidOption,
                       startupIdOption,
                       kdeinitOption,
                       saferOption,
                       restartedOption,
                       keepRunningOption,
                       threadOption,
                       dialogOption,
                       glRendererOption,
                       exceptionNameOption,
                       exceptionWhatOption});

    // Add all unknown options but make sure to print a warning.
    // This enables older DrKonqi's to run by newer KCrash instances with
    // possibly different/new options.
    // KCrash can always send all options it knows to send and be sure that
    // DrKonqi will not explode on them. If an option is not known here it's
    // either too old or too new.
    //
    // To implement this smartly we'll ::parse all arguments, and then ::process
    // them again once we have injected no-op options for all unknown ones.
    // This allows ::process to still do common argument handling for --version
    // as well as standard error handling.
    if (!parser.parse(app.arguments())) {
        const QStringList unknownOptionNames = parser.unknownOptionNames();
        for (const QString &option : unknownOptionNames) {
            qWarning() << "Unknown option" << option << " - ignoring it.";
            parser.addOption(QCommandLineOption(option));
        }
    }

    parser.process(app);
    aboutData.processCommandLine(&parser);

    DrKonqi::setSignal(parser.value(signalOption).toInt());
    DrKonqi::setAppName(parser.value(appNameOption));
    DrKonqi::setAppPath(parser.value(appPathOption));
    DrKonqi::setAppVersion(parser.value(appVersionOption));
    DrKonqi::setBugAddress(parser.value(bugAddressOption));
    DrKonqi::setProgramName(parser.value(programNameOption));
    DrKonqi::setProductName(parser.value(productNameOption));
    DrKonqi::setPid(parser.value(pidOption).toInt());
    DrKonqi::setKdeinit(parser.isSet(kdeinitOption));
    DrKonqi::setSafer(parser.isSet(saferOption));
    DrKonqi::setRestarted(parser.isSet(restartedOption));
    DrKonqi::setKeepRunning(parser.isSet(keepRunningOption));
    DrKonqi::setThread(parser.value(threadOption).toInt());
    DrKonqi::setStartupId(parser.value(startupIdOption));
    DrKonqi::instance()->m_glRenderer = parser.value(glRendererOption);
    DrKonqi::instance()->m_exceptionName = parser.value(exceptionNameOption);
    if (!DrKonqi::instance()->m_exceptionName.isEmpty()) {
        QProcess proc;
        proc.setProgram(u"c++filt"_s);
        proc.setArguments({u"--types"_s, DrKonqi::instance()->m_exceptionName});
        proc.start();
        if (proc.waitForFinished((1000ms).count())) {
            DrKonqi::instance()->m_exceptionName = QString::fromUtf8(proc.readAllStandardOutput().trimmed());
        }
        if (DrKonqi::instance()->m_exceptionName.isEmpty()) { // restore the potentially mangled version if empty
            DrKonqi::instance()->m_exceptionName = parser.value(exceptionNameOption);
        }
    }
    DrKonqi::instance()->m_exceptionWhat = parser.value(exceptionWhatOption);
    auto forceDialog = parser.isSet(dialogOption);

    if (!DrKonqi::init()) {
        return 1;
    }

    app.setQuitOnLastWindowClosed(false);
    // https://bugs.kde.org/show_bug.cgi?id=471941
    app.setQuitLockEnabled(false);

    const bool restarted = parser.isSet(restartedOption);

    // Whether the user should be encouraged to file a bug report
    const bool interactionAllowed = KConfigGroup(KSharedConfig::openConfig(), QStringLiteral("General")).readEntry("InteractionAllowed", true);
    const bool shuttingDown = isShuttingDown();

    if (forceDialog) {
        openDrKonqiDialog();
    } else if (shuttingDown) {
        return 0;
    } else {
        // if no notification service is running (eg. shell crashed, or other desktop environment)
        // and we didn't auto-restart the app, open DrKonqi dialog instead of showing an SNI
        // and emitting a desktop notification.
        if (!StatusNotifier::notificationServiceRegistered() && !restarted) {
            openDrKonqiDialog();
        } else { // StatusNotifierItem (interaction) or notification (no interaction)
            requestDrKonqiDialog(restarted, interactionAllowed);
        }
    }

    return app.exec();
}
