/*
    SPDX-FileCopyrightText: 2014 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "device.h"
#include "activeconnection.h"

#include <QDBusConnection>
#include <QDBusMetaType>

Device::Device(QObject *parent)
    : QObject(parent)
    , m_activeConnection(QDBusObjectPath("/"))
    , m_autoconnect(true)
    , m_capabilities(0)
    , m_deviceType(0)
    , m_dhcp4Config(QDBusObjectPath("/"))
    , m_dhcp6Config(QDBusObjectPath("/"))
    , m_firmwareMissing(false)
    , m_interfaceFlags(NM_DEVICE_INTERFACE_FLAG_NONE)
    , m_ip4Address(0)
    , m_ip4Config(QDBusObjectPath("/"))
    , m_ip6Config(QDBusObjectPath("/"))
    , m_managed(true)
    , m_mtu(0)
    , m_state(30)
{
    qDBusRegisterMetaType<DeviceDBusStateReason>();
}

Device::~Device()
{
}

QDBusObjectPath Device::activeConnection() const
{
    return m_activeConnection;
}

bool Device::autoconnect() const
{
    return m_autoconnect;
}

void Device::setAutoconnect(bool autoconnect)
{
    m_autoconnect = autoconnect;
}

QList<QDBusObjectPath> Device::availableConnections() const
{
    return m_availableConnections;
}

uint Device::capabilities() const
{
    return m_capabilities;
}

QString Device::deviceInterface() const
{
    if (m_deviceType == NetworkManager::Device::Ethernet) {
        return QLatin1String("org.kde.fakenetwork.Device.Wired");
    } else if (m_deviceType == NetworkManager::Device::Wifi) {
        return QLatin1String("org.kde.fakenetwork.Device.Wireless");
    }

    return QLatin1String("org.kde.fakenetwork.Device.Wired");
}

QString Device::devicePath() const
{
    return m_devicePath;
}

void Device::setDevicePath(const QString &devicePath)
{
    m_devicePath = devicePath;
}

uint Device::deviceType() const
{
    return m_deviceType;
}

QDBusObjectPath Device::dhcp4Config() const
{
    return m_dhcp4Config;
}

QDBusObjectPath Device::dhcp6Config() const
{
    return m_dhcp6Config;
}

QString Device::driver() const
{
    return m_driver;
}

QString Device::driverVersion() const
{
    return m_driverVersion;
}

bool Device::firmwareMissing() const
{
    return m_firmwareMissing;
}

QString Device::firmwareVersion() const
{
    return m_firmwareVersion;
}

QString Device::interface() const
{
    return m_interface;
}

int Device::ip4Address() const
{
    return m_ip4Address;
}

QDBusObjectPath Device::ip4Config() const
{
    return m_ip4Config;
}

QDBusObjectPath Device::ip6Config() const
{
    return m_ip6Config;
}

QString Device::ipInterface() const
{
    return m_ipInterface;
}

uint Device::interfaceFlags() const
{
    return m_interfaceFlags;
}

bool Device::managed() const
{
    return m_managed;
}

uint Device::mtu() const
{
    return m_mtu;
}

uint Device::state() const
{
    return m_state;
}

DeviceDBusStateReason Device::stateReason() const
{
    return m_stateReason;
}

QString Device::udi() const
{
    return m_udi;
}

void Device::addAvailableConnection(const QDBusObjectPath &availableConnection)
{
    m_availableConnections << availableConnection;
}

void Device::removeAvailableConnection(const QDBusObjectPath &availableConnection)
{
    m_availableConnections.removeAll(availableConnection);
}

void Device::setActiveConnection(const QString &activeConnection)
{
    m_activeConnection = QDBusObjectPath(activeConnection);
}

void Device::setCapabilities(uint capabilities)
{
    m_capabilities = capabilities;
}

void Device::setDeviceType(uint deviceType)
{
    m_deviceType = deviceType;
}

void Device::setDhcp4Config(const QString &config)
{
    m_dhcp4Config = QDBusObjectPath(config);
}

void Device::setDhcp6Config(const QString &config)
{
    m_dhcp6Config = QDBusObjectPath(config);
}

void Device::setDriver(const QString &driver)
{
    m_driver = driver;
}

void Device::setDriverVersion(const QString &driverVersion)
{
    m_driverVersion = driverVersion;
}

void Device::setFirmwareMissing(bool firmwareMissing)
{
    m_firmwareMissing = firmwareMissing;
}

void Device::setFirmwareVersion(const QString &firmwareVersion)
{
    m_firmwareVersion = firmwareVersion;
}

void Device::setInterface(const QString &interface)
{
    m_interface = interface;
}

void Device::setInterfaceFlags(uint interfaceFlags)
{
    m_interfaceFlags = interfaceFlags;
}

void Device::setIp4Config(const QString &config)
{
    m_ip4Config = QDBusObjectPath(config);
}

void Device::setIp6Config(const QString &config)
{
    m_ip6Config = QDBusObjectPath(config);
}

void Device::setIpv4Address(int address)
{
    m_ip4Address = address;
}

void Device::setIpInterface(const QString &interface)
{
    m_ipInterface = interface;
}

void Device::setManaged(bool managed)
{
    m_managed = managed;
}

void Device::setMtu(uint mtu)
{
    m_mtu = mtu;
}

void Device::setState(uint state)
{
    uint previousState = m_state;
    m_state = state;
    m_stateReason.state = state;

    QDBusMessage message = QDBusMessage::createSignal(m_devicePath, QLatin1String("org.kde.fakenetwork.Device"), QLatin1String("StateChanged"));
    message << previousState << state << m_stateReason.reason;
    QDBusConnection::sessionBus().send(message);
}

void Device::setStateReason(const DeviceDBusStateReason &reason)
{
    m_stateReason = reason;
}

void Device::setUdi(const QString &udi)
{
    m_udi = udi;
}

void Device::Disconnect()
{
    // TODO
}

#include "moc_device.cpp"
