// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dsystemproxyinterface.h"

DNETWORKMANAGER_BEGIN_NAMESPACE

DSystemProxyInterface::DSystemProxyInterface(QObject *parent)
    : QObject(parent)
{
#ifdef USE_FAKE_INTERFACE
    const QString &Service = QStringLiteral("com.deepin.daemon.FakeNetwork");
    const QString &Interface = QStringLiteral("com.deepin.daemon.FakeNetwork.Network");
    const QString &Path = QStringLiteral("/com/deepin/daemon/FakeNetwork/Network");
    QDBusConnection Connection = QDBusConnection::sessionBus();
#else
    const QString &Service = QStringLiteral("com.deepin.daemon.Network");
    const QString &Interface = QStringLiteral("com.deepin.daemon.Network");
    const QString &Path = QStringLiteral("/com/deepin/daemon/Network");
    QDBusConnection Connection = QDBusConnection::sessionBus();
#endif
    m_inter = new DDBusInterface(Service, Path, Interface, Connection, this);
}

QDBusPendingReply<QString> DSystemProxyInterface::getAutoProxy() const
{
    return m_inter->asyncCall("GetAutoProxy");
}

QDBusPendingReply<QString, QString> DSystemProxyInterface::getProxy(const QString &proxyType) const
{
    return m_inter->asyncCallWithArgumentList("GetProxy", {QVariant::fromValue(proxyType)});
}

QDBusPendingReply<QString> DSystemProxyInterface::getProxyIgnoreHosts() const
{
    return m_inter->asyncCall("GetProxyIgnoreHosts");
}

QDBusPendingReply<QString> DSystemProxyInterface::getProxyMethod() const
{
    return m_inter->asyncCall("GetProxyMethod");
}

QDBusPendingReply<void> DSystemProxyInterface::setAutoProxy(const QString &proxyAuto) const
{
    return m_inter->asyncCallWithArgumentList("SetAutoProxy", {QVariant::fromValue(proxyAuto)});
}

QDBusPendingReply<void> DSystemProxyInterface::setProxy(const QString &proxyType, const QString &host, const QString &port) const
{
    return m_inter->asyncCallWithArgumentList(
        "SetProxy", {QVariant::fromValue(proxyType), QVariant::fromValue(host), QVariant::fromValue(port)});
}

QDBusPendingReply<void> DSystemProxyInterface::setProxyIgnoreHosts(const QString &ignoreHosts) const
{
    return m_inter->asyncCallWithArgumentList("SetProxyIgnoreHosts", {QVariant::fromValue(ignoreHosts)});
}

QDBusPendingReply<void> DSystemProxyInterface::setProxyMethod(const QString &method) const
{
    return m_inter->asyncCallWithArgumentList("SetProxyMethod", {QVariant::fromValue(method)});
}

DNETWORKMANAGER_END_NAMESPACE
