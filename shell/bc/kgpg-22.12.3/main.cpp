/*
    SPDX-FileCopyrightText: 2002 Jean-Baptiste Mardelle <bj@altern.org>
    SPDX-FileCopyrightText: 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016 Rolf Eike Beer <kde@opensource.sf-tec.de>
    SPDX-FileCopyrightText: 2016 Andrius Štikonas <andrius@stikonas.eu>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "kgpg.h"

#include <QCommandLineParser>
#include <QDir>

#include <KAboutData>
#include <KCrash>
#include <KDBusService>
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
#include <Kdelibs4ConfigMigrator>
#endif
#include <KLocalizedString>

int main(int argc, char *argv[])
{
    KGpgApp app(argc, argv);
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    app.setAttribute(Qt::AA_UseHighDpiPixmaps, true);
#endif
    KCrash::initialize();
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    Kdelibs4ConfigMigrator migrate(QLatin1String("kgpg"));
    migrate.setConfigFiles({ QLatin1String("kgpgrc") });
    migrate.setUiFiles({ QStringLiteral("keysmanager.rc"), QStringLiteral("kgpgeditor.rc") });
    migrate.migrate();
#endif

    KLocalizedString::setApplicationDomain("kgpg");

    KAboutData about (
        QLatin1String("kgpg"),
        xi18nc("@title", "<application>KGpg</application>"),
        QLatin1String(KGPG_VERSION),
        xi18nc("@title", "KGpg - simple gui for GnuPG"),
        KAboutLicense::GPL,
        xi18nc("@info:credit", "&copy; 2003-2016, The KGpg Developers"));

    about.addAuthor(xi18nc("@info:credit", "Rolf Eike Beer"), i18nc("@info:credit", "Maintainer"), QStringLiteral("kde@opensource.sf-tec.de"));
    about.addAuthor(xi18nc("@info:credit", "Jean-Baptiste Mardelle"), i18nc("@info:credit", "Author and former maintainer"), QStringLiteral("bj@altern.org"));
    about.addAuthor(xi18nc("@info:credit", "Jimmy Gilles"), QString(), QStringLiteral("jimmygilles@gmail.com"));
    about.addAuthor(xi18nc("@info:credit", "Andrius Štikonas"), i18nc("@info:credit", "KF5 port"), QStringLiteral("andrius@stikonas.eu"));

    about.setHomepage(QLatin1String("https://utils.kde.org/projects/kgpg"));

    about.setOrganizationDomain(QByteArray("kde.org"));
    about.setProductName(QByteArray("kgpg"));

    KAboutData::setApplicationData(about);

    QCommandLineParser parser;
    parser.setApplicationDescription(about.shortDescription());
    about.setupCommandLine(&parser);
    app.setupCmdlineParser(parser);

    parser.process(app);
    about.processCommandLine(&parser);

    app.setQuitOnLastWindowClosed(false);
    KDBusService service(KDBusService::Unique);

	service.connect(&service, &KDBusService::activateRequested, &app, &KGpgApp::slotDBusActivation);

	if(!app.newInstance())
		return 1;
	app.handleArguments(parser, QDir::current());

    return app.exec();
}
