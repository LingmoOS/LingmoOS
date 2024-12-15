// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef WIRELESSCONNECT_H
#define WIRELESSCONNECT_H

#include <NetworkManagerQt/WirelessSecuritySetting>
#include <NetworkManagerQt/WirelessSecuritySetting>
#include <NetworkManagerQt/Connection>

namespace ocean {
    namespace network {
        class NetworkDeviceBase;
        class WiredDevice;
        class WirelessDevice;
        class AccessPoints;
        class WiredConnection;
        enum class DeviceType;
    } // namespace network
} // namespace ocean

class WirelessConnect : public QObject
{
    Q_OBJECT

public:
    explicit WirelessConnect(QObject *parent, ocean::network::WirelessDevice *device, ocean::network::AccessPoints *ap);
    ~WirelessConnect() Q_DECL_OVERRIDE;

    void setSsid(const QString &ssid);
    bool passwordIsValid(const QString &password);
    void getoldPassword();

protected:
    void setPassword(const QString &password);
    bool hasPassword();
    void initConnection();
    void activateConnection();
    NetworkManager::WirelessSecuritySetting::KeyMgmt getKeyMgmtByAp(ocean::network::AccessPoints *ap);

public Q_SLOTS:
    void connectNetwork();
    void connectNetworkPassword(const QString password);

Q_SIGNALS:
    void passwordError(const QString oldPassword);

private:
    ocean::network::WirelessDevice *m_device;
    ocean::network::AccessPoints *m_accessPoint;
    QString m_ssid;
    bool m_needUpdate;

    NetworkManager::ConnectionSettings::Ptr m_connectionSettings;
};

#endif //  WIRELESSCONNECT_H
