// SPDX-FileCopyrightText: 2011 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
#include "proxycontrollernm.h"

const static QString networkService         = "com.deepin.daemon.Network";
const static QString proxyChainsPath        = "/com/deepin/daemon/Network/ProxyChains";
const static QString proxyChainsInterface   = "com.deepin.daemon.Network.ProxyChains";

using namespace dde::network;

ProxyController_NM::ProxyController_NM(QObject *parent)
    : ProxyController(parent)
{
    initMember();
    initConnection();
}

void ProxyController_NM::setAppProxy(const AppProxyConfig &config)
{
    QDBusInterface dbusInter(networkService, proxyChainsPath, proxyChainsInterface);
    dbusInter.call("Set", appProxyType(config.type), config.ip, config.port, config.username, config.password);
}

void ProxyController_NM::setAppProxyEnabled(bool enabled)
{
    QDBusInterface dbusInter(networkService, proxyChainsPath, proxyChainsInterface);
    dbusInter.call("SetEnable", enabled);
}

bool ProxyController_NM::appProxyEnabled() const
{
    QDBusInterface dbusInter(networkService, proxyChainsPath, proxyChainsInterface);
    return dbusInter.property("Enable").toBool();
}

void ProxyController_NM::initMember()
{
    QDBusInterface dbusInter(networkService, proxyChainsPath, proxyChainsInterface);
    onTypeChanged(dbusInter.property("Type").toString());
    onIPChanged(dbusInter.property("IP").toString());
    onPortChanged(dbusInter.property("Port").toUInt());
    onUserChanged(dbusInter.property("User").toString());
    onPasswordChanged(dbusInter.property("Password").toString());
}

void ProxyController_NM::initConnection()
{
    QDBusConnection::sessionBus().connect(networkService,
                                         proxyChainsPath,
                                         "org.freedesktop.DBus.Properties",
                                         "PropertiesChanged",
                                         this,
                                         SLOT(onPropertiesChanged(const QString &, const QVariantMap &)));
}

void ProxyController_NM::onPropertiesChanged(const QString &interface, const QVariantMap &values)
{
    if (interface != "com.deepin.daemon.Network.ProxyChains")
        return;

    if (values.contains("Enable")) {
        appEnableChanged(values.value("Enable").toBool());
    }
    if (values.contains("Type")) {
        onTypeChanged(values.value("Type").toString());
    }
    if (values.contains("IP")) {
        onIPChanged(values.value("IP").toString());
    }
    if (values.contains("Port")) {
        onPortChanged(values.value("Port").toUInt());
    }
    if (values.contains("User")) {
        onUserChanged(values.value("User").toString());
    }
    if (values.contains("Password")) {
        onPasswordChanged(values.value("Password").toString());
    }
}
