// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dadapterinterface.h"
#include "btdispatcher.h"
#include "dbluetoothutils.h"

DBLUETOOTH_BEGIN_NAMESPACE

DAdapterInterface::DAdapterInterface(const QString &path, QObject *parent)
    : QObject(parent)
{
#ifndef USE_FAKE_INTERFACE
    const auto &Service = QLatin1String(BlueZService);
    const auto &Connection = QDBusConnection::systemBus();
#else
    const auto &Service = QLatin1String(FakeBlueZService);
    const auto &Connection = QDBusConnection::sessionBus();
#endif
    m_inter = new DDBusInterface(Service, path, QLatin1String(BlueZAdapterInterface), Connection, this);
#ifndef USE_FAKE_INTERFACE
    connect(&BluetoothDispatcher::instance(), &BluetoothDispatcher::adapterRemoved, this, [this](const QDBusObjectPath &adapter) {
        if (m_inter->path() == adapter.path())
            Q_EMIT removed();
    });
    connect(&BluetoothDispatcher::instance(), &BluetoothDispatcher::deviceAdded, this, [this](const QDBusObjectPath &devices) {
        if (devices.path().contains(adapterPath()))
            Q_EMIT deviceAdded(dBusPathToDeviceAddr(devices));
    });
    connect(&BluetoothDispatcher::instance(), &BluetoothDispatcher::deviceRemoved, this, [this](const QDBusObjectPath &devices) {
        if (devices.path().contains(adapterPath()))
            Q_EMIT deviceRemoved(dBusPathToDeviceAddr(devices));
    });
#endif
}

QString DAdapterInterface::address() const
{
    return qdbus_cast<QString>(m_inter->property("Address"));
}

QString DAdapterInterface::addressType() const
{
    return qdbus_cast<QString>(m_inter->property("AddressType"));
}

QString DAdapterInterface::name() const
{
    return qdbus_cast<QString>(m_inter->property("Name"));
}

QString DAdapterInterface::alias() const
{
    return qdbus_cast<QString>(m_inter->property("Alias"));
}

void DAdapterInterface::setAlias(const QString &alias)
{
    m_inter->setProperty("Alias", alias);
}

bool DAdapterInterface::powered() const
{
    return qdbus_cast<bool>(m_inter->property("Powered"));
}

void DAdapterInterface::setPowered(const bool powered)
{
    m_inter->setProperty("Powered", powered);
}

bool DAdapterInterface::discoverable() const
{
    return qdbus_cast<bool>(m_inter->property("Discoverable"));
}

void DAdapterInterface::setDiscoverable(const bool discoverable)
{
    m_inter->setProperty("Discoverable", discoverable);
}

quint32 DAdapterInterface::discoverableTimeout() const
{
    return qdbus_cast<quint32>(m_inter->property("DiscoverableTimeout"));
}

void DAdapterInterface::setDiscoverableTimeout(const quint32 discoverableTimeout)
{
    m_inter->setProperty("DiscoverableTimeout", discoverableTimeout);
}

bool DAdapterInterface::discovering() const
{
    return qdbus_cast<bool>(m_inter->property("Discovering"));
}

QDBusPendingReply<ObjectMap> DAdapterInterface::devices() const
{
    const auto &dispatcher = BluetoothDispatcher::instance();
    return dispatcher.getManagedObjects();
}

QDBusPendingReply<void> DAdapterInterface::removeDevice(const QDBusObjectPath &device) const
{
    return m_inter->asyncCallWithArgumentList("RemoveDevice", {QVariant::fromValue(device)});
}

QDBusPendingReply<void> DAdapterInterface::startDiscovery() const
{
    return m_inter->asyncCall("StartDiscovery");
}

QDBusPendingReply<void> DAdapterInterface::stopDiscovery() const
{
    return m_inter->asyncCall("StopDiscovery");
}

QString DAdapterInterface::adapterPath() const
{
    return m_inter->path();
}

DBLUETOOTH_END_NAMESPACE
