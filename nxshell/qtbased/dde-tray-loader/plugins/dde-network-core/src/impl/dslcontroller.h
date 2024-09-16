// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DSLCONTROLLER_H
#define DSLCONTROLLER_H

#include "networkconst.h"
#include "netutils.h"

#include <QMap>
#include <QObject>

namespace dde {

namespace network {

class DSLItem;
class NetworkDeviceBase;
class WiredDevice;

class DSLController : public QObject
{
    Q_OBJECT

public:
    virtual void connectItem(DSLItem *) {}                                      // 连接到当前的DSL
    virtual void connectItem(const QString &) {}                                // 根据UUID连接当前DSL
    virtual void disconnectItem(WiredDevice *) {}                               // 断开连接
    virtual QList<DSLItem *> items() const { return QList<DSLItem *>(); }       // 返回所有的DSL列表

Q_SIGNALS:
    void itemAdded(const QList<DSLItem *> &);                                   // 新增DSL项目
    void itemRemoved(const QList<DSLItem *> &);                                 // 移除DSL项目
    void itemChanged(const QList<DSLItem *> &);                                 // 项目发生变化（一般是ID发生了变化）
    void activeConnectionChanged();                                             // 连接状态发生变化

protected:
    explicit DSLController(QObject *parent = Q_NULLPTR);
    ~DSLController();
};

class DSLItem : public ControllItems
{
    Q_OBJECT

    friend class DSLController_Inter;
    friend class DSLController_NM;

protected:
    DSLItem();
    ~DSLItem();

    void setConnectionStatus(ConnectionStatus connectStatus);

public:
    ConnectionStatus status() const;                             // 当前连接的连接状态

private:
    ConnectionStatus m_connectStatus;
};

}

}

#endif // UDSLCONTROLLER_H
