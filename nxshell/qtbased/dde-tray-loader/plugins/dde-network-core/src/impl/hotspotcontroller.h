// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef HOTSPOTCONTROLLER_H
#define HOTSPOTCONTROLLER_H

#include "networkconst.h"
#include "netutils.h"

#include <QMap>
#include <QObject>

namespace dde {

namespace network {

class Connection;
class HotspotItem;
class WirelessDevice;
class NetworkDeviceBase;

class HotspotController : public QObject
{
    Q_OBJECT

public:
    virtual void setEnabled(WirelessDevice *device, const bool enable) = 0;              // 开启还是关闭个人热点
    virtual bool enabled(WirelessDevice *device) = 0;                                    // 设备的热点是否可用
    virtual bool supportHotspot() = 0;                                                   // 是否支持个人热点
    virtual void connectItem(HotspotItem *item) = 0;                                     // 连接到个人热点
    virtual void connectItem(WirelessDevice *device, const QString &uuid) = 0;           // 连接到个人热点
    virtual void disconnectItem(WirelessDevice *device) = 0;                             // 断开连接
    virtual QList<HotspotItem *> items(WirelessDevice *device) = 0;                      // 返回列表
    virtual QList<WirelessDevice *> devices() = 0;                                       // 获取支持热点的设备列表

Q_SIGNALS:
    void enabledChanged(const bool &);                                                   // 热点是否可用发生了变化
    void itemAdded(const QMap<WirelessDevice *, QList<HotspotItem *>> &);                // 新增连接的信号
    void itemRemoved(const QMap<WirelessDevice *, QList<HotspotItem *>> &);              // 删除连接的信号
    void itemChanged(const QMap<WirelessDevice *, QList<HotspotItem *>> &);              // 连接信息改变的信号
    void activeConnectionChanged(const QList<WirelessDevice *> &);                       // 活动连接发生变化
    void deviceAdded(const QList<WirelessDevice *> &);                                   // 新增热点设备
    void deviceRemove(const QList<WirelessDevice *> &);                                  // 删除热点设备
    void enableHotspotSwitch(const bool &);                                              // 使能热点开关

protected:
    explicit HotspotController(QObject *parent = Q_NULLPTR);
    virtual ~HotspotController();
};

class HotspotItem : public ControllItems
{
    Q_OBJECT

    friend class HotspotControllerInter;
    friend class HotspotController_NM;

public:
    QString name() const;                                                               // 个人热点名称
    WirelessDevice *device() const;                                                     // 当前热点对应的无线设备
    ConnectionStatus status() const;                                                    // 当前连接的连接状态

protected:
    explicit HotspotItem(WirelessDevice *device);
    ~HotspotItem();

    QString devicePath() const;                                                         // 返回设备的路径
    void setConnectionStatus(const ConnectionStatus &status);

private:
    WirelessDevice *m_device;
    QString m_devicePath;
    ConnectionStatus m_connectionStatus;
};

}

}

#endif // UHOTSPOTCONTROLLER_H
