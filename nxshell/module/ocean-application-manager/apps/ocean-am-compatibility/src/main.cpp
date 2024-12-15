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
    parser.process(app);

    auto arguments = parser.positionalArguments();
    if (arguments.size() != 2){
        qWarning() << "args size must 2";
        return -1;
    }

    if(arguments[1] == "success"){
        qWarning() << "not need compatibility";
        return 0;
    }

    Launcher launcher;

    const auto pos1 = arguments.takeFirst();
    auto info = processUnitName(pos1);
    QString appPath = pos1.startsWith("/") ? pos1 :
                                             QString("%1/%2").arg(OCEANApplicationManager1ObjectPath, escapeToObjectPath(info.applicationID));
    launcher.setPath(appPath);
    auto ret = launcher.run();
    if (!ret) {
        qWarning() << ret.error();
        return ret.error().getErrorCode();
    }
    return 0;
}
