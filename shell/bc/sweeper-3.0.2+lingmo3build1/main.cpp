/*
    SPDX-FileCopyrightText: 2005 Benjamin C Meyer <ben@meyerhome.net>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "sweeper.h"

#include <QApplication>
#include <QCommandLineParser>
#include <QIcon>

#include <KAboutData>
#include <KCrash>
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
#include <Kdelibs4ConfigMigrator>
#endif
#include <KLocalizedString>

#include <config-sweeper.h>

int main(int argc, char *argv[])
{
   QApplication a(argc, argv);
   KLocalizedString::setApplicationDomain("sweeper");
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    Kdelibs4ConfigMigrator migrator(QStringLiteral("sweeper"));
    migrator.setConfigFiles(QStringList() << QStringLiteral("sweeperrc"));
    migrator.setConfigFiles(QStringList() << QStringLiteral("kprivacyrc"));
    migrator.setUiFiles(QStringList() << QStringLiteral("sweeperui.rc"));
    migrator.migrate();
#endif

   KAboutData aboutData(QStringLiteral("sweeper"), i18n("Sweeper"),
                        QStringLiteral(SWEEPER_VERSION),
                        i18n("Helps clean unwanted traces the user leaves on the system."),
                        KAboutLicense::LGPL,
                        i18n("© 2003-2005, Ralf Hoelzer"),
                        QString(),
                        QStringLiteral("https://utils.kde.org/projects/sweeper"));

   aboutData.addAuthor(i18n("Ralf Hoelzer"), i18n("Original author"), QStringLiteral("ralf@well.com"));
   aboutData.addAuthor(i18n("Brian S. Stephan"), i18n("Maintainer"), QStringLiteral("bssteph@irtonline.org"));
   aboutData.addAuthor(i18n("Benjamin Meyer"), i18n("Thumbnail Cache"), QStringLiteral("ben+kdeprivacy@meyerhome.net"));

   // command line
   QCommandLineParser parser;
   KAboutData::setApplicationData(aboutData);
   a.setWindowIcon(QIcon::fromTheme(QStringLiteral("trash-empty")));

   const QCommandLineOption automaticOption(QStringLiteral("automatic"), i18n("Sweeps without user interaction"));
   parser.addOption(automaticOption);
   aboutData.setupCommandLine(&parser);
   parser.process(a);
   aboutData.processCommandLine(&parser);

   KCrash::initialize();

   // Application
   Sweeper *app;
   if (parser.isSet(automaticOption)) {
       app = new Sweeper(true);
   } else {
       app = new Sweeper(false);
       app->show();
   }
   return a.exec();
}

// kate: tab-width 3; indent-mode cstyle; replace-tabs true;
