// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef NETWORKMANAGERPROCESSER_H
#define NETWORKMANAGERPROCESSER_H

#include "netinterface.h"
#include "networkconst.h"
#include "netutils.h"

#include <networkmanagerqt/manager.h>

namespace dde {
namespace network {

class NetworkDeviceBase;
class VPNController_NM;
class HotspotController_NM;

class ProcesserInterface
{
public:
    virtual void deviceEnabledChanged() = 0;

protected:
    ProcesserInterface();
    virtual ~ProcesserInterface();
};

class NetworkManagerProcesser : public NetworkProcesser, public ProcesserInterface
{
    Q_OBJECT

    friend class DSLControl_Inter;

public:
    explicit NetworkManagerProcesser(bool sync, QObject *parent = Q_NULLPTR);
    ~NetworkManagerProcesser() override;

protected:
    QList<NetworkDeviceBase *> devices() override;
    dde::network::Connectivity connectivity() override;
    QList<NetworkDetails *> networkDetails() override;
    ProxyController *proxyController() override;                                          // 返回代理控制管理器
    VPNController *vpnController() override;                                              // 返回VPN控制器
    DSLController *dslController() override;                                              // DSL控制器
    HotspotController *hotspotController() override;                                      // 个人热点控制器
    void deviceEnabledChanged() override;                                                 // 设备开启和禁用的接口

private:
    void initConnections();
    void sortDevice();

private slots:
    void onDeviceAdded(const QString &uni);
    void onDeviceRemove(const QString &uni);
    void onConnectivityChanged(NetworkManager::Connectivity conntity);
    void onDevicesChanged(const QList<QDBusObjectPath> &devices);
    void checkConnectivityFinished(quint32 conntity);
    void onUpdateNetworkDetail();

private:
    QList<NetworkDeviceBase *> m_devices;
    ProxyController *m_proxyController;
    VPNController_NM *m_vpnController;
    DSLController *m_dslController;
    HotspotController_NM *m_hotspotController;
    dde::network::Connectivity m_connectivity;
    bool m_needDetails;
    QList<NetworkManager::Device::Ptr> m_deviceList;
};

}
}

#endif // NETWORKMANAGERPROCESSER_H
