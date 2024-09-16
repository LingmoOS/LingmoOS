// SPDX-FileCopyrightText: 2018 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DSLCONTROLLERNM_H
#define DSLCONTROLLERNM_H

#include "dslcontroller.h"

#include <NetworkManagerQt/Connection>
#include <NetworkManagerQt/WiredDevice>

namespace dde {
namespace network {

class DSLController_NM : public DSLController
{
    Q_OBJECT

    friend class NetworkManagerProcesser;

public:
    void connectItem(DSLItem *item) override;                                      // 连接到当前的DSL
    void connectItem(const QString &uuid) override;                                // 根据UUID连接当前DSL
    void disconnectItem(WiredDevice *device) override;                             // 断开连接
    QList<DSLItem *> items() const override;                                       // 返回所有的DSL列表

protected:
    explicit DSLController_NM(QObject *parent);
    ~DSLController_NM() override;

private:
    void initMember();
    NetworkManager::Connection::List getConnections(const NetworkManager::WiredDevice::Ptr &device);
    void initConnection();
    void initDeviceConnection(const NetworkManager::WiredDevice::Ptr &wiredDevice);
    void connectionAppeared(const NetworkManager::WiredDevice::Ptr &wiredDevice, NetworkManager::Connection::List connections, const QString &uni);
    DSLItem *addPppoeConnection(NetworkManager::WiredDevice::Ptr device, NetworkManager::Connection::Ptr connection);
    void updateActiveConnectionInfo(NetworkManager::WiredDevice *wiredDevice);

private Q_SLOTS:
    void onActiveConnectionChanged();
    void onDeviceAdded(const QString &uni);
    void onDeviceRemoved(const QString &uni);

private:
    NetworkManager::WiredDevice::List m_devices;
    QList<DSLItem *> m_dslItems;
};

}
}

#endif // DSLCONTROLLERNM_H
