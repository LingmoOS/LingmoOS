// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
// 
// SPDX-License-Identifier: GPL-3.0-or-later
#include <QCoreApplication>
#include <QCommandLineParser>
#include <QDebug>
#include <QStringList>

#include "session/sessiondbusproxymanager.hpp"
#include "system/systemdbusproxymanager.hpp"
#include "lingmoruntime.hpp"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    QCommandLineParser parser;
    parser.setApplicationDescription("ocean-api-proxy-session");
    parser.addHelpOption();
    QCommandLineOption serviceOption({"s", "system"}, "system level services");
    parser.addOption(serviceOption);
    parser.process(a);

    DBusProxyManager *manager;
    if(parser.isSet(serviceOption)){
        if (!LingmoRuntime::CheckStartoceanSession()) {
            qWarning() << "check startocean session error.";
            return 0;
        }

        manager = new SystemDBusProxyManager(QDBusConnection::SystemBus);
    }else{
        manager = new SessionDBusProxyManager(QDBusConnection::SessionBus);
    }

    manager->init();
    manager->createDBusProxy();
    qInfo() << "proxy modules finish to start.";

    return a.exec();
}
