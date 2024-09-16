// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DSLCONTROLLERINTER_H
#define DSLCONTROLLERINTER_H

#include "dslcontroller.h"

#include <QMap>
#include <QObject>

namespace dde {

namespace network {

class DSLItem;
class NetworkDeviceBase;

class DSLController_Inter : public DSLController
{
    Q_OBJECT

    friend class NetworkInterProcesser;

public:
    void connectItem(DSLItem *item) override;                                      // 连接到当前的DSL
    void connectItem(const QString &uuid) override;                                // 根据UUID连接当前DSL
    void disconnectItem(WiredDevice *device) override;                             // 断开连接
    inline QList<DSLItem *> items() const  override{ return m_items; }             // 返回所有的DSL列表

protected:
    explicit DSLController_Inter(NetworkInter *networkInter, QObject *parent = Q_NULLPTR);
    ~DSLController_Inter() override;

    void updateDevice(const QList<NetworkDeviceBase *> &devices);
    void updateDSLItems(const QJsonArray &dsljson);
    void updateActiveConnections(const QJsonObject &connectionJson);       // 更新DSL的活动连接信息

private:
    DSLItem *findItem(const QString &path);
    DSLItem *findDSLItemByUuid(const QString &uuid);

private:
    QList<DSLItem *> m_items;
    NetworkInter *m_networkInter;
    QMap<QString, QString> m_deviceInfo;
    QString m_activePath;
};

}

}

#endif // UDSLCONTROLLER_H
