/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 1999 David Faure <faure@kde.org>
    SPDX-FileCopyrightText: 2002-2003 Waldo Bastian <bastian@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include <kbuildsycoca_p.h>

#include <kservice_version.h>

#include <KAboutData>
#include <KLocalizedString>

#include <QCommandLineOption>
#include <QCommandLineParser>
#include <QCoreApplication>
#include <QDateTime>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QStandardPaths>

#include <qplatformdefs.h> // for unlink
#ifdef Q_OS_WIN
#include <qt_windows.h>
#endif

static void crashHandler(int)
{
    // If we crash while reading sycoca, we delete the database
    // in an attempt to recover.
    if (KBuildSycoca::sycocaPath()) {
        unlink(KBuildSycoca::sycocaPath());
    }
}

#if defined(Q_OS_WIN)
// glue function for calling the unix signal handler from the windows unhandled exception filter
// Inspired from KCrash, but heavily simplified
LONG WINAPI win32UnhandledExceptionFilter(_EXCEPTION_POINTERS *)
{
    crashHandler(0);
    return EXCEPTION_EXECUTE_HANDLER; // allow windows to do the default action (terminate)
}
#endif

void setCrashHandler()
{
#if defined(Q_OS_WIN)
    SetUnhandledExceptionFilter(win32UnhandledExceptionFilter);
#elif !defined(Q_OS_ANDROID)
    sigset_t mask;
    sigemptyset(&mask);

#ifdef SIGSEGV
    signal(SIGSEGV, crashHandler);
    sigaddset(&mask, SIGSEGV);
#endif
#ifdef SIGBUS
    signal(SIGBUS, crashHandler);
    sigaddset(&mask, SIGBUS);
#endif
#ifdef SIGFPE
    signal(SIGFPE, crashHandler);
    sigaddset(&mask, SIGFPE);
#endif
#ifdef SIGILL
    signal(SIGILL, crashHandler);
    sigaddset(&mask, SIGILL);
#endif
#ifdef SIGABRT
    signal(SIGABRT, crashHandler);
    sigaddset(&mask, SIGABRT);
#endif

    sigprocmask(SIG_UNBLOCK, &mask, nullptr);
#endif
}

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);

    KLocalizedString::setApplicationDomain("kservice6");

    KAboutData about(QStringLiteral(KBUILDSYCOCA_EXENAME),
                     i18nc("application name", "KBuildSycoca"),
                     QStringLiteral(KSERVICE_VERSION_STRING),
                     i18nc("application description", "Rebuilds the system configuration cache."),
                     KAboutLicense::GPL,
                     i18nc("@info:credit", "Copyright 1999-2014 KDE Developers"));
    about.addAuthor(i18nc("@info:credit", "David Faure"), i18nc("@info:credit", "Author"), QStringLiteral("faure@kde.org"));
    about.addAuthor(i18nc("@info:credit", "Waldo Bastian"), i18nc("@info:credit", "Author"), QStringLiteral("bastian@kde.org"));
    KAboutData::setApplicationData(about);

    QCommandLineParser parser;
    about.setupCommandLine(&parser);
    parser.addOption(
        QCommandLineOption(QStringLiteral("noincremental"), i18nc("@info:shell command-line option", "Disable incremental update, re-read everything")));
    parser.addOption(QCommandLineOption(QStringLiteral("menutest"), i18nc("@info:shell command-line option", "Perform menu generation test run only")));
    parser.addOption(
        QCommandLineOption(QStringLiteral("track"), i18nc("@info:shell command-line option", "Track menu id for debug purposes"), QStringLiteral("menu-id")));
    parser.addOption(
        QCommandLineOption(QStringLiteral("testmode"), i18nc("@info:shell command-line option", "Switch QStandardPaths to test mode, for unit tests only")));
    parser.process(app);
    about.processCommandLine(&parser);

    const bool bMenuTest = parser.isSet(QStringLiteral("menutest"));

    if (parser.isSet(QStringLiteral("testmode"))) {
        QStandardPaths::setTestModeEnabled(true);
    }

    setCrashHandler();

    fprintf(stderr, "%s running...\n", KBUILDSYCOCA_EXENAME);

    const bool incremental = !parser.isSet(QStringLiteral("noincremental"));

    KBuildSycoca sycoca; // Build data base
    if (parser.isSet(QStringLiteral("track"))) {
        sycoca.setTrackId(parser.value(QStringLiteral("track")));
    }
    sycoca.setMenuTest(bMenuTest);
    if (!sycoca.recreate(incremental)) {
        return -1;
    }

    return 0;
}
