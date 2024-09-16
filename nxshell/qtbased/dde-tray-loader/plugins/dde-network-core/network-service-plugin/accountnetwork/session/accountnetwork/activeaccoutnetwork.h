// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef ACTIVEACCOUNTNETWORK_H
#define ACTIVEACCOUNTNETWORK_H

#include <QObject>
#include <QVariantMap>
#include <NetworkManagerQt/ActiveConnection>

namespace NetworkManager {
class Device;
class ActiveConnection;
}

namespace accountnetwork {
namespace sessionservice {

class Account;
class NetworkActivator;
class AccountNetworkConfig;

class ActiveAccountNetwork : public QObject
{
    Q_OBJECT

public:
    explicit ActiveAccountNetwork(QObject *parent = nullptr);
    ~ActiveAccountNetwork() = default;

private:
    void init();
    void initConnection();
    void initDevice();
    void addDevice(const QSharedPointer<NetworkManager::Device> &device);
    QMap<QString, QString> accountNetwork() const;
    QVariantMap authenInfo() const;
    bool canResetCurrentNetwork(const QSharedPointer<NetworkManager::Device> &device, const QMap<QString, QString> &network,
                                const QSharedPointer<NetworkManager::ActiveConnection> &activeConnection) const;
    void onConnectionStateChanged(const QSharedPointer<NetworkManager::Device> &device,
                                 const QSharedPointer<NetworkManager::ActiveConnection> &activeConnection);
    void handlerConnectionSuccess(const QSharedPointer<NetworkManager::Device> &device,
                                  const QSharedPointer<NetworkManager::ActiveConnection> &activeConnection);
    void activeNetwork(const QMap<QString, QString> &network, const QVariantMap &authenInfo);
    bool secretIsPrepare() const;

private slots:
    void onAuthen(const QVariantMap &identity);
    void onCollectionCreated(const QDBusObjectPath &path);

private:
    Account *m_account;
    AccountNetworkConfig *m_config;
    NetworkActivator *m_networkActivator;
    bool m_networkregisted;
    bool m_secretregisted;
    QMap<QString, QString> m_network;
    QVariantMap m_authenInfo;
};

}
}

#endif // SERVICE_H
