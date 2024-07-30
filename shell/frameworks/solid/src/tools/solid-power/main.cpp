/*
    SPDX-FileCopyrightText: 2014 Alejandro Fiestas Olivares <afiestas@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "solid-power.h"

#include <QCommandLineParser>
#include <QCoreApplication>
#include <QTextStream>

static QTextStream sOut(stdout);
int main(int argc, char **argv)
{
    qputenv("SOLID_POWER_BACKEND", "FREE_DESKTOP");

    QCoreApplication app(argc, argv);
    app.setApplicationName(QLatin1String("solid-power"));

    QCommandLineParser parser;
    parser.setApplicationDescription(QCoreApplication::translate("solid-power", "Tool to know and set the power management state of your device"));
    parser.addHelpOption();
    parser.addPositionalArgument("command", QCoreApplication::translate("solid-power", "Command to execute"));

    QCommandLineOption commands("commands", QCoreApplication::translate("solid-power", "Show available commands"));
    parser.addOption(commands);

    parser.process(app);

    if (parser.isSet(commands)) {
        sOut << Qt::endl << QCoreApplication::translate("solid-hardware", "Syntax:") << Qt::endl << Qt::endl;

        sOut << "  solid-power show" << Qt::endl;
        sOut << QCoreApplication::translate("solid-power",
                                            "             # Show all the power management information from the system.\n"
                                            "             # - acPlugged: whether the device is connected to the AC or not\n")
             << Qt::endl;
        return 1;
    }

    SolidPower power;

    QStringList args = parser.positionalArguments();
    if (args.count() < 1) {
        parser.showHelp(1);
        return 1;
    }

    parser.clearPositionalArguments();

    QString command(args.at(0));

    if (command == QLatin1String("show")) {
        power.show();
    } else if (command == QLatin1String("listen")) {
        sOut << "Listening to events:" << Qt::endl;
        power.listen();
        app.exec();
    } else {
        sOut << "Not recognized command" << Qt::endl;
    }
}
