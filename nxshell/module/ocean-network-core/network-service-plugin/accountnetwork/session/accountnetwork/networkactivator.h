// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef NETWORKACTIVATOR_H
#define NETWORKACTIVATOR_H

#include <QObject>
#include <QVariantMap>

namespace NetworkManager {
class WirelessNetwork;
class WiredDevice;
class WirelessDevice;
class Device;
class Connection;
}

namespace accountnetwork {
namespace sessionservice {

class AccountNetworkConfig;

class NetworkActivator : public QObject
{
    Q_OBJECT

public:
    explicit NetworkActivator(AccountNetworkConfig *config, QObject *parent = nullptr);
    ~NetworkActivator();
    void activeNetwork(const QMap<QString, QString> &network, const QVariantMap &certify);
    void activeNetwork(const QSharedPointer<NetworkManager::Device> &device, const QString &id, const QVariantMap &certify);

private:
    void activeWirelessNetwork(const QSharedPointer<NetworkManager::WirelessDevice> &wirelessDevice,
                               const QSharedPointer<NetworkManager::WirelessNetwork> network, const QVariantMap &certify);
    void activeWiredNetwork(const QSharedPointer<NetworkManager::WiredDevice> &wiredDevice,
                               const QString &id, const QVariantMap &certify);

    QString decryptPassword(const QString &password) const;
    bool deviceEnabled(const QSharedPointer<NetworkManager::Device> &device) const;
    QSharedPointer<NetworkManager::Connection> getConnection(const QSharedPointer<NetworkManager::WirelessDevice> &device, const QString &ssid) const;

private:
    AccountNetworkConfig *m_config;
};

}
}

#endif // SERVICE_H
