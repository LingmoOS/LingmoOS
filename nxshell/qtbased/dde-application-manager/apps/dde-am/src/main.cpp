// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <QCoreApplication>
#include <QCommandLineOption>
#include <QCommandLineParser>

#include "launcher.h"
#include "global.h"

int main(int argc, char *argv[])
{
    QCoreApplication app{argc, argv};

    QCommandLineParser parser;
    parser.addHelpOption();
    parser.addVersionOption();

    QCommandLineOption listOption("list", "List all appId.");
    parser.addOption(listOption);
    QCommandLineOption launchedByUserOption("by-user",
                                            "Launched by user, it's useful for counting launched times.");
    parser.addOption(launchedByUserOption);
    parser.addPositionalArgument("appId", "Application's ID.");
    parser.addPositionalArgument("action", "Name of the action identifiers for the application, optionally.");

    parser.process(app);
    if (parser.isSet(listOption)) {
        const auto apps = Launcher::appIds();
        if (!apps) {
            qWarning() << apps.error();
            return apps.error().getErrorCode();
        }
        for (const auto &item :apps.value()) {
            qDebug() << qPrintable(item);
        }
        return 0;
    }

    auto arguments = parser.positionalArguments();
    if (arguments.size() < 1)
        parser.showHelp();

    Launcher launcher;
    if (parser.isSet(launchedByUserOption))
        launcher.setLaunchedType(Launcher::ByUser);

    const auto pos1 = arguments.takeFirst();
    QString appPath = pos1.startsWith("/") ? pos1 :
                                             QString("%1/%2").arg(DDEApplicationManager1ObjectPath, escapeToObjectPath(pos1));
    launcher.setPath(appPath);
    if (arguments.size() >= 1) {
        const auto action = arguments.takeFirst();
        launcher.setAction(action);
    }

    auto ret = launcher.run();
    if (!ret) {
        qWarning() << ret.error();
        return ret.error().getErrorCode();
    }
    return 0;
}
