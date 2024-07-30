/*
    SPDX-FileCopyrightText: 2013 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef PLASMA_NM_NETWORK_STATUS_H
#define PLASMA_NM_NETWORK_STATUS_H

#include <QObject>
#include <QUrl>

#include <qqmlregistration.h>

#include <NetworkManagerQt/Manager>

class NetworkStatus : public QObject
{
    Q_OBJECT
    QML_ELEMENT

    /**
     * Returns a formated list of active connections or NM status when there is no active connection
     */
    Q_PROPERTY(QString activeConnections READ activeConnections NOTIFY activeConnectionsChanged)

    /**
     * Returns the KDE portal network check website URL
     */
    Q_PROPERTY(QUrl networkCheckUrl READ networkCheckUrl CONSTANT)

    /**
     * Returns the network connectivity state
     */
    Q_PROPERTY(NetworkManager::Connectivity connectivity READ connectivity NOTIFY connectivityChanged)
public:
    enum SortedConnectionType {
        Wired,
        Wireless,
        Gsm,
        Cdma,
        Pppoe,
        Adsl,
        Infiniband,
        OLPCMesh,
        Bluetooth,
        Wireguard,
        Vpn,
        Other,
    };

    static SortedConnectionType connectionTypeToSortedType(NetworkManager::ConnectionSettings::ConnectionType type);

    explicit NetworkStatus(QObject *parent = nullptr);
    ~NetworkStatus() override;

    static QUrl networkCheckUrl();

    QString activeConnections() const;
    QString networkStatus() const;
    NetworkManager::Connectivity connectivity() const;

private Q_SLOTS:
    void activeConnectionsChanged();
    void defaultChanged();
    void statusChanged(NetworkManager::Status status);
    void changeActiveConnections();

Q_SIGNALS:
    void activeConnectionsChanged(const QString &activeConnections);
    void connectivityChanged(NetworkManager::Connectivity connectivity);

private:
    QString m_activeConnections;
    QString m_networkStatus;
    NetworkManager::Connectivity m_connectivity = NetworkManager::UnknownConnectivity;

    QString checkUnknownReason() const;
};

#endif // PLAMA_NM_NETWORK_STATUS_H
