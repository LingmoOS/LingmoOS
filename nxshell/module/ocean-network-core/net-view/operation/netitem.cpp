// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "netitem.h"

#include "private/netitemprivate.h"

#include <QDebug>

namespace ocean {
namespace network {

/**
 * 单个列表项的基类
 */
NetItem::NetItem(NetItemPrivate *p, const QString &id)
    : QObject()
    , dptr(p)
{
    setObjectName(id);
}

NetItem::~NetItem() { }

NetItem *NetItem::getChild(int childPos) const
{
    return dptr->getChild(childPos);
}

int NetItem::getChildIndex(const NetItem *child) const
{
    return dptr->getChildIndex(child);
}

#define GETFUN(RETTYPE, CLASS, FUNNAME)                        \
    RETTYPE CLASS::FUNNAME() const                             \
    {                                                          \
        return static_cast<CLASS##Private *>(dptr)->FUNNAME(); \
    }

GETFUN(int, NetItem, getChildrenNumber)
GETFUN(const QVector<NetItem *> &, NetItem, getChildren)
GETFUN(int, NetItem, getIndex)
GETFUN(NetItem *, NetItem, getParent)
GETFUN(NetType::NetItemType, NetItem, itemType)
GETFUN(QString, NetItem, name)
GETFUN(QString, NetItem, id)

// 总线控制器
GETFUN(bool, NetControlItem, isEnabled)
GETFUN(bool, NetControlItem, enabledable)

// 设备基类
// GETFUN(QString, NetDeviceItem, path)
GETFUN(NetType::NetDeviceStatus, NetDeviceItem, status)
GETFUN(QStringList, NetDeviceItem, ips)
GETFUN(int, NetDeviceItem, pathIndex)

// 有线设备

// 无线设备
GETFUN(bool, NetWirelessDeviceItem, apMode)

GETFUN(NetType::NetConnectionStatus, NetConnectionItem, status)

// 无线禁用项

// 有线禁用项

// 有线连接
// 我的网络
// 其他网络
GETFUN(bool, NetWirelessOtherItem, isExpanded)
// 隐藏网络

// 无线网络
GETFUN(uint, NetWirelessItem, flags)
GETFUN(int, NetWirelessItem, strength)
GETFUN(int, NetWirelessItem, strengthLevel)
GETFUN(bool, NetWirelessItem, isSecure)
GETFUN(bool, NetWirelessItem, hasConnection)

QString NetWiredControlItem::name() const
{
    return tr("Wired Network");
}

QString NetWirelessControlItem::name() const
{
    return tr("Wireless Network");
}

QString NetWirelessHioceannItem::name() const
{
    return tr("Connect to hioceann network");
}

// VPNTip
QString NetVPNTipsItem::name() const
{
    return tr("VPN configuration is not connected or failed to connect. Please <a style=\"text-decoration: none;\" href=\"go to the control center\">go to the control center</a> for inspection.");
}

// VPN

// 系统代理
GETFUN(NetType::ProxyMethod, NetSystemProxyControlItem, method)
GETFUN(const QString &, NetSystemProxyControlItem, autoProxy)
GETFUN(const QVariantMap &, NetSystemProxyControlItem, manualProxy)
// 应用代理
GETFUN(const QVariantMap &, NetAppProxyControlItem, config)
// Hotspot
GETFUN(const QVariantMap &, NetHotspotControlItem, config)
GETFUN(const QStringList &, NetHotspotControlItem, optionalDevice)
GETFUN(const QStringList &, NetHotspotControlItem, shareDevice)

// 飞行模式提示项
// DetailsItem
GETFUN(const QList<QStringList> &, NetDetailsInfoItem, details)
GETFUN(const int &, NetDetailsInfoItem, index)
} // namespace network
} // namespace ocean
