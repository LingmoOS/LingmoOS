// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dappproxyinterface.h"
#include <QDBusArgument>

DNETWORKMANAGER_BEGIN_NAMESPACE

DAppProxyInterface::DAppProxyInterface(QObject *parent)
    : QObject(parent)
{
#ifdef USE_FAKE_INTERFACE
    const QString &Service = QStringLiteral("com.deepin.daemon.FakeNetwork");
    const QString &Interface = QStringLiteral("com.deepin.daemon.FakeNetwork.ProxyChains");
    const QString &Path = QStringLiteral("/com/deepin/daemon/FakeNetwork/ProxyChains");
    QDBusConnection Connection = QDBusConnection::sessionBus();
#else
    const QString &Service = QStringLiteral("com.deepin.daemon.Network");
    const QString &Interface = QStringLiteral("com.deepin.daemon.Network.ProxyChains");
    const QString &Path = QStringLiteral("/com/deepin/daemon/Network/ProxyChains");
    QDBusConnection Connection = QDBusConnection::sessionBus();
#endif
    m_inter = new DDBusInterface(Service, Path, Interface, Connection, this);
}

QString DAppProxyInterface::IP() const
{
    return qdbus_cast<QString>(m_inter->property("IP"));
}

QString DAppProxyInterface::password() const
{
    return qdbus_cast<QString>(m_inter->property("Password"));
}

QString DAppProxyInterface::type() const
{
    return qdbus_cast<QString>(m_inter->property("Type"));
}

QString DAppProxyInterface::user() const
{
    return qdbus_cast<QString>(m_inter->property("User"));
}

quint32 DAppProxyInterface::port() const
{
    return qdbus_cast<quint32>(m_inter->property("Port"));
}

QDBusPendingReply<void> DAppProxyInterface::set(
    const QString &type, const QString &ip, const quint32 port, const QString &user, const QString &password) const
{
    return m_inter->asyncCallWithArgumentList("Set",
                                              {QVariant::fromValue(type),
                                               QVariant::fromValue(ip),
                                               QVariant::fromValue(port),
                                               QVariant::fromValue(user),
                                               QVariant::fromValue(password)});
}

DNETWORKMANAGER_END_NAMESPACE
