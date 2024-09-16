// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "pluginmanager.h"
#include "servicemanager.h"

#include <QCommandLineOption>
#include <QCommandLineParser>
#include <QCoreApplication>
#include <QDebug>
#include <QLoggingCategory>

#include <unistd.h>
Q_LOGGING_CATEGORY(dsm_Main, "[Main]")

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    a.setApplicationVersion(VERSION);

    QCommandLineOption groupOption({ { "g", "group" }, "eg:core", "group name" });
    QCommandLineOption nameOption({ { "n", "name" }, "eg:org.deepin.demo", "service name" });
    QCommandLineParser parser;
    parser.setApplicationDescription("deepin service plugin loader");
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addOption(groupOption);
    parser.addOption(nameOption);
    parser.process(a);

    const bool isSetGroup = parser.isSet(groupOption);
    const bool isSetName = parser.isSet(nameOption);

    const QString &typeValue = getuid() < 1000 ? "system" : "user";
    const QString &groupValue = isSetGroup ? parser.value(groupOption) : QString();
    const QString &nameValue = isSetName ? parser.value(nameOption) : QString();

    qCDebug(dsm_Main) << "deepin service config dir:" << QString(SERVICE_CONFIG_DIR);

    QMap<QString, QDBusConnection::BusType> busTypeMap;
    busTypeMap["system"] = QDBusConnection::SystemBus;
    busTypeMap["user"] = QDBusConnection::SessionBus;
    if (isSetName) {
        PluginManager *srv = new PluginManager();
        srv->init(busTypeMap[typeValue]);
        srv->loadByName(nameValue);
    } else if (isSetGroup) {
        PluginManager *srv = new PluginManager();
        srv->init(busTypeMap[typeValue]);
        srv->loadByGroup(groupValue);
    } else {
        ServiceManager *srv = new ServiceManager();
        srv->init(busTypeMap[typeValue]);
    }

    return a.exec();
}
