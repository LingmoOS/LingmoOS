// SPDX-FileCopyrightText: 2018 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ACCESSPOINT_NM_H
#define ACCESSPOINT_NM_H

#include "netinterface.h"

#include <QObject>

#include <NetworkManagerQt/WirelessDevice>
#include <NetworkManagerQt/WirelessNetwork>

namespace dde {
namespace network {

class AccessPointProxyNM : public AccessPointProxy
{
    Q_OBJECT

public:
    AccessPointProxyNM(NetworkManager::WirelessDevice::Ptr device, NetworkManager::WirelessNetwork::Ptr network, QObject *parent = nullptr);
    ~AccessPointProxyNM() override;

    void updateStatus(ConnectionStatus status);
    void updateNetwork(NetworkManager::WirelessNetwork::Ptr network);
    bool contains(const QString &uni) const;

public:
    QString ssid() const override;
    int strength() const override;
    bool secured() const override;
    bool securedInEap() const override;
    int frequency() const override;
    QString path() const override;
    QString devicePath() const override;
    bool connected() const override;
    ConnectionStatus status() const override;
    bool hidden() const override;
    bool isWlan6() const override;

private:
    void initState();
    void initConnection();
    void updateInfo();
    void updateConnection();
    void updateHiddenInfo();

private slots:
    void onUpdateNetwork();

private:
    NetworkManager::WirelessDevice::Ptr m_device;
    NetworkManager::WirelessNetwork::Ptr m_network;
    ConnectionStatus m_status;
    bool m_isHidden;
    int m_strength;
    bool m_secured;
    QList<QMetaObject::Connection> m_connectionList;
};

}
}

#endif // ACCESSPOINT_NM_H
