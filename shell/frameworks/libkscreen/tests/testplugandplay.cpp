/*
 *  SPDX-FileCopyrightText: 2014 Sebastian Kügler <sebas@kde.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "testpnp.h"

#include <QCommandLineParser>
#include <QGuiApplication>

using namespace KScreen;

int main(int argc, char **argv)
{
    QGuiApplication app(argc, argv);

    QCommandLineOption input =
        QCommandLineOption(QStringList() << QStringLiteral("m") << QStringLiteral("monitor"), QStringLiteral("Keep running monitoring for changes"));
    QCommandLineParser parser;
    parser.addHelpOption();
    parser.addOption(input);
    parser.process(app);

    new TestPnp(parser.isSet(input));
    return app.exec();
}
