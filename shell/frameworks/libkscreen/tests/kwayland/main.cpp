/*
 *  SPDX-FileCopyrightText: 2014-2016 Sebastian Kügler <sebas@kde.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "waylandtestserver.h"

#include <QCommandLineParser>
#include <QCoreApplication>

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);

    KScreen::WaylandTestServer server;

    QCommandLineOption config =
        QCommandLineOption(QStringList() << QStringLiteral("c") << QStringLiteral("config"), QStringLiteral("Config file"), QStringLiteral("config"));
    QCommandLineParser parser;
    parser.addHelpOption();
    parser.addOption(config);
    parser.process(app);

    if (parser.isSet(config)) {
        server.setConfig(parser.value(config));
    } else {
        server.setConfig(QString::fromLocal8Bit(TEST_DATA) + QLatin1String("/multipleoutput.json"));
    }
    server.start();
    return app.exec();
}
