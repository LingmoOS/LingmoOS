/* This file is part of the KDE project
   SPDX-FileCopyrightText: 2001-2022 Christoph Cullmann <cullmann@kde.org>
   SPDX-FileCopyrightText: 2001 Joseph Wenninger <jowenn@kde.org>
   SPDX-FileCopyrightText: 2001 Anders Lund <anders.lund@lund.tdcadsl.dk>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kateapp.h"

#include <KAboutData>
#include <KDBusService>
#include <KLocalizedString>

#include <QApplication>
#include <QCommandLineParser>
#include <QStandardPaths>

#ifndef Q_OS_WIN
#include <unistd.h>
#endif

#include <iostream>

extern "C" Q_DECL_EXPORT int main(int argc, char **argv)
{
#if !defined(Q_OS_WIN) && !defined(Q_OS_HAIKU)
    // Prohibit using sudo or kdesu (but allow using the root user directly)
    if (getuid() == 0) {
        if (!qEnvironmentVariableIsEmpty("SUDO_USER")) {
            std::cout << "Running Notepad with sudo can cause bugs and expose you to security vulnerabilities. "
                         "Instead use Notepad normally and you will be prompted for elevated privileges when "
                         "saving documents if needed."
                      << std::endl;
            return EXIT_FAILURE;
        } else if (!qEnvironmentVariableIsEmpty("KDESU_USER")) {
            std::cout << "Running Notepad with kdesu can cause bugs and expose you to security vulnerabilities. "
                         "Instead use Notepad normally and you will be prompted for elevated privileges when "
                         "saving documents if needed."
                      << std::endl;
            return EXIT_FAILURE;
        }
    }
#endif

#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
    /**
     * enable high dpi support
     */
    QCoreApplication::setAttribute(Qt::AA_UseHighDpiPixmaps, true);
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling, true);
#endif

    /**
     * allow fractional scaling
     * we only activate this on Windows, it seems to creates problems on unices
     * (and there the fractional scaling with the QT_... env vars as set by KScreen works)
     * see bug 416078
     *
     * we switched to Qt::HighDpiScaleFactorRoundingPolicy::RoundPreferFloor because of font rendering issues
     * we follow what Krita does here, see https://invent.kde.org/graphics/krita/-/blob/master/krita/main.cc
     * we raise the Qt requirement to  5.15 as it seems some patches went in after 5.14 that are needed
     * see Krita comments, too
     */
#if defined(Q_OS_WIN)
    QGuiApplication::setHighDpiScaleFactorRoundingPolicy(Qt::HighDpiScaleFactorRoundingPolicy::RoundPreferFloor);
#endif

    /**
     * Create application first
     * Enforce application name even if the executable is renamed
     */
    QApplication app(argc, argv);
    app.setApplicationName(QStringLiteral("notepad"));

    /**
     * Connect application with translation catalogs, Kate & Notepad share the same one
     */
    KLocalizedString::setApplicationDomain("kate");

    /**
     * then use i18n and co
     */
    KAboutData aboutData(QStringLiteral("notepad"),
                         i18n("Notepad"),
                         QStringLiteral(KATE_VERSION),
                         i18n("Notepad - Text Editor"),
                         KAboutLicense::LGPL_V2,
                         i18n("(c) 2025 Lingmo OS Team"),
                         QStringLiteral("<img height=\"80\" width=\"80\" src=\":/notepad/notepad.svg\"/>"),
                         QStringLiteral("https://lingmo.org"));

    /**
     * right dbus prefix == org.kde.
     */
    aboutData.setOrganizationDomain(QByteArray("lingmo.org"));

    /**
     * desktop file association to make application icon work (e.g. in Wayland window decoration)
     */
    aboutData.setDesktopFileName(QStringLiteral("org.lingmo.notepad"));

    /**
     * set proper Notepad icon for our about dialog
     */
    aboutData.setProgramLogo(QIcon::fromTheme(QStringLiteral("accessories-text-editor")));

    /**
     * set and register app about data
     */
    KAboutData::setApplicationData(aboutData);

    /**
     * set the program icon
     */
#ifndef Q_OS_MACOS // skip this on macOS to have proper mime-type icon visible
    QApplication::setWindowIcon(QIcon::fromTheme(QStringLiteral("accessories-text-editor")));
#endif

    /**
     * Create command line parser and feed it with known options
     */
    QCommandLineParser parser;
    aboutData.setupCommandLine(&parser);

    // -e/--encoding option
    const QCommandLineOption useEncoding(QStringList() << QStringLiteral("e") << QStringLiteral("encoding"),
                                         i18n("Set encoding for the file to open."),
                                         i18n("encoding"));
    parser.addOption(useEncoding);

    // -l/--line option
    const QCommandLineOption gotoLine(QStringList() << QStringLiteral("l") << QStringLiteral("line"), i18n("Navigate to this line."), i18n("line"));
    parser.addOption(gotoLine);

    // -c/--column option
    const QCommandLineOption gotoColumn(QStringList() << QStringLiteral("c") << QStringLiteral("column"), i18n("Navigate to this column."), i18n("column"));
    parser.addOption(gotoColumn);

    // -i/--stdin option
    const QCommandLineOption readStdIn(QStringList() << QStringLiteral("i") << QStringLiteral("stdin"), i18n("Read the contents of stdin."));
    parser.addOption(readStdIn);

    // --tempfile option
    const QCommandLineOption tempfile(QStringList() << QStringLiteral("tempfile"), i18n("The files/URLs opened by the application will be deleted after use"));
    parser.addOption(tempfile);

    // urls to open
    parser.addPositionalArgument(QStringLiteral("urls"), i18n("Documents to open."), i18n("[urls...]"));

    /**
     * do the command line parsing
     */
    parser.process(app);

    /**
     * handle standard options
     */
    aboutData.processCommandLine(&parser);

    /**
     * construct the real kate app object ;)
     * behaves like a singleton, one unique instance
     * we are passing our local command line parser to it
     */
    KateApp kateApp(parser,
                    KateApp::ApplicationNotepad,
                    QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + QStringLiteral("/notepad/sessions"));

    /**
     * init kate
     * if this returns false, we shall exit
     * else we may enter the main event loop
     */
    if (!kateApp.init()) {
        return 0;
    }

    /**
     * finally register this notepad instance for dbus, don't die if no dbus is around!
     */
    const KDBusService dbusService(KDBusService::Multiple | KDBusService::NoExitOnFailure);

    /**
     * Run the event loop
     */
    return app.exec();
}
