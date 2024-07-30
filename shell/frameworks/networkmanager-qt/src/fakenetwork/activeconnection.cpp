/*
    SPDX-FileCopyrightText: 2014 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "activeconnection.h"
#include "connection.h"

#include <QDBusConnection>

ActiveConnection::ActiveConnection(QObject *parent)
    : QObject(parent)
    , m_connection(QDBusObjectPath("/"))
    , m_default4(false)
    , m_default6(false)
    , m_dhcp4Config(QDBusObjectPath("/"))
    , m_dhcp6Config(QDBusObjectPath("/"))
    , m_ip4Config(QDBusObjectPath("/"))
    , m_ip6Config(QDBusObjectPath("/"))
    , m_master(QDBusObjectPath("/"))
    , m_specificObject(QDBusObjectPath("/"))
    , m_state(0)
    , m_vpn(false)
{
}

ActiveConnection::~ActiveConnection()
{
}

QDBusObjectPath ActiveConnection::connection() const
{
    return m_connection;
}

bool ActiveConnection::default4() const
{
    return m_default4;
}

bool ActiveConnection::default6() const
{
    return m_default6;
}

QList<QDBusObjectPath> ActiveConnection::devices() const
{
    return m_devices;
}

QDBusObjectPath ActiveConnection::dhcp4Config() const
{
    return m_dhcp4Config;
}

QDBusObjectPath ActiveConnection::dhcp6Config() const
{
    return m_dhcp6Config;
}

QString ActiveConnection::id() const
{
    return m_id;
}

QDBusObjectPath ActiveConnection::ip4Config() const
{
    return m_ip4Config;
}

QDBusObjectPath ActiveConnection::ip6Config() const
{
    return m_ip6Config;
}

QDBusObjectPath ActiveConnection::master() const
{
    return m_master;
}

QDBusObjectPath ActiveConnection::specificObject() const
{
    return m_specificObject;
}

uint ActiveConnection::state() const
{
    return m_state;
}

QString ActiveConnection::type() const
{
    return m_type;
}

bool ActiveConnection::vpn() const
{
    return m_vpn;
}

QString ActiveConnection::uuid() const
{
    return m_uuid;
}

void ActiveConnection::addDevice(const QDBusObjectPath &device)
{
    m_devices << device;
}

void ActiveConnection::removeDevice(const QDBusObjectPath &device)
{
    m_devices.removeAll(device);
}

QString ActiveConnection::activeConnectionPath() const
{
    return m_activeConnectionPath;
}

void ActiveConnection::setActiveConnectionPath(const QString &path)
{
    m_activeConnectionPath = path;
}

void ActiveConnection::setConnection(const QDBusObjectPath &connection)
{
    m_connection = connection;

    Connection *usedConnection = static_cast<Connection *>(QDBusConnection::sessionBus().objectRegisteredAt(connection.path()));
    if (usedConnection) {
        NMVariantMapMap settings = usedConnection->GetSettings();
        setId(settings.value(QLatin1String("connection")).value(QLatin1String("id")).toString());
        setUuid(settings.value(QLatin1String("connection")).value(QLatin1String("uuid")).toString());
        setType(settings.value(QLatin1String("connection")).value(QLatin1String("type")).toString());
    }
}

void ActiveConnection::setDefault4(bool default4)
{
    m_default4 = default4;
}

void ActiveConnection::setDefault6(bool default6)
{
    m_default6 = default6;
}

void ActiveConnection::setDhcp4Config(const QDBusObjectPath &dhcp4Config)
{
    m_dhcp4Config = dhcp4Config;
}

void ActiveConnection::setDhcp6Config(const QDBusObjectPath &dhcp6Config)
{
    m_dhcp6Config = dhcp6Config;
}

void ActiveConnection::setId(const QString &id)
{
    m_id = id;
}

void ActiveConnection::setIpv4Config(const QDBusObjectPath &ipv4Config)
{
    m_ip4Config = ipv4Config;
}

void ActiveConnection::setIpv6Config(const QDBusObjectPath &ipv6Config)
{
    m_ip6Config = ipv6Config;
}

void ActiveConnection::setMaster(const QDBusObjectPath &master)
{
    m_master = master;
}

void ActiveConnection::setSpecificObject(const QDBusObjectPath &specificObject)
{
    m_specificObject = specificObject;
}

void ActiveConnection::setState(uint state)
{
    m_state = state;
}

void ActiveConnection::setType(const QString &type)
{
    m_type = type;

    if (type == QLatin1String("vpn")) {
        m_vpn = true;
    }
}

void ActiveConnection::setUuid(const QString &uuid)
{
    m_uuid = uuid;
}

#include "moc_activeconnection.cpp"
