// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "ddeviceInterface.h"

DNETWORKMANAGER_BEGIN_NAMESPACE

DDeviceInterface::DDeviceInterface(const QByteArray &path, QObject *parent)
    : QObject(parent)
{
#ifdef USE_FAKE_INTERFACE
    const QString &Service = QStringLiteral("com.deepin.FakeNetworkManager");
    const QString &deviceInterface = QStringLiteral("com.deepin.FakeNetworkManager.Device");
    const QString &statisticsInterface = QStringLiteral("com.deepin.FakeNetworkManager.Device.Statistics");
    QDBusConnection Connection = QDBusConnection::sessionBus();
#else
    const QString &Service = QStringLiteral("org.freedesktop.NetworkManager");
    const QString &deviceInterface = QStringLiteral("org.freedesktop.NetworkManager.Device");
    const QString &statisticsInterface = QStringLiteral("org.freedesktop.NetworkManager.Device.Statistics");
    QDBusConnection Connection = QDBusConnection::systemBus();
    Connection.connect(Service,
                       path,
                       deviceInterface,
                       "StateChanged",
                       this,
                       SIGNAL(StateChanged(const quint32 newSatate, const quint32 oldState, const quint32 reason)));
#endif
    m_deviceInter = new DDBusInterface(Service, path, deviceInterface, Connection, this);
    m_statisticsInter = new DDBusInterface(Service, path, statisticsInterface, Connection, this);
}

QList<QDBusObjectPath> DDeviceInterface::availableConnections() const
{
    return qdbus_cast<QList<QDBusObjectPath>>(m_deviceInter->property("AvailableConnections"));
}

bool DDeviceInterface::autoconnect() const
{
    return qdbus_cast<bool>(m_deviceInter->property("Autoconnect"));
}

void DDeviceInterface::setAutoconnect(const bool autoconnect) const
{
    m_deviceInter->setProperty("AutoConnect", QVariant::fromValue(autoconnect));
}

bool DDeviceInterface::managed() const
{
    return qdbus_cast<bool>(m_deviceInter->property("Managed"));
}

void DDeviceInterface::setManaged(const bool managed) const
{
    m_deviceInter->setProperty("Managed", QVariant::fromValue(managed));
}

QDBusObjectPath DDeviceInterface::activeConnection() const
{
    return qdbus_cast<QDBusObjectPath>(m_deviceInter->property("ActiveConnection"));
}

QDBusObjectPath DDeviceInterface::DHCP4Config() const
{
    return qdbus_cast<QDBusObjectPath>(m_deviceInter->property("Dhcp4Config"));
}

QDBusObjectPath DDeviceInterface::DHCP6Config() const
{
    return qdbus_cast<QDBusObjectPath>(m_deviceInter->property("Dhcp6Config"));
}

QDBusObjectPath DDeviceInterface::IPv4Config() const
{
    return qdbus_cast<QDBusObjectPath>(m_deviceInter->property("Ip4Config"));
}

QDBusObjectPath DDeviceInterface::IPv6Config() const
{
    return qdbus_cast<QDBusObjectPath>(m_deviceInter->property("Ip6Config"));
}

QString DDeviceInterface::driver() const
{
    return qdbus_cast<QString>(m_deviceInter->property("Driver"));
}

QString DDeviceInterface::interface() const
{
    return qdbus_cast<QString>(m_deviceInter->property("Interface"));
}

QString DDeviceInterface::udi() const
{
    return qdbus_cast<QString>(m_deviceInter->property("Udi"));
}

quint32 DDeviceInterface::deviceType() const
{
    return qdbus_cast<quint32>(m_deviceInter->property("DeviceType"));
}

quint32 DDeviceInterface::interfaceFlags() const
{
    return qdbus_cast<quint32>(m_deviceInter->property("InterfaceFlags"));
}

quint32 DDeviceInterface::state() const
{
    return qdbus_cast<quint32>(m_deviceInter->property("State"));
}

quint32 DDeviceInterface::refreshRateMs() const
{
    return qdbus_cast<quint32>(m_statisticsInter->property("RefreshRateMs"));
}

void DDeviceInterface::setRefreshRateMs(const quint32 newRate) const
{
    m_statisticsInter->setProperty("RefreshRateMs", QVariant::fromValue(newRate));
}

quint64 DDeviceInterface::rxBytes() const
{
    return qdbus_cast<quint64>(m_statisticsInter->property("RxBytes"));
}

quint64 DDeviceInterface::txBytes() const
{
    return qdbus_cast<quint64>(m_statisticsInter->property("TxBytes"));
}

QDBusPendingReply<void> DDeviceInterface::disconnect() const
{
    return m_deviceInter->asyncCall("Disconnect");
}

DNETWORKMANAGER_END_NAMESPACE
