// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DEVICEMANAGERREALIZE_H
#define DEVICEMANAGERREALIZE_H

#include "netinterface.h"

#include <NetworkManagerQt/WiredDevice>
#include <NetworkManagerQt/WirelessDevice>
#include <NetworkManagerQt/WirelessSecuritySetting>

#include <QDBusReply>

namespace NetworkManager {
class WirelessNetwork;
class AccessPoint;
}

class AccessPointInfo;

namespace dde {
namespace network {

class ProcesserInterface;
class IpConfig;

class DeviceManagerRealize : public NetworkDeviceRealize
{
    Q_OBJECT

public:
    explicit DeviceManagerRealize(NetworkManager::Device::Ptr device, QObject *parent = nullptr);
    ~DeviceManagerRealize() override;

protected:
    bool isEnabled() const override;                                                      // 当前的网卡是否启用
    QString interface() const override;                                                   // 返回设备上的Interface
    QString driver() const override;                                                      // 驱动，对应于备上返回值的Driver
    bool managed() const override;                                                        // 对应于设备上返回值的Managed
    QString vendor() const override;                                                      // 对应于设备上返回值的Vendor
    QString uniqueUuid() const override;                                                  // 网络设备的唯一的UUID，对应于设备上返回值的UniqueUuid
    bool usbDevice() const override;                                                      // 是否是USB设备，对应于设备上返回值的UsbDevice
    QString path() const override;                                                        // 设备路径，对应于设备上返回值的Path
    QString activeAp() const override;                                                    // 对应于设备上返回值的ActiveAp
    bool supportHotspot() const override;                                                 // 是否支持热点,对应于设备上返回值的SupportHotspot
    QString usingHwAdr() const override;                                                  // 正在使用的mac地址
    const QStringList ipv4() override;                                                    // IPV4地址
    const QStringList ipv6() override;                                                    // IPV6地址
    QJsonObject activeConnectionInfo() const override;                                    // 获取当前活动连接的信息
    void setEnabled(bool enabled) override;                                               // 开启或禁用网卡
    void disconnectNetwork() override;                                                    // 断开网络连接，该方法是一个虚方法，具体在子类
    DeviceStatus deviceStatus() const override;                                           // 返回设备的状态

protected:
    virtual void addConnection(const NetworkManager::Connection::Ptr &connection) = 0;
    virtual void removeConnection(const QString &connection) = 0;
    virtual void onActiveConnectionChanged() = 0;
    virtual void deviceEnabledChanged(bool) {}
    virtual void deviceEnabledAction(const QDBusReply<QDBusObjectPath> &, bool) {}

private:
    void initConnection();
    void initUsbInfo();
    void initEnabeld();
    void resetConfig(const QDBusObjectPath &ipv4ConfigPath);

protected:
    void updateWiredConnections();

private slots:
    void onDeviceEnabledChanged(QDBusObjectPath path, bool enabled);
    void onConnectionAdded(const QString &connectionUni);
    void onDeviceStateChanged();
    void onDevicePropertiesChanged(const QString &interfaceName, const QVariantMap &changedProperties, const QStringList &invalidatedProperties);

private:
    QSharedPointer<NetworkManager::Device> m_device;
    QSharedPointer<IpConfig> m_ipv4Config;
    bool m_isUsbDevice;
    bool m_isEnabeld;
};

// 有线连接
class WiredDeviceManagerRealize : public DeviceManagerRealize
{
    Q_OBJECT

public:
    WiredDeviceManagerRealize(NetworkManager::WiredDevice::Ptr device, QObject *parent = Q_NULLPTR);
    ~WiredDeviceManagerRealize() override;

protected:
    QString realHwAdr() const override;
    bool connectNetwork(WiredConnection *connection) override;                                              // 连接网络，连接成功抛出deviceStatusChanged信号
    QList<WiredConnection *> wiredItems() const override;                                                   // 有线网络连接列表
    void addConnection(const NetworkManager::Connection::Ptr &connection) override;                         // 新增有线网络连接
    void removeConnection(const QString &connectionUni) override;                                           // 移除有线网络连接
    void onActiveConnectionChanged() override;                                                              // 活动连接变化
    void deviceEnabledAction(const QDBusReply<QDBusObjectPath> &reply, bool enabled) override;
    QString usingHwAdr() const override;
    bool carrier() const override;                                                                          // 是否插入网线

private:
    NetworkManager::WiredDevice::Ptr m_device;
    QList<WiredConnection *> m_wiredConnections;
};

// 无线连接
class WirelessDeviceManagerRealize : public DeviceManagerRealize
{
    Q_OBJECT

public:
    WirelessDeviceManagerRealize(NetworkManager::WirelessDevice::Ptr device, QObject *parent = Q_NULLPTR);
    ~WirelessDeviceManagerRealize() override;
    void addProcesser(ProcesserInterface *processer);
    static NetworkManager::WirelessSecuritySetting::KeyMgmt getKeyMgmtByAp(const NetworkManager::AccessPoint::Ptr &accessPoint);
    static bool checkKeyMgmt(const NetworkManager::AccessPoint::Ptr &accessPoint, NetworkManager::WirelessSecuritySetting::KeyMgmt keyMgmt);

protected:
    bool available() const override;
    bool supportHotspot() const override;
    QString activeAp() const override;
    QString realHwAdr() const override;
    QList<AccessPoints *> accessPointItems() const override;                              // 当前网卡上所有的网络列表
    void scanNetwork() override;                                                          // 重新加载所有的无线网络列表
    void connectNetwork(const AccessPoints *accessPoint) override;                        // 连接网络，连接成功抛出deviceStatusChanged信号
    QList<WirelessConnection *> wirelessItems() const override;                           // 无线网络连接列表
    AccessPoints *activeAccessPoints() const override;                                    // 当前活动的无线连接

    void addConnection(const NetworkManager::Connection::Ptr &connection) override;       // 新增无线网络连接
    void removeConnection(const QString &connectionUni) override;
    void onActiveConnectionChanged() override;
    DeviceStatus deviceStatus() const override;                                           // 返回设备的状态
    void deviceEnabledChanged(bool enabled) override;                                     // 设备启用状态发生变化
    void deviceEnabledAction(const QDBusReply<QDBusObjectPath> &reply, bool enabled) override;
    QString usingHwAdr() const override;

private:
    bool getHotspotIsEnabled() const;
    bool hotspotEnabled() override;
    void addNetwork(const NetworkManager::WirelessNetwork::Ptr &network);
    WirelessConnection *findConnection(const QString &path) const;

private Q_SLOTS:
    void onNetworkAppeared(const QString &ssid);
    void onNetworkDisappeared(const QString &ssid);
    void onInterfaceFlagsChanged();

private:
    NetworkManager::WirelessDevice::Ptr m_device;
    QList<WirelessConnection *> m_wirelessConnections;
    QList<AccessPointInfo *> m_accessPointInfos;
    bool m_hotspotEnabled;
    ProcesserInterface *m_netProcesser;
    bool m_available;
};

class IpConfig : public QObject
{
    Q_OBJECT

public:
    explicit IpConfig(NetworkManager::IpConfig config, const QString &uni, QObject *parent = nullptr);
    ~IpConfig() = default;

signals:
    void ipChanged();

private slots:
    void onPropertiesChanged(const QString &interfaceName, const QVariantMap &changedProperties, const QStringList &invalidatedProperties);

private:
    QStringList m_addresses;
};

}
}
#endif // DEVICEMANAGERREALIZE_H
