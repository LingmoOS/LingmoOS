/* vi: ts=8 sts=4 sw=4
 *
 * This file is part of the KDE project, module kdesu.
 * SPDX-FileCopyrightText: 1998 Pietro Iglio <iglio@fub.it>
 * SPDX-FileCopyrightText: 1999, 2000 Geert Jansen <jansen@kde.org>
 * SPDX-License-Identifier: Artistic-2.0
 */

#include <config-kde-cli-tools.h>

#include <errno.h>
#include <pwd.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/resource.h>
#include <sys/time.h>
#if HAVE_SYS_WAIT_H
#include <sys/wait.h>
#endif
#if HAVE_SYS_PRCTL_H
#include <sys/prctl.h>
#endif
#if HAVE_SYS_PROCCTL_H
#include <sys/procctl.h>
#include <unistd.h>
#endif

#include <QApplication>
#include <QCommandLineParser>
#include <QFile>
#include <QFileInfo>
#include <QLoggingCategory>
#include <private/qtx11extras_p.h>

#include <KSharedConfig>
#include <kaboutdata.h>
#include <kconfiggroup.h>
#include <klocalizedstring.h>
#include <kmessagebox.h>
#include <kshell.h>
#include <kstartupinfo.h>
#include <kuser.h>
#include <kwindowsystem.h>

#include <kdesu/client.h>
#include <kdesu/defaults.h>
#include <kdesu/suprocess.h>

#include "sudlg.h"

#define ERR strerror(errno)

static QLoggingCategory category("org.kde.kdesu");

QByteArray command;
const char *Version = PROJECT_VERSION;

// NOTE: if you change the position of the -u switch, be sure to adjust it
// at the beginning of main()

static int startApp(QCommandLineParser &p);

int main(int argc, char *argv[])
{
    // disable ptrace
#if HAVE_PR_SET_DUMPABLE
    prctl(PR_SET_DUMPABLE, 0);
#endif
#if HAVE_PROC_TRACE_CTL
    int mode = PROC_TRACE_CTL_DISABLE;
    procctl(P_PID, getpid(), PROC_TRACE_CTL, &mode);
#endif

    QApplication app(argc, argv);

    // FIXME: this can be considered a poor man's solution, as it's not
    // directly obvious to a gui user. :)
    // anyway, i vote against removing it even when we have a proper gui
    // implementation.  -- ossi
    QByteArray duser = qgetenv("ADMIN_ACCOUNT");
    if (duser.isEmpty()) {
        duser = "root";
    }

    KLocalizedString::setApplicationDomain(QByteArrayLiteral("kdesu"));

    KAboutData aboutData(QStringLiteral("kdesu"),
                         i18n("KDE su"),
                         QLatin1String(Version),
                         i18n("Runs a program with elevated privileges."),
                         KAboutLicense::Artistic,
                         i18n("Copyright (c) 1998-2000 Geert Jansen, Pietro Iglio"));
    aboutData.addAuthor(i18n("Geert Jansen"), i18n("Maintainer"), QStringLiteral("jansen@kde.org"), QStringLiteral("http://www.stack.nl/~geertj/"));
    aboutData.addAuthor(i18n("Pietro Iglio"), i18n("Original author"), QStringLiteral("iglio@fub.it"));
    app.setWindowIcon(QIcon::fromTheme(QStringLiteral("dialog-password")));

    KAboutData::setApplicationData(aboutData);

    // NOTE: if you change the position of the -u switch, be sure to adjust it
    // at the beginning of main()
    QCommandLineParser parser;
    aboutData.setupCommandLine(&parser);
    parser.addPositionalArgument(QStringLiteral("command"), i18n("Specifies the command to run as separate arguments"));
    parser.addOption(QCommandLineOption(QStringLiteral("c"), i18n("Specifies the command to run as one string"), QStringLiteral("command")));
    parser.addOption(QCommandLineOption(QStringLiteral("f"), i18n("Run command under target uid if <file> is not writable"), QStringLiteral("file")));
    parser.addOption(QCommandLineOption(QStringLiteral("u"), i18n("Specifies the target uid"), QStringLiteral("user"), QString::fromLatin1(duser)));
    parser.addOption(QCommandLineOption(QStringLiteral("n"), i18n("Do not keep password")));
    parser.addOption(QCommandLineOption(QStringLiteral("s"), i18n("Stop the daemon (forgets all passwords)")));
    parser.addOption(QCommandLineOption(QStringLiteral("t"), i18n("Enable terminal output (no password keeping)")));
    parser.addOption(
        QCommandLineOption(QStringLiteral("p"), i18n("Set priority value: 0 <= prio <= 100, 0 is lowest"), QStringLiteral("prio"), QStringLiteral("50")));
    parser.addOption(QCommandLineOption(QStringLiteral("r"), i18n("Use realtime scheduling")));
    parser.addOption(QCommandLineOption(QStringLiteral("noignorebutton"), i18n("Do not display ignore button")));
    parser.addOption(QCommandLineOption(QStringLiteral("i"), i18n("Specify icon to use in the password dialog"), QStringLiteral("icon name")));
    parser.addOption(QCommandLineOption(QStringLiteral("d"), i18n("Do not show the command to be run in the dialog")));
#if WITH_X11
    /* KDialog originally used --embed for attaching the dialog box.  However this is misleading and so we changed to --attach.
     * For consistancy, we silently map --embed to --attach */
    parser.addOption(QCommandLineOption(QStringLiteral("attach"),
                                        i18nc("Transient means that the kdesu app will be attached to the app specified by the winid so that it is like a "
                                              "dialog box rather than some separate program",
                                              "Makes the dialog transient for an X app specified by winid"),
                                        QStringLiteral("winid")));
    parser.addOption(QCommandLineOption(QStringLiteral("embed"), i18n("Embed into a window"), QStringLiteral("winid")));
#endif

    // KApplication::disableAutoDcopRegistration();
    // kdesu doesn't process SM events, so don't even connect to ksmserver
    QByteArray session_manager = qgetenv("SESSION_MANAGER");
    if (!session_manager.isEmpty()) {
        unsetenv("SESSION_MANAGER");
    }
    // but propagate it to the started app
    if (!session_manager.isEmpty()) {
        setenv("SESSION_MANAGER", session_manager.data(), 1);
    }

    {
#if WITH_X11
        KStartupInfoId id;
        id.initId();
        id.setupStartupEnv(); // make DESKTOP_STARTUP_ID env. var. available again
#endif
    }

    parser.process(app);
    aboutData.processCommandLine(&parser);
    int result = startApp(parser);

    if (result == 127) {
        KMessageBox::error(nullptr, i18n("Cannot execute command '%1'.", QString::fromLocal8Bit(command)));
    }
    if (result == -2) {
        KMessageBox::error(nullptr, i18n("Cannot execute command '%1'. It contains invalid characters.", QString::fromLocal8Bit(command)));
    }

    return result;
}

static int startApp(QCommandLineParser &p)
{
    // Stop daemon and exit?
    if (p.isSet(QStringLiteral("s"))) {
        KDESu::Client client;
        if (client.ping() == -1) {
            qCCritical(category) << "Daemon not running -- nothing to stop\n";
            p.showHelp(1);
        }
        if (client.stopServer() != -1) {
            qCDebug(category) << "Daemon stopped\n";
            exit(0);
        }
        qCCritical(category) << "Could not stop daemon\n";
        p.showHelp(1);
    }

    QString icon;
    if (p.isSet(QStringLiteral("i"))) {
        icon = p.value(QStringLiteral("i"));
    }

    bool prompt = true;
    if (p.isSet(QStringLiteral("d"))) {
        prompt = false;
    }

    // Get target uid
    const QByteArray user = p.value(QStringLiteral("u")).toLocal8Bit();
    QByteArray auth_user = user;
    struct passwd *pw = getpwnam(user.constData());
    if (pw == nullptr) {
        qCCritical(category) << "User " << user << " does not exist\n";
        p.showHelp(1);
    }
    bool other_uid = (getuid() != pw->pw_uid);
    bool change_uid = other_uid;
    if (!change_uid) {
        char *cur_user = getenv("USER");
        if (!cur_user) {
            cur_user = getenv("LOGNAME");
        }
        change_uid = (!cur_user || user != cur_user);
    }

    // If file is writeable, do not change uid
    QString file = p.value(QStringLiteral("f"));
    if (other_uid && !file.isEmpty()) {
        if (file.startsWith(QLatin1Char('/'))) {
            file = QStandardPaths::locate(QStandardPaths::GenericConfigLocation, file);
            if (file.isEmpty()) {
                qCCritical(category) << "Config file not found: " << file;
                p.showHelp(1);
            }
        }
        QFileInfo fi(file);
        if (!fi.exists()) {
            qCCritical(category) << "File does not exist: " << file;
            p.showHelp(1);
        }
        change_uid = !fi.isWritable();
    }

    // Get priority/scheduler
    QString tmp = p.value(QStringLiteral("p"));
    bool ok;
    int priority = tmp.toInt(&ok);
    if (!ok || (priority < 0) || (priority > 100)) {
        qCCritical(category) << i18n("Illegal priority: %1", tmp);
        p.showHelp(1);
    }
    int scheduler = SuProcess::SchedNormal;
    if (p.isSet(QStringLiteral("r"))) {
        scheduler = SuProcess::SchedRealtime;
    }
    if ((priority > 50) || (scheduler != SuProcess::SchedNormal)) {
        change_uid = true;
        auth_user = "root";
    }

    // Get command
    if (p.isSet(QStringLiteral("c"))) {
        command = p.value(QStringLiteral("c")).toLocal8Bit();
        // Accepting additional arguments here is somewhat weird,
        // but one can conceive use cases: have a complex command with
        // redirections and additional file names which need to be quoted
        // safely.
    } else {
        if (p.positionalArguments().count() == 0) {
            qCCritical(category) << i18n("No command specified.");
            p.showHelp(1);
        }
    }
    const auto positionalArguments = p.positionalArguments();
    for (const QString &arg : positionalArguments) {
        command += ' ';
        command += QFile::encodeName(KShell::quoteArg(arg));
    }

    // Don't change uid if we're don't need to.
    if (!change_uid) {
        int result = system(command.constData());
        result = WEXITSTATUS(result);
        return result;
    }

    // Check for daemon and start if necessary
    bool just_started = false;
    bool have_daemon = true;
    KDESu::Client client;
    if (client.ping() == -1) {
        if (client.startServer() == -1) {
            qCWarning(category) << "Could not start daemon, reduced functionality.\n";
            have_daemon = false;
        }
        just_started = true;
    }

    // Try to exec the command with kdesud.
    bool keep = !p.isSet(QStringLiteral("n")) && have_daemon;
    bool terminal = p.isSet(QStringLiteral("t"));
    bool withIgnoreButton = !p.isSet(QStringLiteral("noignorebutton"));
    int winid = -1;
    bool attach = p.isSet(QStringLiteral("attach"));
    if (attach) {
        winid = p.value(QStringLiteral("attach")).toInt(&attach, 0); // C style parsing.  If the string begins with "0x", base 16 is used; if the string begins
                                                                     // with "0", base 8 is used; otherwise, base 10 is used.
        if (!attach) {
            qCWarning(category) << "Specified winid to attach to is not a valid number";
        }
    } else if (p.isSet(QStringLiteral("embed"))) {
        /* KDialog originally used --embed for attaching the dialog box.  However this is misleading and so we changed to --attach.
         * For consistancy, we silently map --embed to --attach */
        attach = true;
        winid = p.value(QStringLiteral("embed")).toInt(&attach, 0); // C style parsing.  If the string begins with "0x", base 16 is used; if the string begins
                                                                    // with "0", base 8 is used; otherwise, base 10 is used.
        if (!attach) {
            qCWarning(category) << "Specified winid to attach to is not a valid number";
        }
    }

    QList<QByteArray> env;
    QByteArray options;
    env << ("DESKTOP_STARTUP_ID=" + QX11Info::nextStartupId());

    //     TODO: Maybe should port this to XDG_*, somehow?
    //     if (pw->pw_uid)
    //     {
    //        // Only propagate KDEHOME for non-root users,
    //        // root uses KDEROOTHOME
    //
    //        // Translate the KDEHOME of this user to the new user.
    //        QString kdeHome = KGlobal::dirs()->relativeLocation("home", KGlobal::dirs()->localkdedir());
    //        if (kdeHome[0] != '/')
    //           kdeHome.prepend("~/");
    //        else
    //           kdeHome.clear(); // Use default
    //
    //        env << ("KDEHOME="+ QFile::encodeName(kdeHome));
    //     }

    KUser u;
    env << (QByteArray)("KDESU_USER=" + u.loginName().toLocal8Bit());

    if (keep && !terminal && !just_started) {
        client.setPriority(priority);
        client.setScheduler(scheduler);
        int result = client.exec(command, user, options, env);
        if (result == 0) {
            result = client.exitCode();
            return result;
        }
    }

    // Set core dump size to 0 because we will have
    // root's password in memory.
    struct rlimit rlim;
    rlim.rlim_cur = rlim.rlim_max = 0;
    if (setrlimit(RLIMIT_CORE, &rlim)) {
        qCCritical(category) << "rlimit(): " << ERR;
        p.showHelp(1);
    }

    // Read configuration
    KConfigGroup config(KSharedConfig::openConfig(), "Passwords");
    int timeout = config.readEntry("Timeout", defTimeout);

    // Check if we need a password
    SuProcess proc;
    proc.setUser(auth_user);
    int needpw = proc.checkNeedPassword();
    if (needpw < 0) {
        QString err = i18n("Su returned with an error.\n");
        KMessageBox::error(nullptr, err);
        p.showHelp(1);
    }
    if (needpw == 0) {
        keep = 0;
        qDebug() << "Don't need password!!\n";
    }

    const QString cmd = QString::fromLocal8Bit(command);
    for (const QChar character : cmd) {
        if (!character.isPrint() && character.category() != QChar::Other_Surrogate) {
            return -2;
        }
    }

    // Start the dialog
    QString password;
    if (needpw) {
#if WITH_X11
        KStartupInfoId id;
        id.initId();
        KStartupInfoData data;
        data.setSilent(KStartupInfoData::Yes);
        KStartupInfo::sendChange(id, data);
#endif
        KDEsuDialog dlg(user, auth_user, keep && !terminal, icon, withIgnoreButton);
        if (prompt) {
            dlg.addCommentLine(i18n("Command:"), QFile::decodeName(command));
        }
        if (defKeep) {
            dlg.setKeepPassword(true);
        }

        if ((priority != 50) || (scheduler != SuProcess::SchedNormal)) {
            QString prio;
            if (scheduler == SuProcess::SchedRealtime) {
                prio += i18n("realtime: ");
            }
            prio += QStringLiteral("%1/100").arg(priority);
            if (prompt) {
                dlg.addCommentLine(i18n("Priority:"), prio);
            }
        }

        // Attach dialog
#if WITH_X11
        if (attach) {
            dlg.setAttribute(Qt::WA_NativeWindow, true);
            KWindowSystem::setMainWindow(dlg.windowHandle(), WId(winid));
        }
#endif
        int ret = dlg.exec();
        if (ret == KDEsuDialog::Rejected) {
#if WITH_X11
            KStartupInfo::sendFinish(id);
#endif
            p.showHelp(1);
        }
        if (ret == KDEsuDialog::AsUser) {
            change_uid = false;
        }
        password = dlg.password();
        keep = dlg.keepPassword();
#if WITH_X11
        data.setSilent(KStartupInfoData::No);
        KStartupInfo::sendChange(id, data);
#endif
    }

    // Some events may need to be handled (like a button animation)
    qApp->processEvents();

    // Run command
    if (!change_uid) {
        int result = system(command.constData());
        result = WEXITSTATUS(result);
        return result;
    } else if (keep && have_daemon) {
        client.setPass(password.toLocal8Bit().constData(), timeout);
        client.setPriority(priority);
        client.setScheduler(scheduler);
        int result = client.exec(command, user, options, env);
        if (result == 0) {
            result = client.exitCode();
            return result;
        }
    } else {
        SuProcess proc;
        proc.setTerminal(terminal);
        proc.setErase(true);
        proc.setUser(user);
        proc.setEnvironment(env);
        proc.setPriority(priority);
        proc.setScheduler(scheduler);
        proc.setCommand(command);
        int result = proc.exec(password.toLocal8Bit().constData());
        return result;
    }
    return -1;
}
