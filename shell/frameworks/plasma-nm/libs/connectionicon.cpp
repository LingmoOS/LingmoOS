/*
    SPDX-FileCopyrightText: 2013 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "connectionicon.h"
#include "configuration.h"
#include "uiutils.h"

#include <NetworkManagerQt/BluetoothDevice>
#include <NetworkManagerQt/Connection>
#include <NetworkManagerQt/ConnectionSettings>
#include <NetworkManagerQt/Device>
#include <NetworkManagerQt/ModemDevice>
#include <NetworkManagerQt/WiredDevice>
#include <NetworkManagerQt/WirelessDevice>
#include <NetworkManagerQt/WirelessSetting>

#include <ModemManagerQt/Manager>

#include <QDBusReply>

#include <QCoroDBus>

ConnectionIcon::ConnectionIcon(QObject *parent)
    : QObject(parent)
    , m_wirelessNetwork(nullptr)
    , m_modemNetwork(nullptr)
{
    connect(NetworkManager::notifier(), &NetworkManager::Notifier::primaryConnectionChanged, this, &ConnectionIcon::primaryConnectionChanged);
    connect(NetworkManager::notifier(), &NetworkManager::Notifier::activatingConnectionChanged, this, &ConnectionIcon::activatingConnectionChanged);
    connect(NetworkManager::notifier(), &NetworkManager::Notifier::activeConnectionAdded, this, &ConnectionIcon::activeConnectionAdded);
    connect(NetworkManager::notifier(), &NetworkManager::Notifier::deviceAdded, this, &ConnectionIcon::deviceAdded);
    connect(NetworkManager::notifier(), &NetworkManager::Notifier::deviceRemoved, this, &ConnectionIcon::deviceRemoved);
    connect(NetworkManager::notifier(), &NetworkManager::Notifier::networkingEnabledChanged, this, &ConnectionIcon::networkingEnabledChanged);
    connect(NetworkManager::notifier(), &NetworkManager::Notifier::statusChanged, this, &ConnectionIcon::statusChanged);
    connect(NetworkManager::notifier(), &NetworkManager::Notifier::wirelessEnabledChanged, this, &ConnectionIcon::wirelessEnabledChanged);
    connect(NetworkManager::notifier(), &NetworkManager::Notifier::wirelessHardwareEnabledChanged, this, &ConnectionIcon::wirelessEnabledChanged);
    connect(NetworkManager::notifier(), &NetworkManager::Notifier::wwanEnabledChanged, this, &ConnectionIcon::wwanEnabledChanged);
    connect(NetworkManager::notifier(), &NetworkManager::Notifier::wwanHardwareEnabledChanged, this, &ConnectionIcon::wwanEnabledChanged);

    for (const NetworkManager::Device::Ptr &device : NetworkManager::networkInterfaces()) {
        if (device->type() == NetworkManager::Device::Ethernet) {
            NetworkManager::WiredDevice::Ptr wiredDevice = device.staticCast<NetworkManager::WiredDevice>();
            if (wiredDevice) {
                connect(wiredDevice.data(), &NetworkManager::WiredDevice::carrierChanged, this, &ConnectionIcon::carrierChanged);
            }
        } else if (device->type() == NetworkManager::Device::Wifi) {
            NetworkManager::WirelessDevice::Ptr wifiDevice = device.staticCast<NetworkManager::WirelessDevice>();
            if (wifiDevice) {
                connect(wifiDevice.data(), &NetworkManager::WirelessDevice::availableConnectionAppeared, this, &ConnectionIcon::wirelessNetworkAppeared);
                connect(wifiDevice.data(), &NetworkManager::WirelessDevice::networkAppeared, this, &ConnectionIcon::wirelessNetworkAppeared);
            }
        }
    }

    for (const NetworkManager::ActiveConnection::Ptr &activeConnection : NetworkManager::activeConnections()) {
        addActiveConnection(activeConnection->path());
    }
    setStates();

    setIcons();
}

ConnectionIcon::~ConnectionIcon() = default;

bool ConnectionIcon::connecting() const
{
    return m_connecting;
}

QString ConnectionIcon::connectionIcon() const
{
    if (m_vpn && !m_connectionIcon.contains(QLatin1String("available"))) {
        return m_connectionIcon + QStringLiteral("-locked");
    }

    if (m_limited && !m_connectionIcon.contains(QLatin1String("available"))) {
        return m_connectionIcon + QStringLiteral("-limited");
    }

    return m_connectionIcon;
}

QString ConnectionIcon::connectionTooltipIcon() const
{
    return m_connectionTooltipIcon;
}

NetworkManager::Connectivity ConnectionIcon::connectivity() const
{
    return m_connectivity;
}

void ConnectionIcon::setConnectivity(NetworkManager::Connectivity connectivity)
{
    if (m_connectivity == connectivity) {
        return;
    }

    m_connectivity = connectivity;
    Q_EMIT connectivityChanged(connectivity);

    const bool limited = (connectivity == NetworkManager::Portal || connectivity == NetworkManager::Limited);
    if (m_limited != limited) {
        m_limited = limited;
        Q_EMIT connectionIconChanged(connectionIcon());
    }
}

void ConnectionIcon::activatingConnectionChanged(const QString &connection)
{
    Q_UNUSED(connection);
    setIcons();
}

void ConnectionIcon::addActiveConnection(const QString &activeConnection)
{
    NetworkManager::ActiveConnection::Ptr active = NetworkManager::findActiveConnection(activeConnection);

    if (active) {
        NetworkManager::VpnConnection::Ptr vpnConnection;
        connect(active.data(), &NetworkManager::ActiveConnection::destroyed, this, &ConnectionIcon::activeConnectionDestroyed);
        if (active->vpn()) {
            vpnConnection = active.objectCast<NetworkManager::VpnConnection>();
            connect(vpnConnection.data(), &NetworkManager::VpnConnection::stateChanged, this, &ConnectionIcon::vpnConnectionStateChanged);
        } else {
            connect(active.data(), &NetworkManager::ActiveConnection::stateChanged, this, &ConnectionIcon::activeConnectionStateChanged, Qt::UniqueConnection);
        }
    }
}

void ConnectionIcon::activeConnectionAdded(const QString &activeConnection)
{
    addActiveConnection(activeConnection);
    setStates();
}

void ConnectionIcon::activeConnectionStateChanged(NetworkManager::ActiveConnection::State state)
{
    Q_UNUSED(state);
    setStates();
}

void ConnectionIcon::activeConnectionDestroyed()
{
    setStates();
}

void ConnectionIcon::carrierChanged(bool carrier)
{
    Q_UNUSED(carrier);
    setIcons();
}

void ConnectionIcon::deviceAdded(const QString &device)
{
    NetworkManager::Device::Ptr dev = NetworkManager::findNetworkInterface(device);

    if (!dev) {
        return;
    }

    if (dev->type() == NetworkManager::Device::Ethernet) {
        NetworkManager::WiredDevice::Ptr wiredDev = dev.objectCast<NetworkManager::WiredDevice>();
        connect(wiredDev.data(), &NetworkManager::WiredDevice::carrierChanged, this, &ConnectionIcon::carrierChanged);
    }
}

void ConnectionIcon::deviceRemoved(const QString &device)
{
    Q_UNUSED(device);

    if (NetworkManager::status() == NetworkManager::Disconnected) {
        setDisconnectedIcon();
    }
}

void ConnectionIcon::modemNetworkRemoved()
{
    m_modemNetwork.clear();
}

void ConnectionIcon::modemSignalChanged(ModemManager::SignalQualityPair signalQuality)
{
    int diff = m_signal - signalQuality.signal;

    if (diff >= 10 || diff <= -10) {
        m_signal = signalQuality.signal;

        setIconForModem();
    }
}

void ConnectionIcon::networkingEnabledChanged(bool enabled)
{
    if (!enabled) {
        setConnectionIcon(QStringLiteral("network-unavailable"));
    }
}

void ConnectionIcon::primaryConnectionChanged(const QString &connection)
{
    if (!connection.isEmpty()) {
        setIcons();
    }
}

void ConnectionIcon::statusChanged(NetworkManager::Status status)
{
    if (status == NetworkManager::Disconnected) {
        setDisconnectedIcon();
    }
}

void ConnectionIcon::vpnConnectionStateChanged(NetworkManager::VpnConnection::State state, NetworkManager::VpnConnection::StateChangeReason reason)
{
    Q_UNUSED(state);
    Q_UNUSED(reason);
    setStates();
    setIcons();
}

void ConnectionIcon::wirelessEnabledChanged(bool enabled)
{
    Q_UNUSED(enabled);
    setIcons();
}

void ConnectionIcon::wwanEnabledChanged(bool enabled)
{
    Q_UNUSED(enabled);
    setIcons();
}

void ConnectionIcon::wirelessNetworkAppeared(const QString &network)
{
    Q_UNUSED(network);
    setIcons();
}

void ConnectionIcon::setStates()
{
    bool connecting = false;
    bool vpn = false;
    for (const NetworkManager::ActiveConnection::Ptr &activeConnection : NetworkManager::activeConnections()) {
        NetworkManager::VpnConnection::Ptr vpnConnection;
        if (activeConnection->vpn()) {
            vpnConnection = activeConnection.objectCast<NetworkManager::VpnConnection>();
        }

        if (!vpnConnection) {
            if (activeConnection->state() == NetworkManager::ActiveConnection::Activating && UiUtils::isConnectionTypeSupported(activeConnection->type())) {
                connecting = true;
            }
            if (activeConnection->type() == NetworkManager::ConnectionSettings::ConnectionType::WireGuard) {
                vpn = true;
            }
        } else {
            if (vpnConnection->state() == NetworkManager::VpnConnection::Activated) {
                vpn = true;
            } else if (vpnConnection->state() == NetworkManager::VpnConnection::Prepare //
                       || vpnConnection->state() == NetworkManager::VpnConnection::NeedAuth //
                       || vpnConnection->state() == NetworkManager::VpnConnection::Connecting //
                       || vpnConnection->state() == NetworkManager::VpnConnection::GettingIpConfig) {
                connecting = true;
            }
        }
    }

    setVpn(vpn);
    setConnecting(connecting);
}

void ConnectionIcon::setIcons()
{
    m_signal = 0;
    if (m_modemNetwork) {
        disconnect(m_modemNetwork.data(), nullptr, this, nullptr);
        m_modemNetwork.clear();
    }
    if (m_wirelessNetwork) {
        disconnect(m_wirelessNetwork.data(), nullptr, this, nullptr);
        m_wirelessNetwork.clear();
    }

    NetworkManager::ActiveConnection::Ptr connection = NetworkManager::activatingConnection();

    // Set icon based on the current primary connection if the activating connection is virtual
    // since we're not setting icons for virtual connections
    if (!connection || (connection && UiUtils::isConnectionTypeVirtual(connection->type()))
        || connection->type() == NetworkManager::ConnectionSettings::WireGuard) {
        connection = NetworkManager::primaryConnection();
    }

    /* Fallback: If we still don't have an active connection with default route or the default route goes through a connection
                 of generic type (some type of VPNs) we need to go through all other active connections and pick the one with
                 highest probability of being the main one (order is: vpn, wired, wireless, gsm, cdma, bluetooth) */
    if ((!connection && !NetworkManager::activeConnections().isEmpty()) || (connection && connection->type() == NetworkManager::ConnectionSettings::Generic)
        || (connection && connection->type() == NetworkManager::ConnectionSettings::Tun)) {
        for (const NetworkManager::ActiveConnection::Ptr &activeConnection : NetworkManager::activeConnections()) {
            const NetworkManager::ConnectionSettings::ConnectionType type = activeConnection->type();
            if (type == NetworkManager::ConnectionSettings::Bluetooth) {
                if (connection && connection->type() <= NetworkManager::ConnectionSettings::Bluetooth) {
                    connection = activeConnection;
                }
            } else if (type == NetworkManager::ConnectionSettings::Cdma) {
                if (connection && connection->type() <= NetworkManager::ConnectionSettings::Cdma) {
                    connection = activeConnection;
                }
            } else if (type == NetworkManager::ConnectionSettings::Gsm) {
                if (connection && connection->type() <= NetworkManager::ConnectionSettings::Gsm) {
                    connection = activeConnection;
                }
            } else if (type == NetworkManager::ConnectionSettings::Vpn) {
                connection = activeConnection;
            } else if (type == NetworkManager::ConnectionSettings::WireGuard) {
                connection = activeConnection;
            } else if (type == NetworkManager::ConnectionSettings::Wired) {
                if (connection
                    && (connection->type() != NetworkManager::ConnectionSettings::Vpn //
                        || connection->type() != NetworkManager::ConnectionSettings::WireGuard)) {
                    connection = activeConnection;
                }
            } else if (type == NetworkManager::ConnectionSettings::Wireless) {
                if (connection
                    && (connection->type() != NetworkManager::ConnectionSettings::Vpn //
                        && (connection->type() != NetworkManager::ConnectionSettings::Wired))) {
                    connection = activeConnection;
                }
            }
        }
    }

    if (connection && !connection->devices().isEmpty()) {
        NetworkManager::Device::Ptr device = NetworkManager::findNetworkInterface(connection->devices().first());

        if (device) {
            NetworkManager::Device::Type type = device->type();
            if (type == NetworkManager::Device::Wifi) {
                NetworkManager::WirelessDevice::Ptr wifiDevice = device.objectCast<NetworkManager::WirelessDevice>();
                if (wifiDevice->mode() == NetworkManager::WirelessDevice::Adhoc) {
                    setWirelessIconForSignalStrength(100);
                } else {
                    NetworkManager::AccessPoint::Ptr ap = wifiDevice->activeAccessPoint();
                    if (ap) {
                        setWirelessIcon(device, ap->ssid());
                    }
                }
            } else if (type == NetworkManager::Device::Ethernet) {
                setConnectionIcon(QStringLiteral("network-wired-activated"));
                setConnectionTooltipIcon(QStringLiteral("network-wired-activated"));
            } else if (type == NetworkManager::Device::Modem) {
                setModemIcon(device);
            } else if (type == NetworkManager::Device::Bluetooth) {
                NetworkManager::BluetoothDevice::Ptr btDevice = device.objectCast<NetworkManager::BluetoothDevice>();
                if (btDevice) {
                    if (btDevice->bluetoothCapabilities().testFlag(NetworkManager::BluetoothDevice::Dun)) {
                        setModemIcon(device);
                    } else {
                        setConnectionIcon(QStringLiteral("network-wireless-bluetooth-symbolic"));
                        setConnectionTooltipIcon(QStringLiteral("network-wireless-bluetooth"));
                    }
                }
            } else if (type == 29) { // TODO change to WireGuard enum value once it is added
                // WireGuard is a VPN but is not implemented
                // in NetworkManager as a VPN, so we don't want to
                // do anything just because it has a device
                // associated with it.
            } else {
                // Ignore other devices (bond/bridge/team etc.)
                setDisconnectedIcon();
            }
        }
    } else {
        setDisconnectedIcon();
    }
}

void ConnectionIcon::setDisconnectedIcon()
{
    if (Configuration::self().airplaneModeEnabled()) {
        setConnectionIcon(QStringLiteral("network-flightmode-on"));
        return;
    }

    if (NetworkManager::status() == NetworkManager::Unknown //
        || NetworkManager::status() == NetworkManager::Asleep) {
        setConnectionIcon(QStringLiteral("network-unavailable"));
        return;
    }

    bool wired = false;
    bool wireless = false;
    bool modem = false;

    m_limited = false;
    m_vpn = false;

    for (const NetworkManager::Device::Ptr &device : NetworkManager::networkInterfaces()) {
        if (device->type() == NetworkManager::Device::Ethernet) {
            NetworkManager::WiredDevice::Ptr wiredDev = device.objectCast<NetworkManager::WiredDevice>();
            if (wiredDev->carrier()) {
                wired = true;
            }
        } else if (device->type() == NetworkManager::Device::Wifi //
                   && NetworkManager::isWirelessEnabled() //
                   && NetworkManager::isWirelessHardwareEnabled()) {
            NetworkManager::WirelessDevice::Ptr wifiDevice = device.objectCast<NetworkManager::WirelessDevice>();
            if (!wifiDevice->accessPoints().isEmpty() || !wifiDevice->availableConnections().isEmpty()) {
                wireless = true;
            }
        } else if (device->type() == NetworkManager::Device::Modem //
                   && NetworkManager::isWwanEnabled() //
                   && NetworkManager::isWwanHardwareEnabled()) {
            modem = true;
        }
    }

    if (wired) {
        setConnectionIcon(QStringLiteral("network-wired-available"));
        setConnectionTooltipIcon(QStringLiteral("network-wired"));
        return;
    } else if (wireless) {
        setConnectionIcon(QStringLiteral("network-wireless-available"));
        setConnectionTooltipIcon(QStringLiteral("network-wireless-connected-00"));
        return;
    } else if (modem) {
        setConnectionIcon(QStringLiteral("network-mobile-available"));
        setConnectionTooltipIcon(QStringLiteral("phone"));
        return;
    } else {
        setConnectionIcon(QStringLiteral("network-unavailable"));
        setConnectionTooltipIcon(QStringLiteral("network-wired"));
    }
}

void ConnectionIcon::setModemIcon(const NetworkManager::Device::Ptr &device)
{
    NetworkManager::ModemDevice::Ptr modemDevice = device.objectCast<NetworkManager::ModemDevice>();

    if (!modemDevice) {
        setConnectionIcon(QStringLiteral("network-mobile-100"));

        return;
    }

    ModemManager::ModemDevice::Ptr modem = ModemManager::findModemDevice(device->udi());
    if (modem) {
        if (modem->hasInterface(ModemManager::ModemDevice::ModemInterface)) {
            m_modemNetwork = modem->interface(ModemManager::ModemDevice::ModemInterface).objectCast<ModemManager::Modem>();
        }
    }

    if (m_modemNetwork) {
        connect(m_modemNetwork.data(), &ModemManager::Modem::signalQualityChanged, this, &ConnectionIcon::modemSignalChanged, Qt::UniqueConnection);
        connect(m_modemNetwork.data(), &ModemManager::Modem::accessTechnologiesChanged, this, &ConnectionIcon::setIconForModem, Qt::UniqueConnection);
        connect(m_modemNetwork.data(), &ModemManager::Modem::destroyed, this, &ConnectionIcon::modemNetworkRemoved);

        m_signal = m_modemNetwork->signalQuality().signal;
        setIconForModem();
    } else {
        setConnectionIcon(QStringLiteral("network-mobile-0"));
        setConnectionTooltipIcon(QStringLiteral("phone"));
        return;
    }
}

void ConnectionIcon::setIconForModem()
{
    if (!m_signal) {
        m_signal = m_modemNetwork->signalQuality().signal;
    }
    QString strength = QStringLiteral("00");

    if (m_signal == 0) {
        strength = QLatin1Char('0');
    } else if (m_signal < 20) {
        strength = QStringLiteral("20");
    } else if (m_signal < 40) {
        strength = QStringLiteral("40");
    } else if (m_signal < 60) {
        strength = QStringLiteral("60");
    } else if (m_signal < 80) {
        strength = QStringLiteral("80");
    } else {
        strength = QStringLiteral("100");
    }

    QString result;

    switch (m_modemNetwork->accessTechnologies()) {
    case MM_MODEM_ACCESS_TECHNOLOGY_GSM:
    case MM_MODEM_ACCESS_TECHNOLOGY_GSM_COMPACT:
        result = QStringLiteral("network-mobile-%1");
        break;
    case MM_MODEM_ACCESS_TECHNOLOGY_GPRS:
        result = QStringLiteral("network-mobile-%1-gprs");
        break;
    case MM_MODEM_ACCESS_TECHNOLOGY_EDGE:
        result = QStringLiteral("network-mobile-%1-edge");
        break;
    case MM_MODEM_ACCESS_TECHNOLOGY_UMTS:
        result = QStringLiteral("network-mobile-%1-umts");
        break;
    case MM_MODEM_ACCESS_TECHNOLOGY_HSDPA:
        result = QStringLiteral("network-mobile-%1-hsdpa");
        break;
    case MM_MODEM_ACCESS_TECHNOLOGY_HSUPA:
        result = QStringLiteral("network-mobile-%1-hsupa");
        break;
    case MM_MODEM_ACCESS_TECHNOLOGY_HSPA:
    case MM_MODEM_ACCESS_TECHNOLOGY_HSPA_PLUS:
        result = QStringLiteral("network-mobile-%1-hspa");
        break;
    case MM_MODEM_ACCESS_TECHNOLOGY_LTE:
        result = QStringLiteral("network-mobile-%1-lte");
        break;
#if MM_CHECK_VERSION(1, 14, 0)
    case MM_MODEM_ACCESS_TECHNOLOGY_5GNR:
        result = QStringLiteral("network-mobile-%1-5g");
        break;
#endif
    default:
        result = QStringLiteral("network-mobile-%1");
        break;
    }

    setConnectionIcon(QString(result).arg(strength));
    setConnectionTooltipIcon(QStringLiteral("phone"));
}

void ConnectionIcon::setWirelessIcon(const NetworkManager::Device::Ptr &device, const QString &ssid)
{
    NetworkManager::WirelessDevice::Ptr wirelessDevice = device.objectCast<NetworkManager::WirelessDevice>();
    if (device) {
        m_wirelessNetwork = wirelessDevice->findNetwork(ssid);
    } else {
        m_wirelessNetwork.clear();
    }

    if (m_wirelessNetwork) {
        connect(m_wirelessNetwork.data(),
                &NetworkManager::WirelessNetwork::signalStrengthChanged,
                this,
                &ConnectionIcon::setWirelessIconForSignalStrength,
                Qt::UniqueConnection);

        setWirelessIconForSignalStrength(m_wirelessNetwork->signalStrength());
    } else {
        setDisconnectedIcon();
    }
}

void ConnectionIcon::setWirelessIconForSignalStrength(int strength)
{
    int iconStrength = 100;
    if (strength == 0) {
        iconStrength = 0;
        setConnectionTooltipIcon(QStringLiteral("network-wireless-connected-00"));
    } else if (strength < 20) {
        iconStrength = 20;
        setConnectionTooltipIcon(QStringLiteral("network-wireless-connected-20"));
    } else if (strength < 40) {
        iconStrength = 40;
        setConnectionTooltipIcon(QStringLiteral("network-wireless-connected-40"));
    } else if (strength < 60) {
        iconStrength = 60;
        setConnectionTooltipIcon(QStringLiteral("network-wireless-connected-60"));
    } else if (strength < 80) {
        iconStrength = 80;
        setConnectionTooltipIcon(QStringLiteral("network-wireless-connected-80"));
    } else if (strength < 100) {
        setConnectionTooltipIcon(QStringLiteral("network-wireless-connected-100"));
    }

    const QString icon = QStringLiteral("network-wireless-%1").arg(iconStrength);

    setConnectionIcon(icon);
}

void ConnectionIcon::setConnecting(bool connecting)
{
    if (connecting != m_connecting) {
        m_connecting = connecting;
        Q_EMIT connectingChanged(m_connecting);
    }
}

void ConnectionIcon::setConnectionIcon(const QString &icon)
{
    if (icon != m_connectionIcon) {
        m_connectionIcon = icon;
        Q_EMIT connectionIconChanged(connectionIcon());
    }
}

void ConnectionIcon::setConnectionTooltipIcon(const QString &icon)
{
    if (icon != m_connectionTooltipIcon) {
        m_connectionTooltipIcon = icon;
        Q_EMIT connectionTooltipIconChanged(m_connectionTooltipIcon);
    }
}

void ConnectionIcon::setVpn(bool vpn)
{
    if (m_vpn != vpn) {
        m_vpn = vpn;
        Q_EMIT connectionIconChanged(connectionIcon());
    }
}

#include "moc_connectionicon.cpp"
