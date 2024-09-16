// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef SYSTEMSERVICE_H
#define SYSTEMSERVICE_H

#include "constants.h"

#include <QObject>

namespace network {
namespace systemservice {

class SystemContainer;

class SystemService : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.deepin.service.SystemNetwork")
    // 1, Qtdbus会自动导出改类的dbus接口
    // 2，添加了"D-Bus Introspection"后，可以指定需要导出到dbus的接口
    // 3，"D-Bus Introspection"可不添加，默认导出所有
    //    Q_CLASSINFO("D-Bus Introspection", ""
    //"  <interface name=\"com.deepin.service.SystemNetwork\">\n"
    //"    <method name=\"IpConflicted\">\n"
    //"      <arg direction=\"out\" type=\"b\"/>\n"
    //"    </method>\n"
    //"    <method name=\"Connectivity\">\n"
    //"      <arg direction=\"out\" type=\"i\"/>\n"
    //"    </method>\n"
    //"  </interface>\n"
    //        "")
    Q_PROPERTY(int Connectivity READ Connectivity NOTIFY ConnectivityChanged)
    Q_PROPERTY(QString PortalUrl READ PortalUrl NOTIFY PortalDetected)

public:
    explicit SystemService(SystemContainer *network, QObject *parent = nullptr);

signals:
    void IpConflictChanged(const QString &devicePath, const QString &ip, bool conflicted);
    void PortalDetected(const QString &url);
    void ConnectivityChanged(int);

private:
    int Connectivity();
    QString PortalUrl();

public slots:
    bool IpConflicted(const QString &devicePath);

private:
    SystemContainer *m_network;
};

}
}

#endif // SERVICE_H
