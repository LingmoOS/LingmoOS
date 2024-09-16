// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef NETWORKHANDLER_H
#define NETWORKHANDLER_H

#include <QObject>

namespace NetworkManager {
class ActiveConnection;
class Device;
}

namespace accountnetwork {
namespace systemservice {

class AccountManager;
class NetworkConfig;

class NetworkHandler : public QObject
{
    Q_OBJECT

public:
    NetworkHandler(AccountManager *accountManager, NetworkConfig *conf, QObject *parent = Q_NULLPTR);
    ~NetworkHandler();
    void disconnectNetwork();

private:
    void initDevices();
    void initConnection();
    QString getSaveId(const QSharedPointer<NetworkManager::ActiveConnection> &activeConnection, bool isWireless);
    QSharedPointer<NetworkManager::Device> getDeviceByInterfaceName(const QString &interfaceName) const;
    void deviceActiveHandler(const QSharedPointer<NetworkManager::Device> &device);

private slots:
    void onDeviceAdded(const QString &uni);

private:
    AccountManager *m_accountManager;
    NetworkConfig *m_networkConfig;
};

}
}

#endif // NETWORKSERVICE_H
