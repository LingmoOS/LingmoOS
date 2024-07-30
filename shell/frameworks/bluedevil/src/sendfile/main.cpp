/*
 *  SPDX-FileCopyrightText: 2010-2012 Alejandro Fiestas Olivares <afiestas@kde.org>
 *  SPDX-FileCopyrightText: 2010 UFO Coders <info@ufocoders.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "sendfilewizard.h"
#include "version.h"

#include <QApplication>
#include <QCommandLineOption>
#include <QCommandLineParser>

#include <KAboutData>
#include <KDBusService>
#include <KLocalizedString>
#include <KWindowSystem>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    KAboutData aboutData(QStringLiteral("bluedevilsendfile"),
                         i18n("Bluetooth File Transfer"),
                         QStringLiteral(BLUEDEVIL_VERSION_STRING),
                         i18n("Bluetooth File Transfer"),
                         KAboutLicense::GPL,
                         i18n("(c) 2010, UFO Coders"));

    aboutData.addAuthor(QStringLiteral("David Rosca"), //
                        i18n("Maintainer"),
                        QStringLiteral("nowrep@gmail.com"),
                        QStringLiteral("http://david.rosca.cz"));

    aboutData.addAuthor(QStringLiteral("Alejandro Fiestas Olivares"),
                        i18n("Previous Maintainer"),
                        QStringLiteral("afiestas@kde.org"),
                        QStringLiteral("http://www.afiestas.org/"));
    app.setWindowIcon(QIcon::fromTheme(QStringLiteral("preferences-system-bluetooth")));
    app.setQuitOnLastWindowClosed(false);
    app.setQuitLockEnabled(false);

    KAboutData::setApplicationData(aboutData);
    KDBusService service;

    QCommandLineOption kioOption(QStringList() << QStringLiteral("kio") << QStringLiteral("k"));
    kioOption.setDescription(i18n("Specify receiving device by MAC address."));
    kioOption.setValueName(QStringLiteral("bluetooth://address"));

    QCommandLineOption ubiOption(QStringList() << QStringLiteral("ubi") << QStringLiteral("u"));
    ubiOption.setDescription(i18n("Specify receiving device by UBI path."));
    ubiOption.setValueName(QStringLiteral("ubi"));

    QCommandLineOption filesOption(QStringList() << QStringLiteral("files") << QStringLiteral("f"));
    filesOption.setDescription(i18n("Files to be sent."));
    filesOption.setValueName(QStringLiteral("files"));

    QCommandLineParser parser;
    parser.addOption(kioOption);
    parser.addOption(ubiOption);
    parser.addOption(filesOption);
    aboutData.setupCommandLine(&parser);

    parser.process(app);
    aboutData.processCommandLine(&parser);

    QString deviceInfo = parser.value(ubiOption);
    if (deviceInfo.isEmpty()) {
        deviceInfo = parser.value(kioOption);
    }

    SendFileWizard *wizard = new SendFileWizard(deviceInfo, parser.values(filesOption));

    QObject::connect(&service, &KDBusService::activateRequested, wizard, [wizard]() {
        KWindowSystem::updateStartupId(wizard->windowHandle());
        KWindowSystem::activateWindow(wizard->windowHandle());
    });

    return app.exec();
}
