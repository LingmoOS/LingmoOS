// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef NETWORKINITIALIZATION_H
#define NETWORKINITIALIZATION_H

#include "constants.h"

#include <QObject>

namespace NetworkManager {
class WiredDevice;
class Device;
class Connection;
}

namespace network {
namespace systemservice {

class NetworkInitialization : public QObject
{
    Q_OBJECT

public:
    static void doInit();

protected:
    explicit NetworkInitialization(QObject *parent = nullptr);
    ~NetworkInitialization() = default;
    void initDeviceInfo();
    void initConnection();

private:
    void addFirstConnection(const QSharedPointer<NetworkManager::WiredDevice> &device);
    bool hasConnection(const QSharedPointer<NetworkManager::WiredDevice> &device, QList<QSharedPointer<NetworkManager::Connection>> &unSaveDevices);
    QString connectionMatchName() const;
    void installTranslator(const QString &locale);
    void hideWirelessDevice(const QSharedPointer<NetworkManager::Device> &device, bool disableNetwork);
    void initDeviceConnection(const QSharedPointer<NetworkManager::WiredDevice> &device);

private slots:
    void onUserChanged(const QString &json);
    void onInitDeviceConnection();

private:
    QStringList m_newConnectionNames;
    bool m_initilized;
    QStringList m_creatingDevices;
};

}
}

#endif // SERVICE_H
