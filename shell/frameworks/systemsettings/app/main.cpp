/**
 * SPDX-FileCopyrightText: 2009 Ben Cooksley <bcooksley@kde.org>
 *
 * This file was sourced from the System Settings package
 * SPDX-FileCopyrightText: 2005 Benjamin C Meyer <ben+systempreferences at meyerhome dot net>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include <KAboutData>
#include <KCrash>
#include <QApplication>
#include <QCommandLineParser>
#include <QSurfaceFormat>

#include <KDBusService>
#include <KLocalizedString>
#include <KPluginMetaData>
#include <KWindowSystem>
#include <iostream>

#include "SettingsBase.h"
#include "kcmmetadatahelpers.h"

int main(int argc, char *argv[])
{
    auto format = QSurfaceFormat::defaultFormat();
    format.setOption(QSurfaceFormat::ResetNotification);
    QSurfaceFormat::setDefaultFormat(format);

    // Make sure the binary name is either kinfocenter or systemsettings,
    // Anything else will just be considered as "systemsettings"
    const QString executableName = QString::fromUtf8(argv[0]);
    QString binaryName = QStringLiteral("systemsettings");
    SidebarMode::ApplicationMode mode = SidebarMode::SystemSettings;
    if (executableName.endsWith(QLatin1String("kinfocenter"))) {
        binaryName = QStringLiteral("kinfocenter");
        mode = SidebarMode::InfoCenter;
    }

    // exec is systemsettings, but we need the QPT to use the right config from the qApp constructor
    // which is before KAboutData::setApplicationData
    QCoreApplication::setApplicationName(binaryName);

    QApplication application(argc, argv);
    // The ki18n application domain must be set before we make any i18n() calls.
    KLocalizedString::setApplicationDomain(QByteArrayLiteral("systemsettings"));

    KAboutData aboutData;

    if (mode == SidebarMode::InfoCenter) {
        // About data
        aboutData = KAboutData(QStringLiteral("kinfocenter"),
                               i18n("Info Center"),
                               QStringLiteral(PROJECT_VERSION),
                               i18n("Centralized and convenient overview of system information."),
                               KAboutLicense::GPL,
                               i18n("(c) 2009, Ben Cooksley"));
        aboutData.setDesktopFileName(QStringLiteral("org.kde.kinfocenter"));

        application.setWindowIcon(QIcon::fromTheme(QStringLiteral("hwinfo")));

    } else {
        aboutData = KAboutData(QStringLiteral("systemsettings"),
                               i18n("System Settings"),
                               QStringLiteral(PROJECT_VERSION),
                               i18n("Central configuration center by KDE."),
                               KAboutLicense::GPL,
                               i18n("(c) 2009, Ben Cooksley"));

        if (qEnvironmentVariableIsSet("KDE_FULL_SESSION")) {
            aboutData.setDesktopFileName(QStringLiteral("systemsettings"));
        } else {
            aboutData.setDesktopFileName(QStringLiteral("kdesystemsettings"));
        }

        application.setWindowIcon(QIcon::fromTheme(QStringLiteral("preferences-system")));
    }

    aboutData.addAuthor(i18n("Ben Cooksley"), i18n("Maintainer"), QStringLiteral("bcooksley@kde.org"));
    aboutData.addAuthor(i18n("Marco Martin"), i18n("Author"), QStringLiteral("mart@kde.org"));
    aboutData.addAuthor(i18n("Mathias Soeken"), i18n("Developer"), QStringLiteral("msoeken@informatik.uni-bremen.de"));
    aboutData.addAuthor(i18n("Will Stephenson"), i18n("Internal module representation, internal module model"), QStringLiteral("wstephenson@kde.org"));

    KAboutData::setApplicationData(aboutData);

    QCoreApplication::setOrganizationDomain(QStringLiteral("kde.org"));

    QCommandLineParser parser;

    parser.addOption(QCommandLineOption(QStringLiteral("list"), i18n("List all possible modules")));
    parser.addPositionalArgument(QStringLiteral("module"), i18n("Configuration module to open"));
    parser.addOption(QCommandLineOption(QStringLiteral("args"), i18n("Arguments for the module"), QStringLiteral("arguments")));

    aboutData.setupCommandLine(&parser);

    parser.process(application);
    aboutData.processCommandLine(&parser);

    if (parser.isSet(QStringLiteral("list"))) {
        std::cout << i18n("The following modules are available:").toLocal8Bit().data() << std::endl;

        auto source = mode == SidebarMode::InfoCenter ? MetaDataSource::KInfoCenter : MetaDataSource::SystemSettings;
        const auto modules = findKCMsMetaData(source) << findExternalKCMModules(source);

        int maxLen = 0;

        for (const auto &metaData : modules) {
            const int len = metaData.pluginId().length();
            if (len > maxLen) {
                maxLen = len;
            }
        }

        for (const auto &metaData : modules) {
            QString entry(QStringLiteral("%1 - %2"));

            entry = entry.arg(metaData.pluginId().leftJustified(maxLen, QLatin1Char(' ')),
                              !metaData.description().isEmpty() ? metaData.description() : i18n("No description available"));

            std::cout << entry.toLocal8Bit().data() << std::endl;
        }
        return 0;
    }

    if (parser.positionalArguments().count() > 1) {
        std::cerr << "Only one module argument may be passed" << std::endl;
        return -1;
    }

    const QStringList args = parser.value(QStringLiteral("args")).split(QRegularExpression(QStringLiteral(" +")), Qt::SkipEmptyParts);
    QString startupModule;

    if (parser.positionalArguments().count() == 1) {
        startupModule = parser.positionalArguments().constFirst();
    }

    if (!args.isEmpty() && startupModule.isEmpty()) {
        std::cerr << "Arguments may only be passed when specifying a module" << std::endl;
        return -1;
    }

    KDBusService service(KDBusService::Unique);

    KCrash::initialize();

    auto mainWindow = new SettingsBase(mode, startupModule, args);

    QObject::connect(&service, &KDBusService::activateRequested, mainWindow, [mainWindow](const QStringList &arguments) {
        // We can't use startupModule and args from above since they come from the existing instance, so we need to parse arguments.
        // We don't need to do the error checking again though.
        QCommandLineParser parser;
        parser.addPositionalArgument(QStringLiteral("module"), i18n("Configuration module to open"));
        parser.addOption(QCommandLineOption(QStringLiteral("args"), i18n("Arguments for the module"), QStringLiteral("arguments")));

        parser.parse(arguments);

        const QStringList args = parser.value(QStringLiteral("args")).split(QRegularExpression(QStringLiteral(" +")), Qt::SkipEmptyParts);
        QString startupModule;

        if (parser.positionalArguments().count() == 1) {
            startupModule = parser.positionalArguments().constFirst();
        }

        if (!startupModule.isEmpty()) {
            mainWindow->setStartupModule(startupModule);
            mainWindow->setStartupModuleArgs(args);
            mainWindow->reloadStartupModule();
        }

        KWindowSystem::updateStartupId(mainWindow->windowHandle());
        KWindowSystem::activateWindow(mainWindow->windowHandle());
    });

    return application.exec();
}
