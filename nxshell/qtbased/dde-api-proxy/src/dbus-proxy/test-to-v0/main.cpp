// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
// 
// SPDX-License-Identifier: GPL-3.0-or-later


#include <QCoreApplication>
#include <QCommandLineParser>

#include "system/com_deepin_daemon_Accounts.hpp"

// #include "session/soundeffect.hpp"
// #include "session/dbusdemo2.hpp"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    QCommandLineParser parser;
    parser.setApplicationDescription("dde-api-proxy");
    parser.addHelpOption();
    QCommandLineOption systemOption(QStringList() << "S" << "system", "system proxy.");
    QCommandLineOption sessionption(QStringList() << "s" << "session", "session proxy.");
    parser.addOption(systemOption);
    parser.addOption(sessionption);
    parser.process(a);

    if(parser.isSet(systemOption)) {
        qInfo() << "dbus system proxy.";
        new SystemAccountsProxy("com.deepin.daemon.Accounts", "/com/deepin/daemon/Accounts", "com.deepin.daemon.Accounts",
            "com.test.daemon.Accounts", "/com/test/daemon/Accounts", "com.test.daemon.Accounts", QDBusConnection::SystemBus);
    } else if (parser.isSet(sessionption)) {
        qInfo() << "dbus session proxy.";

        // new SoundEffect();
        // new DBusProxy2("com.deepin.dbusdemo", "/com/deepin/dbusdemo", "com.deepin.dbusdemo",
        //     "com.test.dbusdemo", "/com/test/dbusdemo", "com.test.dbusdemo");
    } else {
        qWarning() << "param error.";
        return -1;
    }

    return a.exec();
}
