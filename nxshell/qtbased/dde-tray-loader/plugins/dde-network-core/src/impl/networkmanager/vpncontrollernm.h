// SPDX-FileCopyrightText: 2018 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef VPNCONTROLLER_NM_H
#define VPNCONTROLLER_NM_H

#include "vpncontroller.h"

#include <NetworkManagerQt/Connection>

namespace dde {
namespace network {

class VPNController_NM : public VPNController
{
    Q_OBJECT

    friend class NetworkManagerProcesser;

public:
    void setEnabled(const bool enabled) override;                                        // 开启或者关闭VPN
    bool enabled() const override;                                                       // VPN开启还是关闭
    void connectItem(VPNItem *item) override;                                            // 连接VPN
    void connectItem(const QString &uuid) override;                                      // 连接VPN(重载函数)
    void disconnectItem() override;                                                      // 断开当前活动VPN连接
    QList<VPNItem *> items() const override;                                             // 返回所有的连接

protected:
    explicit VPNController_NM(QObject *parent);
    ~VPNController_NM() override;

private:
    void initMember();
    void initConnections();
    VPNItem *addVpnConnection(const NetworkManager::Connection::Ptr &connection);
    void sortVPNItems();
    QList<VPNItem *> findAutoConnectItems() const;
    NetworkManager::ActiveConnection::List findActiveConnection() const;
    NetworkManager::Connection::Ptr findConnectionByVPNItem(VPNItem *vpnItem) const;     // 根据VPN的条目查找具体的Connection;

private Q_SLOTS:
    void onConnectionAdded(const QString &path);
    void onConnectionRemoved(const QString &path);
    void onActiveConnectionsChanged();
    void onPropertiesChanged(const QString &interfaceName, const QVariantMap &changedProperties);

private:
    QList<VPNItem *> m_items;
    QMap<VPNItem *, NetworkManager::Connection::Ptr> m_vpnConnectionsMap;
};

}
}

#endif // VPNCONTROLLER_NM_H
