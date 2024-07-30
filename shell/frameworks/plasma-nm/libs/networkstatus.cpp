/*
    SPDX-FileCopyrightText: 2013 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "networkstatus.h"
#include "uiutils.h"

#include <QDBusConnection>
#include <QDBusConnectionInterface>

#include <NetworkManagerQt/ActiveConnection>
#include <NetworkManagerQt/Connection>

#include <KLocalizedString>

NetworkStatus::SortedConnectionType NetworkStatus::connectionTypeToSortedType(NetworkManager::ConnectionSettings::ConnectionType type)
{
    switch (type) {
    case NetworkManager::ConnectionSettings::Adsl:
        return NetworkStatus::NetworkStatus::Adsl;
        break;
    case NetworkManager::ConnectionSettings::Bluetooth:
        return NetworkStatus::Bluetooth;
        break;
    case NetworkManager::ConnectionSettings::Cdma:
        return NetworkStatus::Cdma;
        break;
    case NetworkManager::ConnectionSettings::Gsm:
        return NetworkStatus::Gsm;
        break;
    case NetworkManager::ConnectionSettings::Infiniband:
        return NetworkStatus::Infiniband;
        break;
    case NetworkManager::ConnectionSettings::OLPCMesh:
        return NetworkStatus::OLPCMesh;
        break;
    case NetworkManager::ConnectionSettings::Pppoe:
        return NetworkStatus::Pppoe;
        break;
    case NetworkManager::ConnectionSettings::Vpn:
        return NetworkStatus::Vpn;
        break;
    case NetworkManager::ConnectionSettings::Wired:
        return NetworkStatus::Wired;
        break;
    case NetworkManager::ConnectionSettings::Wireless:
        return NetworkStatus::Wireless;
        break;
    case NetworkManager::ConnectionSettings::WireGuard:
        return NetworkStatus::Wireguard;
        break;
    default:
        return NetworkStatus::Other;
        break;
    }
}

NetworkStatus::NetworkStatus(QObject *parent)
    : QObject(parent)
{
    connect(NetworkManager::notifier(), &NetworkManager::Notifier::connectivityChanged, this, &NetworkStatus::changeActiveConnections);
    connect(NetworkManager::notifier(), &NetworkManager::Notifier::connectivityChanged, this, &NetworkStatus::connectivityChanged);
    connect(NetworkManager::notifier(), &NetworkManager::Notifier::statusChanged, this, &NetworkStatus::statusChanged);
    connect(NetworkManager::notifier(), &NetworkManager::Notifier::activeConnectionsChanged, this, QOverload<>::of(&NetworkStatus::activeConnectionsChanged));

    activeConnectionsChanged();
    statusChanged(NetworkManager::status());

    QDBusPendingReply<uint> pendingReply = NetworkManager::checkConnectivity();
    auto callWatcher = new QDBusPendingCallWatcher(pendingReply);
    connect(callWatcher, &QDBusPendingCallWatcher::finished, this, [this](QDBusPendingCallWatcher *watcher) {
        QDBusPendingReply<uint> reply = *watcher;
        if (reply.isValid()) {
            connectivityChanged((NetworkManager::Connectivity)reply.value());
        }
        watcher->deleteLater();
    });
}

NetworkStatus::~NetworkStatus() = default;

QUrl NetworkStatus::networkCheckUrl()
{
    return QUrl(QStringLiteral("http://networkcheck.kde.org/"));
}

QString NetworkStatus::activeConnections() const
{
    return m_activeConnections;
}

QString NetworkStatus::networkStatus() const
{
    return m_networkStatus;
}

NetworkManager::Connectivity NetworkStatus::connectivity() const
{
    return NetworkManager::connectivity();
}

void NetworkStatus::activeConnectionsChanged()
{
    for (const NetworkManager::ActiveConnection::Ptr &active : NetworkManager::activeConnections()) {
        connect(active.data(), &NetworkManager::ActiveConnection::default4Changed, this, &NetworkStatus::defaultChanged, Qt::UniqueConnection);
        connect(active.data(), &NetworkManager::ActiveConnection::default6Changed, this, &NetworkStatus::defaultChanged, Qt::UniqueConnection);
        connect(active.data(), &NetworkManager::ActiveConnection::stateChanged, this, &NetworkStatus::changeActiveConnections);
    }

    changeActiveConnections();
}

void NetworkStatus::defaultChanged()
{
    statusChanged(NetworkManager::status());
}

void NetworkStatus::statusChanged(NetworkManager::Status status)
{
    const auto oldNetworkStatus = m_networkStatus;
    switch (status) {
    case NetworkManager::ConnectedLinkLocal:
        m_networkStatus = i18nc("A network device is connected, but there is only link-local connectivity", "Connected");
        break;
    case NetworkManager::ConnectedSiteOnly:
        m_networkStatus = i18nc("A network device is connected, but there is only site-local connectivity", "Connected");
        break;
    case NetworkManager::Connected:
        m_networkStatus = i18nc("A network device is connected, with global network connectivity", "Connected");
        break;
    case NetworkManager::Asleep:
        m_networkStatus = i18nc("Networking is inactive and all devices are disabled", "Inactive");
        break;
    case NetworkManager::Disconnected:
        m_networkStatus = i18nc("There is no active network connection", "Disconnected");
        break;
    case NetworkManager::Disconnecting:
        m_networkStatus = i18nc("Network connections are being cleaned up", "Disconnecting");
        break;
    case NetworkManager::Connecting:
        m_networkStatus = i18nc("A network device is connecting to a network and there is no other available network connection", "Connecting");
        break;
    default:
        m_networkStatus = checkUnknownReason();
        break;
    }

    if (status == NetworkManager::ConnectedLinkLocal || status == NetworkManager::ConnectedSiteOnly || status == NetworkManager::Connected) {
        changeActiveConnections();
    } else if (m_activeConnections != m_networkStatus) {
        m_activeConnections = m_networkStatus;
        Q_EMIT activeConnectionsChanged(m_activeConnections);
    }
}

void NetworkStatus::changeActiveConnections()
{
    if (NetworkManager::status() != NetworkManager::Connected && NetworkManager::status() != NetworkManager::ConnectedLinkLocal
        && NetworkManager::status() != NetworkManager::ConnectedSiteOnly) {
        return;
    }

    QString activeConnections;
    const QString format = QStringLiteral("%1: %2");

    QList<NetworkManager::ActiveConnection::Ptr> activeConnectionList = NetworkManager::activeConnections();
    std::sort(activeConnectionList.begin(),
              activeConnectionList.end(),
              [](const NetworkManager::ActiveConnection::Ptr &left, const NetworkManager::ActiveConnection::Ptr &right) {
                  return NetworkStatus::connectionTypeToSortedType(left->type()) < NetworkStatus::connectionTypeToSortedType(right->type());
              });

    for (const NetworkManager::ActiveConnection::Ptr &active : activeConnectionList) {
        if (!active->devices().isEmpty() && UiUtils::isConnectionTypeSupported(active->type())) {
            NetworkManager::Device::Ptr device = NetworkManager::findNetworkInterface(active->devices().first());
            if (device
                && ((device->type() != NetworkManager::Device::Generic && device->type() <= NetworkManager::Device::Team)
                    || device->type() == NetworkManager::Device::WireGuard)
                && device->type() != NetworkManager::Device::Loopback) {
                bool connecting = false;
                bool connected = false;
                QString conType;
                QString status;
                NetworkManager::VpnConnection::Ptr vpnConnection;

                if (active->vpn()) {
                    conType = i18n("VPN");
                    vpnConnection = active.objectCast<NetworkManager::VpnConnection>();
                } else {
                    conType = UiUtils::interfaceTypeLabel(device->type(), device);
                }

                if (vpnConnection && active->vpn()) {
                    if (vpnConnection->state() >= NetworkManager::VpnConnection::Prepare
                        && vpnConnection->state() <= NetworkManager::VpnConnection::GettingIpConfig) {
                        connecting = true;
                    } else if (vpnConnection->state() == NetworkManager::VpnConnection::Activated) {
                        connected = true;
                    }
                } else {
                    if (active->state() == NetworkManager::ActiveConnection::Activated) {
                        connected = true;
                    } else if (active->state() == NetworkManager::ActiveConnection::Activating) {
                        connecting = true;
                    }
                }

                if (active->type() == NetworkManager::ConnectionSettings::ConnectionType::WireGuard) {
                    conType = i18n("WireGuard");
                    connected = true;
                }

                NetworkManager::Connection::Ptr connection = active->connection();
                if (connecting) {
                    status = i18n("Connecting to %1", connection->name());
                } else if (connected) {
                    switch (NetworkManager::connectivity()) {
                    case NetworkManager::NoConnectivity:
                        status = i18n("Connected to %1 (no connectivity)", connection->name());
                        break;
                    case NetworkManager::Limited:
                        status = i18n("Connected to %1 (limited connectivity)", connection->name());
                        break;
                    case NetworkManager::Portal:
                        status = i18n("Connected to %1 (log in required)", connection->name());
                        break;
                    default:
                        status = i18n("Connected to %1", connection->name());
                        break;
                    }
                }

                if (!activeConnections.isEmpty()) {
                    activeConnections += QLatin1Char('\n');
                }
                activeConnections += format.arg(conType, status);

                connect(connection.data(), &NetworkManager::Connection::updated, this, &NetworkStatus::changeActiveConnections, Qt::UniqueConnection);
            }
        }
    }

    if (m_activeConnections != activeConnections) {
        m_activeConnections = activeConnections;
        Q_EMIT activeConnectionsChanged(activeConnections);
    }
}

QString NetworkStatus::checkUnknownReason() const
{
    // Check if NetworkManager is running.
    if (!QDBusConnection::systemBus().interface()->isServiceRegistered(QStringLiteral(NM_DBUS_INTERFACE))) {
        return i18n("NetworkManager not running");
    }

    // Check for compatible NetworkManager version.
    if (NetworkManager::compareVersion(0, 9, 8) < 0) {
        return i18n("NetworkManager 0.9.8 required, found %1.", NetworkManager::version());
    }

    return i18nc("global connection state", "Unknown");
}

#include "moc_networkstatus.cpp"
