/*
    SPDX-FileCopyrightText: 2021 David Edmundson <davidedmundson@kde.org>
    SPDX-FileCopyrightText: 2021 Alexander Lohnau <alexander.lohnau@gmx.de>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "interactiveconsole.h"
#include <QApplication>
#include <QCommandLineParser>
#include <QDebug>

int main(int argc, char **argv)
{
    QApplication app(argc, argv);
    InteractiveConsole::ConsoleMode mode = InteractiveConsole::LingmoConsole;

    QCommandLineParser parser;
    QCommandLineOption lingmoOpt(QStringLiteral("lingmo"));
    QCommandLineOption kwinOpt(QStringLiteral("kwin"));
    parser.addOption(lingmoOpt);
    parser.addOption(kwinOpt);
    parser.addHelpOption();
    parser.process(app);
    if (parser.isSet(lingmoOpt) && parser.isSet(kwinOpt)) {
        qWarning() << "Only one mode can be specified when launching the interactive console";
        exit(1);
    } else if (parser.isSet(kwinOpt)) {
        mode = InteractiveConsole::KWinConsole;
    } else if (parser.isSet(lingmoOpt)) {
        mode = InteractiveConsole::LingmoConsole;
    }
    // set to delete on close
    auto console = new InteractiveConsole(mode);
    console->show();
    return app.exec();
}
