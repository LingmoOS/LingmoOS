/*
    SPDX-FileCopyrightText: 2014 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "fakenetwork.h"
#include "connection.h"
#include "wireddevice.h"
#include "wirelessdevice.h"

#include <QDBusConnection>
#include <QDBusMetaType>
#include <QTimer>

#include "../manager.h"
#include "connectionsettings.h"

FakeNetwork::FakeNetwork(QObject *parent)
    : QObject(parent)
    , m_activatingConnection(QDBusObjectPath("/"))
    , m_connectivity(NetworkManager::Connectivity::NoConnectivity)
    , m_networkingEnabled(true)
    , m_primaryConnection(QDBusObjectPath("/"))
    , m_state(20)
    , m_version(QLatin1String("0.9.10.0"))
    , m_wimaxEnabled(true)
    , m_wimaxHardwareEnabled(true)
    , m_wirelessEnabled(true)
    , m_wirelessHardwareEnabled(true)
    , m_wwanEnabled(true)
    , m_wwanHardwareEnabled(true)
    , m_activeConnectionsCounter(0)
    , m_deviceCounter(0)
    , m_settings(new Settings(this))
{
    qDBusRegisterMetaType<QDBusObjectPath>();
    registerService();
    connect(m_settings, &Settings::connectionAdded, this, &FakeNetwork::onConnectionAdded);
    connect(m_settings, &Settings::connectionRemoved, this, &FakeNetwork::onConnectionRemoved);
}

FakeNetwork::~FakeNetwork()
{
    unregisterService();
    qDeleteAll(m_devices);
    delete m_settings;
}

QDBusObjectPath FakeNetwork::activatingConnection() const
{
    return m_activatingConnection;
}

QList<QDBusObjectPath> FakeNetwork::activeConnections() const
{
    return m_activeConnections.keys();
}

uint FakeNetwork::connectivity() const
{
    return m_connectivity;
}

QList<QDBusObjectPath> FakeNetwork::devices() const
{
    return m_devices.keys();
}

bool FakeNetwork::networkingEnabled() const
{
    return m_networkingEnabled;
}

QDBusObjectPath FakeNetwork::primaryConnection() const
{
    return m_primaryConnection;
}

uint FakeNetwork::state() const
{
    return m_state;
}

QString FakeNetwork::version() const
{
    return m_version;
}

bool FakeNetwork::wimaxEnabled() const
{
    return m_wimaxEnabled;
}

void FakeNetwork::setWimaxEnabled(bool enabled)
{
    m_wimaxEnabled = enabled;

    QVariantMap map;
    map.insert(QLatin1String("WimaxEnabled"), m_wimaxEnabled);
    Q_EMIT PropertiesChanged(map);
}

bool FakeNetwork::wimaxHardwareEnabled() const
{
    return m_wimaxHardwareEnabled;
}

void FakeNetwork::setWimaxHardwareEnabled(bool enabled)
{
    m_wimaxHardwareEnabled = enabled;

    QVariantMap map;
    map.insert(QLatin1String("WimaxHardwareEnabled"), m_wimaxHardwareEnabled);
    Q_EMIT PropertiesChanged(map);
}

bool FakeNetwork::wirelessEnabled() const
{
    return m_wirelessEnabled;
}

void FakeNetwork::setWirelessEnabled(bool enabled)
{
    m_wirelessEnabled = enabled;

    QVariantMap map;
    map.insert(QLatin1String("WirelessEnabled"), m_wirelessEnabled);
    Q_EMIT PropertiesChanged(map);
}

bool FakeNetwork::wirelessHardwareEnabled() const
{
    return m_wirelessHardwareEnabled;
}

void FakeNetwork::setWirelessHardwareEnabled(bool enabled)
{
    m_wirelessHardwareEnabled = enabled;

    QVariantMap map;
    map.insert(QLatin1String("WirelessHardwareEnabled"), m_wirelessHardwareEnabled);
    Q_EMIT PropertiesChanged(map);
}

bool FakeNetwork::wwanEnabled() const
{
    return m_wwanEnabled;
}

void FakeNetwork::setWwanEnabled(bool enabled)
{
    m_wwanEnabled = enabled;

    QVariantMap map;
    map.insert(QLatin1String("WwanEnabled"), m_wwanEnabled);
    Q_EMIT PropertiesChanged(map);
}

bool FakeNetwork::wwanHardwareEnabled() const
{
    return m_wwanHardwareEnabled;
}

void FakeNetwork::addDevice(Device *device)
{
    QString newDevicePath = QString("/org/kde/fakenetwork/Devices/") + QString::number(m_deviceCounter++);
    device->setDevicePath(newDevicePath);
    m_devices.insert(QDBusObjectPath(newDevicePath), device);
    QDBusConnection::sessionBus().registerObject(newDevicePath, device, QDBusConnection::ExportScriptableContents);
    Q_EMIT DeviceAdded(QDBusObjectPath(newDevicePath));
}

void FakeNetwork::removeDevice(Device *device)
{
    m_devices.remove(QDBusObjectPath(device->devicePath()));
    QDBusConnection::sessionBus().unregisterObject(device->devicePath());
    Q_EMIT DeviceRemoved(QDBusObjectPath(device->devicePath()));
}

void FakeNetwork::registerService()
{
    QDBusConnection::sessionBus().registerService(QLatin1String("org.kde.fakenetwork"));
    QDBusConnection::sessionBus().registerObject(QLatin1String("/org/kde/fakenetwork"), this, QDBusConnection::ExportScriptableContents);
    QDBusConnection::sessionBus().registerObject(QLatin1String("/org/kde/fakenetwork/Settings"), m_settings, QDBusConnection::ExportScriptableContents);

    for (auto it = m_devices.cbegin(); it != m_devices.cend(); ++it) {
        const QDBusObjectPath &devicePath = it.key();
        QDBusConnection::sessionBus().registerObject(devicePath.path(), it.value(), QDBusConnection::ExportScriptableContents);
        Q_EMIT DeviceAdded(devicePath);
    }
}

void FakeNetwork::unregisterService()
{
    for (auto it = m_devices.cbegin(); it != m_devices.cend(); ++it) {
        const QDBusObjectPath &devicePath = it.key();
        QDBusConnection::sessionBus().unregisterObject(devicePath.path());
        Q_EMIT DeviceRemoved(devicePath);
    }

    QDBusConnection::sessionBus().unregisterObject(QLatin1String("/org/kde/fakenetwork/Settings"));
    QDBusConnection::sessionBus().unregisterObject(QLatin1String("/org/kde/fakenetwork"));
    QDBusConnection::sessionBus().unregisterService(QLatin1String("org.kde.fakenetwork"));
}

QDBusObjectPath FakeNetwork::ActivateConnection(const QDBusObjectPath &connection, const QDBusObjectPath &device, const QDBusObjectPath &specific_object)
{
    QString newActiveConnectionPath = QString("/org/kde/fakenetwork/ActiveConnection/") + QString::number(m_activeConnectionsCounter++);
    ActiveConnection *newActiveConnection = new ActiveConnection(this);
    newActiveConnection->addDevice(device);
    newActiveConnection->setActiveConnectionPath(newActiveConnectionPath);
    newActiveConnection->setConnection(connection);
    newActiveConnection->setSpecificObject(specific_object);
    newActiveConnection->setState(NetworkManager::ActiveConnection::Activating);

    m_activeConnections.insert(QDBusObjectPath(newActiveConnectionPath), newActiveConnection);
    QDBusConnection::sessionBus().registerObject(newActiveConnectionPath, newActiveConnection, QDBusConnection::ExportScriptableContents);

    m_activatingConnection = QDBusObjectPath(newActiveConnectionPath);

    QVariantMap map;
    map.insert(QLatin1String("ActiveConnections"), QVariant::fromValue<QList<QDBusObjectPath>>(m_activeConnections.keys()));
    map.insert(QLatin1String("ActivatingConnection"), QVariant::fromValue(QDBusObjectPath(newActiveConnectionPath)));
    Q_EMIT PropertiesChanged(map);

    Device *usedDevice = static_cast<Device *>(QDBusConnection::sessionBus().objectRegisteredAt(device.path()));
    if (usedDevice) {
        m_activatedDevice = usedDevice->devicePath();
        // Start simulation of activation
        usedDevice->setActiveConnection(newActiveConnectionPath);
        usedDevice->setState(NetworkManager::Device::Preparing);
        QTimer::singleShot(100, this, SLOT(updateConnectingState()));
    }

    return QDBusObjectPath(newActiveConnectionPath);
}

void FakeNetwork::updateConnectingState()
{
    QVariantMap deviceMap;
    Device *device = m_devices.value(QDBusObjectPath(m_activatedDevice));
    if (device->state() == NetworkManager::Device::Preparing) {
        device->setState(NetworkManager::Device::ConfiguringHardware);
    } else if (device->state() == NetworkManager::Device::ConfiguringHardware) {
        device->setState(NetworkManager::Device::NeedAuth);
    } else if (device->state() == NetworkManager::Device::NeedAuth) {
        device->setState(NetworkManager::Device::ConfiguringIp);
    } else if (device->state() == NetworkManager::Device::ConfiguringIp) {
        device->setState(NetworkManager::Device::CheckingIp);
    } else if (device->state() == NetworkManager::Device::CheckingIp) {
        device->setState(NetworkManager::Device::Activated);

        ActiveConnection *activeConnection =
            static_cast<ActiveConnection *>(QDBusConnection::sessionBus().objectRegisteredAt(device->activeConnection().path()));
        if (activeConnection) {
            QVariantMap activeConnectionMap;
            activeConnectionMap.insert(QLatin1String("State"), NetworkManager::ActiveConnection::Activated);

            activeConnection->setState(NetworkManager::ActiveConnection::Activated);
            QDBusMessage message = QDBusMessage::createSignal(activeConnection->activeConnectionPath(),
                                                              QLatin1String("org.kde.fakenetwork.Connection.Active"),
                                                              QLatin1String("PropertiesChanged"));
            message << activeConnectionMap;
            QDBusConnection::sessionBus().send(message);

            QDBusMessage message2 = QDBusMessage::createSignal(activeConnection->activeConnectionPath(),
                                                               QLatin1String("org.kde.fakenetwork.Connection.Active"),
                                                               QLatin1String("StateChanged"));
            message2 << (uint)2 << (uint)1; // NM_ACTIVE_CONNECTION_STATE_ACTIVATED << NM_ACTIVE_CONNECTION_STATE_REASON_NONE
            QDBusConnection::sessionBus().send(message2);
        }
        // TODO: set dhcp4Config, dhcp6Config, ip4Config, ip6Config
        // IP Interface is usually same as interface
        device->setIpInterface(device->interface());
        // Set some IP address
        device->setIpv4Address(1763189258);

        deviceMap.insert(QLatin1String("IpInterface"), device->ipInterface());
        deviceMap.insert(QLatin1String("Ip4Address"), device->ip4Address());
        deviceMap.insert(QLatin1String("ActiveConnection"), m_activatingConnection.path());

        // Update FakeNetwork state, connectivity, primary connection
        m_connectivity = NetworkManager::Connectivity::Full;
        m_primaryConnection = m_activatingConnection;
        m_activatingConnection = QDBusObjectPath("/");
        m_state = 70;
        QVariantMap networkMap;
        networkMap.insert(QLatin1String("ActivatingConnection"), QVariant::fromValue(m_activatingConnection));
        networkMap.insert(QLatin1String("Connectivity"), NetworkManager::Connectivity::Full);
        networkMap.insert(QLatin1String("PrimaryConnection"), QVariant::fromValue(m_primaryConnection));
        networkMap.insert(QLatin1String("State"), m_state);

        QDBusMessage message =
            QDBusMessage::createSignal(QLatin1String("/org/kde/fakenetwork"), QLatin1String("org.kde.fakenetwork"), QLatin1String("PropertiesChanged"));
        message << networkMap;
        QDBusConnection::sessionBus().send(message);

        Q_EMIT StateChanged(m_state);
    }

    deviceMap.insert(QLatin1String("State"), device->state());
    QDBusMessage message = QDBusMessage::createSignal(device->devicePath(), device->deviceInterface(), QLatin1String("PropertiesChanged"));
    message << deviceMap;
    QDBusConnection::sessionBus().send(message);

    if (device->state() != NetworkManager::Device::Activated) {
        QTimer::singleShot(100, this, SLOT(updateConnectingState()));
    }
}

uint FakeNetwork::CheckConnectivity() const
{
    return m_connectivity;
}

void FakeNetwork::DeactivateConnection(const QDBusObjectPath &active_connection)
{
    ActiveConnection *activeConnection = m_activeConnections.value(active_connection);
    if (activeConnection) {
        activeConnection->setState(NetworkManager::ActiveConnection::Deactivating);

        QVariantMap activeConnectionMap;
        activeConnectionMap.insert(QLatin1String("State"), NetworkManager::ActiveConnection::Deactivating);

        activeConnection->setState(NetworkManager::ActiveConnection::Activated);
        QDBusMessage message = QDBusMessage::createSignal(activeConnection->activeConnectionPath(),
                                                          QLatin1String("org.kde.fakenetwork.Connection.Active"),
                                                          QLatin1String("PropertiesChanged"));
        message << activeConnectionMap;
        QDBusConnection::sessionBus().send(message);

        QDBusMessage message2 = QDBusMessage::createSignal(activeConnection->activeConnectionPath(),
                                                           QLatin1String("org.kde.fakenetwork.Connection.Active"),
                                                           QLatin1String("StateChanged"));
        message2 << (uint)4 << (uint)2; // NM_ACTIVE_CONNECTION_STATE_DEACTIVATED << NM_ACTIVE_CONNECTION_STATE_REASON_USER_DISCONNECTED
        QDBusConnection::sessionBus().send(message2);

        Device *device = m_devices.value(activeConnection->devices().first());
        if (device) {
            m_deactivatedDevice = device->devicePath();
            device->setState(NetworkManager::Device::Deactivating);
            QTimer::singleShot(100, this, SLOT(updateDeactivatingState()));
        }

        // Update FakeNetwork state, connectivity, primary connection
        m_connectivity = NetworkManager::Connectivity::NoConnectivity;
        m_primaryConnection = QDBusObjectPath("/");
        m_state = 20;
        QVariantMap networkMap;
        networkMap.insert(QLatin1String("Connectivity"), m_connectivity);
        networkMap.insert(QLatin1String("PrimaryConnection"), QVariant::fromValue(m_primaryConnection));
        networkMap.insert(QLatin1String("State"), m_state);

        message = QDBusMessage::createSignal(QLatin1String("/org/kde/fakenetwork"), QLatin1String("org.kde.fakenetwork"), QLatin1String("PropertiesChanged"));
        message << networkMap;
        QDBusConnection::sessionBus().send(message);

        Q_EMIT StateChanged(m_state);
    }
}

void FakeNetwork::updateDeactivatingState()
{
    QVariantMap deviceMap;
    Device *device = m_devices.value(QDBusObjectPath(m_deactivatedDevice));
    ActiveConnection *activeConnection = static_cast<ActiveConnection *>(QDBusConnection::sessionBus().objectRegisteredAt(device->activeConnection().path()));
    if (activeConnection) {
        QVariantMap activeConnectionMap;
        activeConnectionMap.insert(QLatin1String("State"), NetworkManager::ActiveConnection::Deactivated);

        activeConnection->setState(NetworkManager::ActiveConnection::Activated);
        QDBusMessage message = QDBusMessage::createSignal(activeConnection->activeConnectionPath(),
                                                          QLatin1String("org.kde.fakenetwork.Connection.Active"),
                                                          QLatin1String("PropertiesChanged"));
        message << activeConnectionMap;
        QDBusConnection::sessionBus().send(message);

        QDBusMessage message2 = QDBusMessage::createSignal(activeConnection->activeConnectionPath(),
                                                           QLatin1String("org.kde.fakenetwork.Connection.Active"),
                                                           QLatin1String("StateChanged"));
        message2 << (uint)3 << (uint)2; // NM_ACTIVE_CONNECTION_STATE_DEACTIVATING << NM_ACTIVE_CONNECTION_STATE_REASON_USER_DISCONNECTED
        QDBusConnection::sessionBus().send(message2);

        removeActiveConnection(QDBusObjectPath(activeConnection->activeConnectionPath()));
    }

    device->setActiveConnection(QLatin1String("/"));
    device->setState(NetworkManager::Device::Disconnected);
    // TODO: set dhcp4Config, dhcp6Config, ip4Config, ip6Config
    // IP Interface is usually same as interface
    device->setIpInterface("");
    // Set some IP address
    device->setIpv4Address(0);

    deviceMap.insert(QLatin1String("ActiveConnection"), device->activeConnection().path());
    deviceMap.insert(QLatin1String("IpInterface"), device->ipInterface());
    deviceMap.insert(QLatin1String("Ip4Address"), device->ip4Address());
    deviceMap.insert(QLatin1String("State"), device->state());

    QDBusMessage message = QDBusMessage::createSignal(device->devicePath(), device->deviceInterface(), QLatin1String("PropertiesChanged"));
    message << deviceMap;
    QDBusConnection::sessionBus().send(message);
}

QDBusObjectPath FakeNetwork::GetDeviceByIpIface(const QString &iface)
{
    Q_UNUSED(iface)
    // TODO
    return QDBusObjectPath();
}

QList<QDBusObjectPath> FakeNetwork::GetDevices() const
{
    return m_devices.keys();
}

void FakeNetwork::onConnectionAdded(const QDBusObjectPath &connection)
{
    Connection *newConnection = static_cast<Connection *>(QDBusConnection::sessionBus().objectRegisteredAt(connection.path()));
    if (newConnection) {
        NMVariantMapMap settings = newConnection->GetSettings();
        NetworkManager::ConnectionSettings::ConnectionType type =
            NetworkManager::ConnectionSettings::typeFromString(settings.value(QLatin1String("connection")).value(QLatin1String("type")).toString());
        if (!m_devices.isEmpty()) {
            Device *selectedDevice = nullptr;
            for (Device *device : std::as_const(m_devices)) {
                if (type == NetworkManager::ConnectionSettings::Wired && device->deviceType() == NetworkManager::Device::Ethernet) {
                    selectedDevice = device;
                    device->addAvailableConnection(connection);
                    break;
                } else if (type == NetworkManager::ConnectionSettings::Wireless && device->deviceType() == NetworkManager::Device::Wifi) {
                    selectedDevice = device;
                    // TODO
                    break;
                }
            }

            if (selectedDevice) {
                QVariantMap map;
                map.insert(QLatin1String("AvailableConnections"), QVariant::fromValue<QList<QDBusObjectPath>>(selectedDevice->availableConnections()));

                QDBusMessage message =
                    QDBusMessage::createSignal(selectedDevice->devicePath(), selectedDevice->deviceInterface(), QLatin1String("PropertiesChanged"));
                message << map;
                QDBusConnection::sessionBus().send(message);
            }
        }
    }
}

void FakeNetwork::onConnectionRemoved(const QDBusObjectPath &connection)
{
    for (Device *device : std::as_const(m_devices)) {
        if (device && device->availableConnections().contains(connection)) {
            device->removeAvailableConnection(connection);

            QVariantMap map;
            map.insert(QLatin1String("AvailableConnections"), QVariant::fromValue<QList<QDBusObjectPath>>(device->availableConnections()));

            QDBusMessage message = QDBusMessage::createSignal(device->devicePath(), device->deviceInterface(), QLatin1String("PropertiesChanged"));
            message << map;
            QDBusConnection::sessionBus().send(message);
        }
    }
}

void FakeNetwork::removeActiveConnection(const QDBusObjectPath &activeConnection)
{
    delete m_activeConnections.value(activeConnection);
    m_activeConnections.remove(activeConnection);
    QDBusConnection::sessionBus().unregisterObject(activeConnection.path());

    QVariantMap map;
    map.insert(QLatin1String("ActiveConnections"), QVariant::fromValue<QList<QDBusObjectPath>>(m_activeConnections.keys()));
    Q_EMIT PropertiesChanged(map);
}

#include "moc_fakenetwork.cpp"
