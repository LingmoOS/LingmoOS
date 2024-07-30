/*
 *   SPDX-FileCopyrightText: 2006 Aaron Seigo <aseigo@kde.org>
 *
 *   SPDX-License-Identifier: GPL-2.0-or-later
 */

#include <iostream>

#include <KAboutData>
#include <KLocalizedString>
#include <KPluginMetaData>
#include <QApplication>

#include <Plasma5Support/PluginLoader>
#include <qcommandlineoption.h>
#include <qcommandlineparser.h>

#include "engineexplorer.h"

void listEngines()
{
    int maxLen = 0;
    QMap<QString, QString> engines;
    const auto plugins = Plasma5Support::PluginLoader::listDataEngineMetaData();
    for (const KPluginMetaData &info : plugins) {
        int len = info.pluginId().length();
        if (len > maxLen) {
            maxLen = len;
        }

        QString name = info.pluginId();
        QString comment = info.description();

        if (comment.isEmpty()) {
            comment = i18n("No description available");
        }

        engines.insert(name, comment);
    }

    QMap<QString, QString>::const_iterator it;
    for (it = engines.constBegin(); it != engines.constEnd(); ++it) {
        QString engine("%1 - %2");
        engine = engine.arg(it.key().leftJustified(maxLen, ' ')).arg(it.value());
        std::cout << engine.toLocal8Bit().data() << std::endl;
    }
}

int main(int argc, char **argv)
{
    QApplication app(argc, argv);

    KLocalizedString::setApplicationDomain("plasmaengineexplorer");

    KAboutData aboutData("plasmaengineexplorer",
                         i18n("Plasma Engine Explorer"),
                         PROJECT_VERSION,
                         i18n("Explore the data published by Plasma DataEngines"),
                         KAboutLicense::GPL,
                         i18n("(c) 2006, The KDE Team"));
    aboutData.addAuthor(i18n("Aaron J. Seigo"), i18n("Author and maintainer"), "aseigo@kde.org");
    QApplication::setWindowIcon(QIcon::fromTheme(QStringLiteral("plasma"), app.windowIcon()));

    KAboutData::setApplicationData(aboutData);

    QCommandLineParser parser;
    aboutData.setupCommandLine(&parser);
    parser.addOption(QCommandLineOption(QStringList() << "list", i18n("Displays a list of known engines and their descriptions")));
    parser.addOption(QCommandLineOption(QStringList() << "height", i18n("The desired height in pixels"), "pixels"));
    parser.addOption(QCommandLineOption(QStringList() << "width", i18n("The desired width in pixels"), "pixels"));
    parser.addOption(QCommandLineOption(QStringList() << "x", i18n("The desired x position in pixels"), "pixels"));
    parser.addOption(QCommandLineOption(QStringList() << "y", i18n("The desired y position in pixels"), "pixels"));
    parser.addOption(QCommandLineOption(QStringList() << "engine", i18n("The data engine to use"), "data engine"));
    parser.addOption(QCommandLineOption(QStringList() << "source", i18n("The source to request"), "data engine"));
    parser.addOption(QCommandLineOption(QStringList() << "interval", i18n("Update interval in milliseconds"), "ms"));
    parser.addOption(QCommandLineOption(QStringList() << "app",
                                        i18n("Only show engines associated with the parent application; "
                                             "maps to the X-KDE-ParentApp entry in the DataEngine's .desktop file."),
                                        "application"));

    parser.process(app);
    aboutData.processCommandLine(&parser);

    if (parser.isSet("list")) {
        listEngines();
        return 0;
    }

    EngineExplorer *w = new EngineExplorer;

    bool ok1, ok2 = false;
    // get size
    int x = parser.value("height").toInt(&ok1);
    int y = parser.value("width").toInt(&ok2);
    if (ok1 && ok2) {
        w->resize(x, y);
    }

    // get pos if available
    x = parser.value("x").toInt(&ok1);
    y = parser.value("y").toInt(&ok2);
    if (ok1 && ok2) {
        w->move(x, y);
    }

    // set interval
    int interval = parser.value("interval").toInt(&ok1);
    if (ok1) {
        w->setInterval(interval);
    }

    // set engine
    QString engine = parser.value("engine");
    if (!engine.isEmpty()) {
        w->setEngine(engine);

        QString source = parser.value("source");
        if (!source.isEmpty()) {
            w->requestSource(source);
        }
    }

    if (parser.isSet("app")) {
        w->setApp(parser.value("app"));
    }

    w->show();
    return app.exec();
}
