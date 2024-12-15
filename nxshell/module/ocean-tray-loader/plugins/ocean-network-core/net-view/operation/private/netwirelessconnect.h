// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef NETWIRELESSCONNECT_H
#define NETWIRELESSCONNECT_H

#include <NetworkManagerQt/Connection>
#include <NetworkManagerQt/Security8021xSetting>
#include <NetworkManagerQt/WirelessSecuritySetting>

namespace ocean {
namespace network {

class NetworkDeviceBase;
class WiredDevice;
class WirelessDevice;
class AccessPoints;
class WiredConnection;
enum class DeviceType;

class NetWirelessConnect : public QObject
{
    Q_OBJECT

public:
    explicit NetWirelessConnect(ocean::network::WirelessDevice *device, ocean::network::AccessPoints *ap, QObject *parent = nullptr);
    ~NetWirelessConnect() Q_DECL_OVERRIDE;

    void setSsid(const QString &ssid);
    bool initConnection();

    bool passwordIsValid(const QString &password);
    void updateProp();

    QString needSecrets();
    bool needInputPassword();
    bool needInputIdentify();

protected:
    void setPassword(const QString &password);
    bool hasPassword();
    void activateConnection();
    NetworkManager::WirelessSecuritySetting::KeyMgmt getKeyMgmtByAp(ocean::network::AccessPoints *ap);
    NetworkManager::Security8021xSetting::EapMethod getEapMethod() const;
    NetworkManager::Security8021xSetting::AuthMethod getAuthMethod() const;
    QVariantMap setAuthen(const QVariantMap &param);

public Q_SLOTS:
    void connectNetwork();
    QVariantMap connectNetworkParam(const QVariantMap &param);

private:
    ocean::network::WirelessDevice *m_device;
    ocean::network::AccessPoints *m_accessPoint;
    QString m_ssid;
    bool m_needUpdate;
    bool m_needIdentify;

    NetworkManager::ConnectionSettings::Ptr m_connectionSettings;
};

} // namespace network
} // namespace ocean

#endif //  NETWIRELESSCONNECT_H
