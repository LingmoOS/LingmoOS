/*
 *   SPDX-FileCopyrightText: 2010-2013 Alejandro Fiestas Olivares <afiestas@kde.org>
 *   SPDX-FileCopyrightText: 2010 UFO Coders <info@ufocoders.com>
 *
 *   SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "bluewizard.h"
#include "version.h"

#include <QApplication>
#include <QCommandLineParser>
#include <QIcon>
#include <QUrl>

#include <KAboutData>
#include <KDBusService>
#include <KLocalizedString>
#include <KWindowSystem>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    KAboutData aboutData(QStringLiteral("bluedevilwizard"),
                         i18n("Add Bluetooth Device"),
                         QStringLiteral(BLUEDEVIL_VERSION_STRING),
                         i18n("Add Bluetooth Device"),
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

    KAboutData::setApplicationData(aboutData);
    KDBusService service(KDBusService::Unique);

    QCommandLineParser parser;
    aboutData.setupCommandLine(&parser);

    parser.process(app);
    aboutData.processCommandLine(&parser);

    BlueWizard *wizard = new BlueWizard;

    QObject::connect(&service, &KDBusService::activateRequested, wizard, [wizard]() {
        KWindowSystem::updateStartupId(wizard->windowHandle());
        KWindowSystem::activateWindow(wizard->windowHandle());
    });

    return app.exec();
}
