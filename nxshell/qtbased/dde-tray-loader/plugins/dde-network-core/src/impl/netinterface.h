// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef NETINTERFACE_H
#define NETINTERFACE_H

#include "networkconst.h"

#include <QObject>
#include <QQueue>

namespace dde {
namespace network {

class NetworkDeviceBase;
class ProxyController;
class VPNController;
class DSLController;
class HotspotController;
class NetworkDetails;
class DSLItem;
class VPNItem;
class HotspotItem;
class WirelessDevice;
class AccessPoints;
class WiredDevice;
class WiredConnection;
class WirelessConnection;
class IPConflictChecker;

class NetworkProcesser : public QObject
{
    Q_OBJECT

public:
    virtual QList<NetworkDeviceBase *> devices() = 0;
    virtual Connectivity connectivity() = 0;
    virtual QList<NetworkDetails *> networkDetails();
    virtual ProxyController *proxyController();                                          // 返回代理控制管理器
    virtual VPNController *vpnController();                                              // 返回VPN控制器
    virtual DSLController *dslController();                                              // DSL控制器
    virtual HotspotController *hotspotController();                                      // 个人热点控制器
    virtual void retranslate();                                                          // 更新翻译

public slots:
    void onIpConflictChanged(const QString &devicePath, const QString &ip, bool conflicted);

protected:
    explicit NetworkProcesser(QObject *parent);
    virtual ~NetworkProcesser();

Q_SIGNALS:
    void deviceAdded(QList<NetworkDeviceBase *>);                                        // 新增设备发出的信号
    void deviceRemoved(QList<NetworkDeviceBase *>);                                      // 移除设备发出的信号
    void connectivityChanged(const Connectivity &);                                      // 网络连接状态发生变化的时候发出的信号
    void connectionChanged();                                                            // 连接信息发生变化的时候触发的信号
    void activeConnectionChange();                                                       // 网络数据发生变化

protected:
    void updateDeviceName();                                                             // 更新设备名称
};

class NetworkDeviceRealize : public QObject
{
    Q_OBJECT

    friend class NetworkDeviceBase;
    friend class NetworkInterProcesser;
    friend class NetworkManagerProcesser;

Q_SIGNALS:
    void deviceStatusChanged(const DeviceStatus &);                                      // 状态发生变化的时候触发的信号
    void enableChanged(const bool);                                                      // 开启设备或禁用设备的时候发出的信号(参数值True表示开启设备，False表示禁用设备)
    void connectionChanged();                                                            // 连接发生变化的时候触发的信号
    void nameChanged(const QString &);                                                   // 网卡名称发生变化的时候触发的信号
    void removed();                                                                      // 设备移除
    // 无线网络信号
    void networkAdded(const QList<AccessPoints *> &);                                    // wlan新增网络
    void networkRemoved(const QList<AccessPoints *> &);                                  // wlan列表减少网络
    void connectionFailed(const AccessPoints *);                                         // 连接无线wlan失败，第一个参数为失败的热点，第二个参数为对应的connection的Uuid
    void connectionSuccess(const AccessPoints *);                                        // 连接无线网络wlan成功，参数为对应的wlan
    void hotspotEnableChanged(const bool &);                                             // 热点是否可用发生变化
    void accessPointInfoChanged(const QList<AccessPoints *> &);                          // wlan信号强度发生变化的网络
    void activeConnectionChanged();                                                      // 活动连接发生变化的时候发出的信号
    void wirelessConnectionAdded(const QList<WirelessConnection *> &);                     // 新增连接
    void wirelessConnectionRemoved(const QList<WirelessConnection *> &);                   // 删除连接
    void wirelessConnectionPropertyChanged(const QList<WirelessConnection *> &);         // 连接属性发生变化
    // 有线设备的信号
    void connectionAdded(const QList<WiredConnection *> &);                              // 新增连接
    void connectionRemoved(const QList<WiredConnection *> &);                            // 删除连接
    void connectionPropertyChanged(const QList<WiredConnection *> &);                    // 连接属性发生变化
    void ipV4Changed();                                                                  // IPv4地址发生变化
    void availableChanged(bool);                                                         // 是否可用发生变化    
    void carrierChanged(bool carrier);                                                   // 是否插入网线状态发生了变化

public:
    void setDevice(NetworkDeviceBase *device);                                           // 设置当前的设备
    virtual bool isEnabled() const;                                                      // 当前的网卡是否启用
    virtual bool available() const;                                                      // 当前设备是否可用
    virtual bool IPValid();                                                              // IP是否合法
    virtual QString interface() const;                                                   // 返回设备上的Interface
    virtual QString driver() const;                                                      // 驱动，对应于备上返回值的Driver
    virtual bool managed() const;                                                        // 对应于设备上返回值的Managed
    virtual QString vendor() const;                                                      // 对应于设备上返回值的Vendor
    virtual QString uniqueUuid() const;                                                  // 网络设备的唯一的UUID，对应于设备上返回值的UniqueUuid
    virtual bool usbDevice() const;                                                      // 是否是USB设备，对应于设备上返回值的UsbDevice
    virtual QString path() const;                                                        // 设备路径，对应于设备上返回值的Path
    virtual QString activeAp() const;                                                    // 对应于设备上返回值的ActiveAp
    virtual bool supportHotspot() const;                                                 // 是否支持热点,对应于设备上返回值的SupportHotspot
    virtual QString realHwAdr() const;                                                   // mac地址
    virtual QString usingHwAdr() const;                                                  // 正在使用的mac地址
    virtual const QStringList ipv4();                                                    // IPV4地址
    virtual const QStringList ipv6();                                                    // IPV6地址
    virtual QJsonObject activeConnectionInfo() const;                                    // 获取当前活动连接的信息
    virtual void setEnabled(bool enabled);                                               // 开启或禁用网卡
    virtual void disconnectNetwork();                                                    // 断开网络连接，该方法是一个虚方法，具体在子类
    bool ipConflicted();
    virtual DeviceStatus deviceStatus() const;

    // 无线网络接口
    virtual QList<AccessPoints *> accessPointItems() const;                              // 当前网卡上所有的网络列表
    virtual void scanNetwork() {}                                                        // 重新加载所有的无线网络列表
    virtual void connectNetwork(const AccessPoints *item);                               // 连接网络，连接成功抛出deviceStatusChanged信号
    virtual QList<WirelessConnection *> wirelessItems() const;                           // 无线网络连接列表
    virtual AccessPoints *activeAccessPoints() const;                                    // 当前活动的无线连接
    virtual bool hotspotEnabled() { return false; }                                      // 无线网络是否开启热点

    // 有线网络接口
    virtual bool connectNetwork(WiredConnection *connection);                            // 连接网络，连接成功抛出deviceStatusChanged信号
    virtual QList<WiredConnection *> wiredItems() const;                                 // 有线网络连接列表
    virtual bool carrier() const;                                                        // 是否插入网线，true为插入网线，false为未插入网线
    void setIpConflict(bool ipConflicted);

protected:
    explicit NetworkDeviceRealize(QObject *parent);
    virtual ~NetworkDeviceRealize();

    NetworkDeviceBase *device() const;

    virtual QString statusStringDetail();
    virtual QString getStatusName();
    void enqueueStatus(const DeviceStatus &status);
    virtual void setDeviceStatus(const DeviceStatus &status);
    void sortWiredItem(QList<WiredConnection *> &items);
    bool ipConflicted() const;

private:
    NetworkDeviceBase *m_device;
    QQueue<DeviceStatus> m_statusQueue;
    DeviceStatus m_deviceStatus;
    bool m_ipConflicted;
};

class NetworkDetailRealize : public QObject
{
    Q_OBJECT

public:
    virtual QString name() = 0;
    virtual QList<QPair<QString, QString>> items() = 0;

signals:
    void infoChanged();

protected:
    explicit NetworkDetailRealize(QObject *parent) : QObject(parent) {}
    virtual ~NetworkDetailRealize() = default;
};

class AccessPointProxy : public QObject
{
    Q_OBJECT

    friend class AccessPoints;

protected:
    explicit AccessPointProxy(QObject *parent) : QObject(parent) {}
    virtual ~AccessPointProxy() = default;

public:
    virtual QString ssid() const = 0;                                           // 网络SSID，对应于返回接口中的Ssid
    virtual int strength() const = 0;                                           // 信号强度，对应于返回接口中的Strength
    virtual bool secured() const = 0;                                           // 是否加密，对应于返回接口中的Secured
    virtual bool securedInEap() const = 0;                                      // 对应于返回接口中的SecuredInEap
    virtual int frequency() const = 0;                                          // 频率，对应于返回接口中的Frequency
    virtual QString path() const = 0;                                           // 路径，对应于返回接口中的Path
    virtual QString devicePath() const = 0;                                     // 对应的设备的路径，为返回接口中的key值
    virtual bool connected() const = 0;                                         // 网络是否连接成功
    virtual ConnectionStatus status() const = 0;                                // 当前网络的连接状态
    virtual bool hidden() const = 0;                                            // 是否为隐藏网络
    virtual bool isWlan6() const = 0;

Q_SIGNALS:
    void strengthChanged(const int);                                            // 当前信号强度变化
    void connectionStatusChanged(ConnectionStatus);
    void securedChanged(bool);
};

}
}

#endif // INTERFACE_H
