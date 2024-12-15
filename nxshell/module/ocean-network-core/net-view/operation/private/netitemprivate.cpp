// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "netitemprivate.h"

#include "netmanager.h"

#include <QDebug>

namespace ocean {
namespace network {

#define GETFUN(RETTYPE, CLASS, FUNNAME, VALUE) \
    RETTYPE CLASS##Private::FUNNAME() const    \
    {                                          \
        return VALUE;                          \
    }
#define UPDATEFUN(CLASS, TYPE, NAME)                                      \
    void CLASS##Private::update##NAME(TYPE NAME)                          \
    {                                                                     \
        if (m_##NAME != NAME) {                                           \
            m_##NAME = NAME;                                              \
            Q_EMIT static_cast<CLASS *>(m_item)->NAME##Changed(m_##NAME); \
        }                                                                 \
    }

/**
 * 单个列表项的基类
 */
NetItemPrivate::NetItemPrivate()
    : m_item(nullptr)
{
}

NetItemPrivate::~NetItemPrivate()
{
    while (!m_children.isEmpty()) {
        NetItem *child = m_children.first();
        removeChild(child->dptr);
        delete child->dptr;
    }
    delete m_item;
    m_item = nullptr;
}

#define CaseNewItem(Type)                                           \
    case NetType::Type:                                             \
        netItemPrivate = new Net##Type##Private();                  \
        netItemPrivate->m_item = new Net##Type(netItemPrivate, id); \
        break

NetItemPrivate *NetItemPrivate::New(NetType::NetItemType type, const QString &id)
{
    NetItemPrivate *netItemPrivate = nullptr;
    switch (type) {
        CaseNewItem(WirelessControlItem);
        CaseNewItem(WirelessDeviceItem);
        CaseNewItem(WirelessItem);
        CaseNewItem(WirelessHioceannItem);
        CaseNewItem(WiredControlItem);
        CaseNewItem(WiredDeviceItem);
        CaseNewItem(WiredItem);
        CaseNewItem(ConnectionItem);
        CaseNewItem(Item);
        CaseNewItem(WirelessMineItem);
        CaseNewItem(WirelessOtherItem);
        CaseNewItem(VPNControlItem);
        CaseNewItem(HotspotControlItem);
        CaseNewItem(AirplaneControlItem);
        CaseNewItem(DSLControlItem);
        CaseNewItem(DetailsItem);
        CaseNewItem(DetailsInfoItem);
        CaseNewItem(SystemProxyControlItem);
        CaseNewItem(AppProxyControlItem);
    case NetType::WirelessDisabledItem:
    case NetType::AirplaneModeTipsItem:
    case NetType::WiredDisabledItem:
    case NetType::VPNTipsItem:
        break;
    }
    return netItemPrivate;
}

NetItem *NetItemPrivate::item()
{
    return m_item;
}

QString NetItemPrivate::id() const
{
    return m_item->objectName();
}

QString NetItemPrivate::name() const
{
    return m_name;
}

GETFUN(NetType::NetItemType, NetItem, itemType, NetType::NetItemType::Item)

NetItem *NetItemPrivate::getChild(int childPos) const
{
    return (childPos < getChildrenNumber() ? m_children[childPos] : nullptr);
}

int NetItemPrivate::getChildIndex(const NetItem *child) const
{
    int index = 0;
    for (auto it = m_children.cbegin(); it != m_children.cend(); it++, index++) {
        if (*it == child) {
            return index;
        }
    }

    return -1;
}

bool NetItemPrivate::addChild(NetItemPrivate *child, int index)
{
    if (!child || m_children.contains(child->item())) {
        return false;
    }
    if (index < 0 || index >= m_children.size())
        index = m_children.size();

    Q_EMIT m_item->childAboutToBeAoceand(m_item, index);
    m_children.insert(m_children.begin() + index, child->item());

    child->m_parent = m_item;
    Q_EMIT m_item->childAoceand(child->item());
    Q_EMIT m_item->childrenChanged();
    return true;
}

void NetItemPrivate::removeChild(NetItemPrivate *child)
{
    int index = 0;
    for (auto it = m_children.begin(); it != m_children.end(); it++, index++) {
        if (*it == child->item()) {
            Q_EMIT m_item->childAboutToBeRemoved(m_item, index);
            m_children.erase(it);
            child->m_parent = nullptr;
            Q_EMIT m_item->childRemoved(child->item());
            Q_EMIT m_item->childrenChanged();
            break;
        }
    }
}

// UPDATEFUN(NetItem, const QString &, name)
void NetItemPrivate::updatename(const QString &name)
{
    if (m_name != name) {
        m_name = name;
        Q_EMIT m_item->nameChanged(name);
    }
}

void NetItemPrivate::updateid(const QString &id)
{
    if (m_item->objectName() != id) {
        QString oldID = m_item->objectName();
        m_item->setObjectName(id);
        Q_EMIT m_item->idChanged(m_item->objectName(), oldID);
    }
}

void NetItemPrivate::emitDataChanged()
{
    Q_EMIT m_item->dataChanged();
}

// 总线控制器
GETFUN(bool, NetControlItem, isEnabled, m_enabled)
GETFUN(bool, NetControlItem, enabledable, m_enabledable)

UPDATEFUN(NetControlItem, const bool, enabled)
UPDATEFUN(NetControlItem, const bool, enabledable)

NetControlItemPrivate::NetControlItemPrivate()
    : NetItemPrivate()
    , m_enabled(false)
    , m_enabledable(true)
{
}

// 设备基类
GETFUN(NetType::NetDeviceStatus, NetDeviceItem, status, m_status)
GETFUN(QStringList, NetDeviceItem, ips, m_ips)
GETFUN(int, NetDeviceItem, pathIndex, m_pathIndex)
// GETFUN(QString, NetDeviceItem, path, m_path)
UPDATEFUN(NetDeviceItem, const NetType::NetDeviceStatus, status)
UPDATEFUN(NetDeviceItem, const QStringList &, ips)
UPDATEFUN(NetDeviceItem, const int &, pathIndex)

// UPDATEFUN(NetDeviceItem, const QString &, path)

NetDeviceItemPrivate::NetDeviceItemPrivate()
    : NetControlItemPrivate()
    , m_status(NetType::NetDeviceStatus::DS_Unknown)
    , m_pathIndex(0)
{
}

// 有线控制
GETFUN(NetType::NetItemType, NetWiredControlItem, itemType, NetType::NetItemType::WiredControlItem)

// 无线控制
GETFUN(NetType::NetItemType, NetWirelessControlItem, itemType, NetType::NetItemType::WirelessControlItem)

// 有线设备
GETFUN(NetType::NetItemType, NetWiredDeviceItem, itemType, NetType::NetItemType::WiredDeviceItem)
// 无线设备
GETFUN(NetType::NetItemType, NetWirelessDeviceItem, itemType, NetType::NetItemType::WirelessDeviceItem)
GETFUN(bool, NetWirelessDeviceItem, apMode, m_apMode)
UPDATEFUN(NetWirelessDeviceItem, bool, apMode)

NetWirelessDeviceItemPrivate::NetWirelessDeviceItemPrivate()
    : NetDeviceItemPrivate()
    , m_apMode(false)
{
}
// 连接基类
GETFUN(NetType::NetConnectionStatus, NetConnectionItem, status, m_status)
UPDATEFUN(NetConnectionItem, NetType::NetConnectionStatus, status)

NetConnectionItemPrivate::NetConnectionItemPrivate()
    : NetItemPrivate()
    , m_status(NetType::NetConnectionStatus::CS_UnConnected)
{
}
// 有线连接
GETFUN(NetType::NetItemType, NetWiredItem, itemType, NetType::NetItemType::WiredItem)
// 我的网络
GETFUN(NetType::NetItemType, NetWirelessMineItem, itemType, NetType::NetItemType::WirelessMineItem)
// 其他网络
GETFUN(NetType::NetItemType, NetWirelessOtherItem, itemType, NetType::NetItemType::WirelessOtherItem)
GETFUN(bool, NetWirelessOtherItem, isExpanded, m_expanded)
UPDATEFUN(NetWirelessOtherItem, bool, expanded)

NetWirelessOtherItemPrivate::NetWirelessOtherItemPrivate()
    : NetItemPrivate()
    , m_expanded(false)
{
}
// 隐藏网络
GETFUN(NetType::NetItemType, NetWirelessHioceannItem, itemType, NetType::NetItemType::WirelessHioceannItem)

// 无线网络
GETFUN(NetType::NetItemType, NetWirelessItem, itemType, NetType::NetItemType::WirelessItem)
GETFUN(uint, NetWirelessItem, flags, m_flags)
GETFUN(int, NetWirelessItem, strength, m_strength)
GETFUN(int, NetWirelessItem, strengthLevel, m_strengthLevel)
GETFUN(bool, NetWirelessItem, isSecure, m_secure)
GETFUN(bool, NetWirelessItem, hasConnection, m_hasConnection)
UPDATEFUN(NetWirelessItem, uint, flags)
// UPDATEFUN(NetWirelessItem, int, strength)
UPDATEFUN(NetWirelessItem, bool, secure)
UPDATEFUN(NetWirelessItem, bool, hasConnection)

void NetWirelessItemPrivate::updatestrength(int strength)
{
    if (strength != m_strength) {
        m_strength = strength;
        Q_EMIT static_cast<NetWirelessItem *>(m_item)->strengthChanged(m_strength);
        int level = NetManager::StrengthLevel(m_strength);
        if (m_strengthLevel != level) {
            m_strengthLevel = level;
            Q_EMIT static_cast<NetWirelessItem *>(m_item)->strengthLevelChanged(m_strengthLevel);
        }
    }
}

NetWirelessItemPrivate::NetWirelessItemPrivate()
    : NetConnectionItemPrivate()
    , m_flags(0)
    , m_strength(0)
    , m_strengthLevel(0)
    , m_secure(false)
{
}

// VPN
GETFUN(NetType::NetItemType, NetVPNControlItem, itemType, NetType::NetItemType::VPNControlItem)
GETFUN(bool, NetVPNControlItem, isExpanded, m_expanded)
UPDATEFUN(NetVPNControlItem, bool, expanded)

NetVPNControlItemPrivate::NetVPNControlItemPrivate()
    : NetDeviceItemPrivate()
    , m_expanded(false)
{
}
// 系统代理
GETFUN(NetType::NetItemType, NetSystemProxyControlItem, itemType, NetType::NetItemType::SystemProxyControlItem)
GETFUN(NetType::ProxyMethod, NetSystemProxyControlItem, method, m_method)
GETFUN(const QString &, NetSystemProxyControlItem, autoProxy, m_autoProxy)
GETFUN(const QVariantMap &, NetSystemProxyControlItem, manualProxy, m_manualProxy)
UPDATEFUN(NetSystemProxyControlItem, NetType::ProxyMethod, method)
UPDATEFUN(NetSystemProxyControlItem, const QString &, autoProxy)
UPDATEFUN(NetSystemProxyControlItem, const QVariantMap &, manualProxy)

// 应用代理
GETFUN(NetType::NetItemType, NetAppProxyControlItem, itemType, NetType::NetItemType::AppProxyControlItem)
GETFUN(const QVariantMap &, NetAppProxyControlItem, config, m_config)
UPDATEFUN(NetAppProxyControlItem, const QVariantMap &, config)

// DSL
GETFUN(NetType::NetItemType, NetDSLControlItem, itemType, NetType::NetItemType::DSLControlItem)
// Hotspot
GETFUN(NetType::NetItemType, NetHotspotControlItem, itemType, NetType::NetItemType::HotspotControlItem)
GETFUN(const QVariantMap &, NetHotspotControlItem, config, m_config)
GETFUN(const QStringList &, NetHotspotControlItem, shareDevice, m_shareDevice)
GETFUN(const QStringList &, NetHotspotControlItem, optionalDevice, m_optionalDevice)
UPDATEFUN(NetHotspotControlItem, const QVariantMap &, config)
UPDATEFUN(NetHotspotControlItem, const QStringList &, shareDevice)
UPDATEFUN(NetHotspotControlItem, const QStringList &, optionalDevice)
// Airplane
GETFUN(NetType::NetItemType, NetAirplaneControlItem, itemType, NetType::NetItemType::AirplaneControlItem)
// DetailsItem
GETFUN(NetType::NetItemType, NetDetailsItem, itemType, NetType::NetItemType::DetailsItem)
GETFUN(NetType::NetItemType, NetDetailsInfoItem, itemType, NetType::NetItemType::DetailsInfoItem)

GETFUN(const QList<QStringList> &, NetDetailsInfoItem, details, m_details)
GETFUN(const int &, NetDetailsInfoItem, index, m_index)
UPDATEFUN(NetDetailsInfoItem, const QList<QStringList> &, details)
UPDATEFUN(NetDetailsInfoItem, const int &, index)

} // namespace network
} // namespace ocean
