// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef VPNCONTROLLERINTER_H
#define VPNCONTROLLERINTER_H

#include "networkconst.h"
#include "netutils.h"
#include "vpncontroller.h"

#include <QObject>

namespace dde {

namespace network {

class VPNItem;

class VPNController_Inter : public VPNController
{
    Q_OBJECT

    friend class NetworkInterProcesser;

public:
    void setEnabled(const bool enabled) override;                                                         // 开启或者关闭VPN
    inline bool enabled() const override { return m_enabled; }                                            // VPN开启还是关闭
    void connectItem(VPNItem *item) override;                                                             // 连接VPN
    void connectItem(const QString &uuid) override;                                                       // 连接VPN(重载函数)
    void disconnectItem() override;                                                                       // 断开当前活动VPN连接
    QList<VPNItem *> items() const override { return m_vpnItems; }                                        // 获取所有的VPN列表

protected:
    explicit VPNController_Inter(NetworkInter *networkInter, QObject *parent = Q_NULLPTR);
    ~VPNController_Inter() override;

    void updateVPNItems(const QJsonArray &vpnArrays);
    void updateActiveConnection(const QJsonObject &activeConection);

private:
    VPNItem *findItem(const QString &path);
    VPNItem *findItemByUuid(const QString &uuid);

private Q_SLOTS:
    void onEnableChanged(const bool enabled);

private:
    NetworkInter *m_networkInter;
    bool m_enabled;
    QList<VPNItem *> m_vpnItems;
    QString m_activePath;
};

}

}

#endif // UVPNCONTROLLER_H
