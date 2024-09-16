// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dactiveconnectioninterface.h"
#include <QDBusArgument>

DNETWORKMANAGER_BEGIN_NAMESPACE

DActiveConnectionInterface::DActiveConnectionInterface(const QByteArray &path, QObject *parent)
    : QObject(parent)
{
#ifdef USE_FAKE_INTERFACE
    const QString &Service = QStringLiteral("com.deepin.FakeNetworkManager");
    const QString &Interface = QStringLiteral("com.deepin.FakeNetworkManager.Connection.Active");
    QDBusConnection Connection = QDBusConnection::sessionBus();
#else
    const QString &Service = QStringLiteral("org.freedesktop.NetworkManager");
    const QString &Interface = QStringLiteral("org.freedesktop.NetworkManager.Connection.Active");
    QDBusConnection Connection = QDBusConnection::systemBus();
    Connection.connect(Service, path, Interface, "StateChanged", this, SIGNAL(StateChanged(const quint32, const quint32)));
#endif
    m_inter = new DDBusInterface(Service, path, Interface, Connection, this);
}

QList<QDBusObjectPath> DActiveConnectionInterface::devices() const
{
    return qdbus_cast<QList<QDBusObjectPath>>(m_inter->property("Devices"));
}

bool DActiveConnectionInterface::vpn() const
{
    return qdbus_cast<bool>(m_inter->property("Vpn"));
};

QDBusObjectPath DActiveConnectionInterface::connection() const
{
    return qdbus_cast<QDBusObjectPath>(m_inter->property("Connection"));
};

QDBusObjectPath DActiveConnectionInterface::DHCP4Config() const
{
    return qdbus_cast<QDBusObjectPath>(m_inter->property("Dhcp4Config"));
};

QDBusObjectPath DActiveConnectionInterface::DHCP6Config() const
{
    return qdbus_cast<QDBusObjectPath>(m_inter->property("Dhcp6Config"));
};

QDBusObjectPath DActiveConnectionInterface::IP4Config() const
{
    return qdbus_cast<QDBusObjectPath>(m_inter->property("Ip4Config"));
};

QDBusObjectPath DActiveConnectionInterface::IP6Config() const
{
    return qdbus_cast<QDBusObjectPath>(m_inter->property("Ip6Config"));
};

QDBusObjectPath DActiveConnectionInterface::specificObject() const
{
    return qdbus_cast<QDBusObjectPath>(m_inter->property("SpecificObject"));
};

QString DActiveConnectionInterface::id() const
{
    return qdbus_cast<QString>(m_inter->property("Id"));
};

QString DActiveConnectionInterface::uuid() const
{
    return qdbus_cast<QString>(m_inter->property("Uuid"));
};

QString DActiveConnectionInterface::type() const
{
    return qdbus_cast<QString>(m_inter->property("Type"));
};

quint32 DActiveConnectionInterface::state() const
{
    return qdbus_cast<quint32>(m_inter->property("State"));
};

DNETWORKMANAGER_END_NAMESPACE
