// SPDX-FileCopyrightText: 2023 Yari Polla <skilvingr@gmail.com>
// SPDX-License-Identifier: LGPL-2.0-or-later

#include "wirelessstatus.h"

#include <NetworkManagerQt/ActiveConnection>
#include <NetworkManagerQt/Manager>
#include <NetworkManagerQt/WirelessSetting>

WirelessStatus::WirelessStatus(QObject *parent)
    : QObject(parent)
{
    connect(NetworkManager::notifier(), &NetworkManager::Notifier::activeConnectionsChanged, this, &WirelessStatus::activeConnectionsChanged);

    activeConnectionsChanged();
}

WirelessStatus::~WirelessStatus() = default;

QString WirelessStatus::wifiSSID() const
{
    return m_wifiSSID;
}

QString WirelessStatus::hotspotSSID() const
{
    return m_hotspotSSID;
}

void WirelessStatus::activeConnectionsChanged()
{
    for (const NetworkManager::ActiveConnection::Ptr &active : NetworkManager::activeConnections()) {
        connect(active.data(), &NetworkManager::ActiveConnection::stateChanged, this, &WirelessStatus::stateChanged);

        Q_EMIT active.data()->stateChanged(active->state());
    }
}

void WirelessStatus::stateChanged()
{
    auto connection = qobject_cast<NetworkManager::ActiveConnection *>(QObject::sender());

    if (connection->type() == NetworkManager::ConnectionSettings::ConnectionType::Wireless) {
        NetworkManager::WirelessSetting::Ptr wifiSetting =
            connection->connection()->settings()->setting(NetworkManager::Setting::Wireless).dynamicCast<NetworkManager::WirelessSetting>();

        bool connected = connection->state() == NetworkManager::ActiveConnection::Activated;

        if (wifiSetting->mode() == NetworkManager::WirelessSetting::Ap || wifiSetting->mode() == NetworkManager::WirelessSetting::Adhoc) {
            if (connected) {
                m_hotspotSSID = connection->connection()->name();
            } else {
                m_hotspotSSID = "";
            }

            Q_EMIT hotspotSSIDChanged(m_hotspotSSID);
        } else {
            if (connected) {
                m_wifiSSID = connection->connection()->name();
            } else {
                m_wifiSSID = "";
            }

            Q_EMIT wifiSSIDChanged(m_wifiSSID);
        }
    }
}

#include "moc_wirelessstatus.cpp"
