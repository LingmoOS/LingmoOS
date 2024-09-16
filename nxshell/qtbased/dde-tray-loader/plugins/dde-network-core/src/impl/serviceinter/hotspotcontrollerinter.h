// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef HOTSPOTCONTROLLERINTER_H
#define HOTSPOTCONTROLLERINTER_H

#include "networkconst.h"
#include "netutils.h"
#include "hotspotcontroller.h"

#include <QMap>
#include <QObject>

namespace dde {

namespace network {

class Connection;
class HotspotItem;
class WirelessDevice;
class NetworkDeviceBase;

class HotspotControllerInter : public HotspotController
{
    Q_OBJECT

    friend class NetworkInterProcesser;
    friend class NetworkManagerProcesser;

public:
    void setEnabled(WirelessDevice *device, const bool enable) override;              // 开启还是关闭个人热点
    bool enabled(WirelessDevice *device) override;                                    // 设备的热点是否可用
    bool supportHotspot() override;                                                   // 是否支持个人热点
    void connectItem(HotspotItem *item) override;                                     // 连接到个人热点
    void connectItem(WirelessDevice *device, const QString &uuid) override;           // 连接到个人热点
    void disconnectItem(WirelessDevice *device) override;                             // 断开连接
    QList<HotspotItem *> items(WirelessDevice *device) override;                      // 返回列表
    QList<WirelessDevice *> devices() override;                                       // 获取支持热点的设备列表

protected:
    explicit HotspotControllerInter(NetworkInter *networkInter, QObject *parent = Q_NULLPTR);
    ~HotspotControllerInter() override;

    void updateDevices(const QList<NetworkDeviceBase *> &devices);
    void updateConnections(const QJsonArray &jsons);
    HotspotItem *findItem(WirelessDevice *device, const QJsonObject &json);

    void updateActiveConnection(const QJsonObject &activeConnections);

    WirelessDevice *findDevice(const QString &path);
    bool isHotspotConnection(const QString &uuid);

private:
    QList<WirelessDevice *> m_devices;
    QList<HotspotItem *> m_hotspotItems;
    NetworkInter *m_networkInter;
};

}

}

#endif // UHOTSPOTCONTROLLER_H
