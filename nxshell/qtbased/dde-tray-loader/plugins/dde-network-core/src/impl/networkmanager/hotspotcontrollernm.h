// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef HOTSPOTCONTROLLERNM_H
#define HOTSPOTCONTROLLERNM_H

#include "networkconst.h"
#include "netutils.h"
#include "hotspotcontroller.h"

#include <QMap>
#include <QObject>

namespace NetworkManager {
    class WirelessDevice;
}

namespace dde {

namespace network {

class Connection;
class HotspotItem;
class WirelessDevice;
class NetworkDeviceBase;

class HotspotController_NM : public HotspotController
{
    Q_OBJECT

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
    explicit HotspotController_NM(QObject *parent = Q_NULLPTR);
    virtual ~HotspotController_NM() override;

    void updateDevices(const QList<NetworkDeviceBase *> &devices);

private:
    QSharedPointer<NetworkManager::WirelessDevice> findWirelessDevice(WirelessDevice *wirelessDevice) const;
    HotspotItem *addConnection(WirelessDevice *device, NetworkManager::Connection::Ptr connection);
    void onActiveConnectionChanged(WirelessDevice *device, NetworkManager::ActiveConnection::Ptr activeConnection);
    void sortItem();

private slots:
    void onConnectionAdded(const QString &connectionUni);
    void onConnectionRemoved(const QString &connectionUni);

private:
    QList<WirelessDevice *> m_devices;
    QMap<WirelessDevice *, QList<HotspotItem *>> m_deviceHotpots;
};

}

}

#endif // UHOTSPOTCONTROLLER_H
