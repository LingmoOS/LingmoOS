// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef VPNCONTROLLER_H
#define VPNCONTROLLER_H

#include "networkconst.h"
#include "netutils.h"

#include <QObject>

namespace dde {

namespace network {

class VPNItem;

class VPNController : public QObject
{
    Q_OBJECT

    friend class NetworkInterProcesser;
    friend class NetworkManagerProcesser;

public:
    virtual void setEnabled(const bool enabled) = 0;                                        // 开启或者关闭VPN
    virtual bool enabled() const = 0;                                                       // VPN开启还是关闭
    virtual void connectItem(VPNItem *item) = 0;                                            // 连接VPN
    virtual void connectItem(const QString &uuid) = 0;                                      // 连接VPN(重载函数)
    virtual void disconnectItem() = 0;                                                      // 断开当前活动VPN连接
    virtual QList<VPNItem *> items() const = 0;                                             // 获取所有的VPN列表

Q_SIGNALS:
    void enableChanged(const bool);                                                         // 开启关闭VPN发出的信号
    void itemAdded(const QList<VPNItem *> &);                                               // 新增VPN发出的信号
    void itemRemoved(const QList<VPNItem *> &);                                             // 移除VPN发出的信号
    void itemChanged(const QList<VPNItem *> &);                                             // VPN项发生变化（ID）
    void activeConnectionChanged();                                                         // 活动连接发生变化的时候发出的信号

protected:
    explicit VPNController(QObject *parent = Q_NULLPTR);
    ~VPNController();
};

class VPNItem : public ControllItems
{
    Q_OBJECT

    friend class VPNController_Inter;
    friend class VPNController_NM;

public:
    ConnectionStatus status() const;                                                        // 当前连接的连接状态

protected:
    VPNItem();
    ~VPNItem();

    void setConnectionStatus(ConnectionStatus status);

private:
    ConnectionStatus m_connectionStatus;
};

}

}

#endif // UVPNCONTROLLER_H
