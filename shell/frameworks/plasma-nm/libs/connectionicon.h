/*
    SPDX-FileCopyrightText: 2013 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef PLASMA_NM_CONNECTION_ICON_H
#define PLASMA_NM_CONNECTION_ICON_H

#include <ModemManagerQt/Modem>
#include <NetworkManagerQt/ActiveConnection>
#include <NetworkManagerQt/Manager>
#include <NetworkManagerQt/VpnConnection>
#include <NetworkManagerQt/WirelessNetwork>

#include <QCoroCore>

#include <qqmlregistration.h>

class ConnectionIcon : public QObject
{
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(bool connecting READ connecting NOTIFY connectingChanged)
    Q_PROPERTY(QString connectionIcon READ connectionIcon NOTIFY connectionIconChanged)
    Q_PROPERTY(QString connectionTooltipIcon READ connectionTooltipIcon NOTIFY connectionTooltipIconChanged)

    Q_PROPERTY(NetworkManager::Connectivity connectivity READ connectivity WRITE setConnectivity NOTIFY connectivityChanged)
public:
    explicit ConnectionIcon(QObject *parent = nullptr);
    ~ConnectionIcon() override;

    bool connecting() const;
    QString connectionIcon() const;
    QString connectionTooltipIcon() const;

    NetworkManager::Connectivity connectivity() const;
    void setConnectivity(NetworkManager::Connectivity connectivity);
    Q_SIGNAL void connectivityChanged(NetworkManager::Connectivity connectivity);

private Q_SLOTS:
    void activatingConnectionChanged(const QString &connection);
    void activeConnectionAdded(const QString &activeConnection);
    void activeConnectionDestroyed();
    void activeConnectionStateChanged(NetworkManager::ActiveConnection::State state);
    void carrierChanged(bool carrier);
    void deviceAdded(const QString &device);
    void deviceRemoved(const QString &device);
    void networkingEnabledChanged(bool enabled);
    void primaryConnectionChanged(const QString &connection);
    void modemNetworkRemoved();
    void modemSignalChanged(ModemManager::SignalQualityPair signalQuality);
    void setIconForModem();
    void statusChanged(NetworkManager::Status status);
    void setWirelessIconForSignalStrength(int strength);
    void vpnConnectionStateChanged(NetworkManager::VpnConnection::State state, NetworkManager::VpnConnection::StateChangeReason reason);
    void wirelessEnabledChanged(bool enabled);
    void wirelessNetworkAppeared(const QString &network);
    void wwanEnabledChanged(bool enabled);
Q_SIGNALS:
    void connectingChanged(bool connecting);
    void connectionIconChanged(const QString &icon);
    void connectionTooltipIconChanged(const QString &icon);

private:
    void addActiveConnection(const QString &activeConnection);
    void setConnecting(bool connecting);
    void setConnectionIcon(const QString &icon);
    void setConnectionTooltipIcon(const QString &icon);
    void setVpn(bool vpn);
    uint m_signal = 0;
    NetworkManager::WirelessNetwork::Ptr m_wirelessNetwork;

    bool m_connecting = false;
    bool m_limited = false;
    bool m_vpn = false;
    QString m_connectionIcon;
    QString m_connectionTooltipIcon;
    NetworkManager::Connectivity m_connectivity = NetworkManager::UnknownConnectivity;

    void setDisconnectedIcon();
    void setIcons();
    void setStates();
    void setWirelessIcon(const NetworkManager::Device::Ptr &device, const QString &ssid);
    ModemManager::Modem::Ptr m_modemNetwork;
    void setModemIcon(const NetworkManager::Device::Ptr &device);
};

#endif // PLASMA_NM_CONNECTION_ICON_H
