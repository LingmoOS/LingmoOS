// SPDX-FileCopyrightText: 2018 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef IPMANAGER_H
#define IPMANAGER_H

#include <NetworkManagerQt/IpConfig>
#include <NetworkManagerQt/Device>

namespace dde {
namespace network {

class IpManager : public QObject
{
    Q_OBJECT

public:
    explicit IpManager(NetworkManager::Device::Ptr device, QObject *parent = nullptr);
    ~IpManager() = default;
    NetworkManager::IpAddresses ipAddresses() const;

signals:
    void ipChanged();

private:
    bool changeIpv4Config(const QString &uni);

private slots:
    void onPropertiesChanged(const QString &interfaceName, const QVariantMap &changedProperties, const QStringList &invalidatedProperties);
    void onDevicePropertiesChanged(const QString &interfaceName, const QVariantMap &changedProperties, const QStringList &invalidatedProperties);

private:
    NetworkManager::IpAddresses m_ipAddresses;
    NetworkManager::Device::Ptr m_device;
};

}
}

#endif // VPNCONTROLLER_NM_H
