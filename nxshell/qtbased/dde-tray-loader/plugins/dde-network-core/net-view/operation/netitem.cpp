// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "netitem.h"

#include "netmanager.h"

#include <QDebug>

namespace dde {
namespace network {

void NetItemRegisterMetaType()
{
    qRegisterMetaType<NetItemType>("NetItemType");
    qRegisterMetaType<NetConnectionStatus>("NetConnectionStatus");
    qRegisterMetaType<NetDeviceStatus>("NetDeviceStatus");
}

/**
 * 单个列表项的基类
 */
NetItem::NetItem(const QString &id)
    : QObject()
{
    setObjectName(id);
    QMetaObject::invokeMethod(this, &NetItem::retranslateUi, Qt::QueuedConnection);
}

NetItem::~NetItem() { }

QString NetItem::id() const
{
    return objectName();
}

NetItemType NetItem::itemType() const
{
    return NetItemType::RootItem;
}

QVariant NetItem::sortValue() const
{
    return QString::number(itemType(), 16) + name().toLower();
}

QString NetItem::name() const
{
    return m_name;
}

void NetItem::retranslateUi() { }

NetItem *NetItem::getChild(int childPos) const
{
    return (childPos < getChildrenNumber() ? m_children[childPos] : nullptr);
}

int NetItem::getChildIndex(const NetItem *child) const
{
    for (int i = 0; i < m_children.size(); ++i)
        if (m_children[i] == child)
            return static_cast<int>(i);

    return -1;
}

bool NetItem::addChild(NetItem *child, int index)
{
    if (!child || m_children.contains(child)) {
        return false;
    }
    if (index < 0 || index >= m_children.size())
        index = m_children.size();

    Q_EMIT childAboutToBeAdded(this, index);
    m_children.insert(m_children.begin() + index, child);

    child->setParent(this);
    Q_EMIT childAdded(child);
    return true;
}

void NetItem::removeChild(NetItem *child)
{
    int pos = getChildIndex(child);
    if (pos >= 0)
        removeChild(pos);
}

void NetItem::removeChild(int index)
{
    if (index < 0 || index >= m_children.size()) {
        return;
    }

    NetItem *child = m_children[index];
    Q_EMIT childAboutToBeRemoved(this, index);
    m_children.erase(m_children.begin() + index);
    child->setParent(nullptr);
    Q_EMIT childRemoved(child);
}

void NetItem::updateName(const QString &name)
{
    if (m_name != name) {
        m_name = name;
        Q_EMIT nameChanged(m_name);
        emitDataChanged();
    }
}

void NetItem::emitDataChanged()
{
    Q_EMIT dataChanged();
}

// 总线控制器
bool NetControlItem::isEnabled() const
{
    return m_enabled;
}

bool NetControlItem::enabledable() const
{
    return m_enabledable;
}

void NetControlItem::updateEnabled(const bool enabled)
{
    if (enabled != m_enabled) {
        m_enabled = enabled;
        Q_EMIT enabledChanged(m_enabled);
    }
}

void NetControlItem::updateEnabledable(const bool enabledable)
{
    if (enabledable != m_enabledable) {
        m_enabledable = enabledable;
        Q_EMIT enabledableChanged(m_enabledable);
    }
}

NetControlItem::NetControlItem(const QString &id)
    : NetItem(id)
    , m_enabled(false)
    , m_enabledable(true)
{
}

NetControlItem::~NetControlItem() { }

// 设备基类
NetDeviceStatus NetDeviceItem::status() const
{
    return m_status;
}

QStringList NetDeviceItem::ips() const
{
    return m_ips;
}

void NetDeviceItem::updateStatus(const NetDeviceStatus status)
{
    if (status != m_status) {
        m_status = status;
        Q_EMIT statusChanged(m_status);
    }
}

void NetDeviceItem::updateIps(const QStringList &ips)
{
    if (ips != m_ips) {
        m_ips = ips;
        Q_EMIT ipsChanged(m_ips);
    }
}

NetDeviceItem::NetDeviceItem(const QString &id)
    : NetControlItem(id)
    , m_status(NetDeviceStatus::Unknown)

{
}

NetDeviceItem::~NetDeviceItem() { }

// 有线设备
NetItemType NetWiredControlItem::itemType() const
{
    return NetItemType::WiredControlItem;
}

void NetWiredControlItem::retranslateUi()
{
    updateName(tr("Wired Network"));
}

NetWiredControlItem::NetWiredControlItem(const QString &id)
    : NetControlItem(id)
{
}

// 无线设备
NetItemType NetWirelessControlItem::itemType() const
{
    return NetItemType::WirelessControlItem;
}

void NetWirelessControlItem::retranslateUi()
{
    updateName(tr("Wireless Network"));
}

NetWirelessControlItem::NetWirelessControlItem(const QString &id)
    : NetControlItem(id)
{
}

NetTipsItem::NetTipsItem(const QString &id, const QString &linkActivatedText, const bool tipsLinkEnabled)
    : NetItem(id)
    , m_linkActivatedText(linkActivatedText)
    , m_tipsLinkEnabled(tipsLinkEnabled)
{
}

NetItemType NetTipsItem::itemType() const
{
    return NetItemType::RootItem;
}

// VPN
NetItemType NetVPNControlItem::itemType() const
{
    return NetItemType::VPNControlItem;
}

void NetVPNControlItem::retranslateUi()
{
    updateName(tr("VPN"));
}

void NetVPNControlItem::updateExpanded(bool isExpanded)
{
    if (m_isExpanded == isExpanded) {
        return;
    }
    m_isExpanded = isExpanded;
    Q_EMIT expandedChanged(m_isExpanded);
}

bool NetVPNControlItem::isExpanded() const
{
    return m_isExpanded;
}

NetVPNControlItem::NetVPNControlItem(const QString &id)
    : NetDeviceItem(id)
    , m_isExpanded(false)
{
}

NetVPNTipsItem::NetVPNTipsItem(const QString &id, const QString &linkActivatedText, const bool tipsLinkEnabled)
    : NetTipsItem(id, linkActivatedText, tipsLinkEnabled)
{
}

NetItemType NetVPNTipsItem::itemType() const
{
    return NetItemType::VPNTipsItem;
}

void NetVPNTipsItem::retranslateUi()
{
    updateName(tr("VPN configuration is not connected or failed to connect. Please <a style=\"text-decoration: none;\" href=\"go to the control center\">go to the control center</a> for inspection."));
}

// 系统代理
NetItemType NetSystemProxyControlItem::itemType() const
{
    return NetItemType::SystemProxyControlItem;
}

void NetSystemProxyControlItem::retranslateUi()
{
    updateName(tr("System Proxy"));
}

NetSystemProxyControlItem::NetSystemProxyControlItem(const QString &id)
    : NetControlItem(id)
{
}

NetItemType NetWiredDeviceItem::itemType() const
{
    return NetItemType::WiredDeviceItem;
}

NetItemType NetWirelessDeviceItem::itemType() const
{
    return NetItemType::WirelessDeviceItem;
}

bool NetWirelessDeviceItem::isApMode() const
{
    return m_isApMode;
}

void NetWirelessDeviceItem::updateApMode(bool apMode)
{
    if (apMode != m_isApMode) {
        m_isApMode = apMode;
        Q_EMIT apModeChanged(m_isApMode);
    }
}

NetWirelessDeviceItem::NetWirelessDeviceItem(const QString &id)
    : NetDeviceItem(id)
    , m_isApMode(false)
{
}

NetItemType NetWirelessOtherItem::itemType() const
{
    return NetItemType::WirelessOtherItem;
}

void NetWirelessOtherItem::retranslateUi()
{
    updateName(tr("Other Networks"));
}

bool NetWirelessOtherItem::isExpanded() const
{
    return m_isExpanded;
}

void NetWirelessOtherItem::updateExpanded(bool isExpanded)
{
    if (m_isExpanded == isExpanded) {
        return;
    }
    m_isExpanded = isExpanded;
    Q_EMIT expandedChanged(m_isExpanded);
}

NetWirelessOtherItem::NetWirelessOtherItem(const QString &id)
    : NetItem(id)
    , m_isExpanded(false)
{
}

NetItemType NetWirelessMineItem::itemType() const
{
    return NetItemType::WirelessMineItem;
}

void NetWirelessMineItem::retranslateUi()
{
    updateName(tr("My Networks"));
}

NetWirelessMineItem::NetWirelessMineItem(const QString &id)
    : NetItem(id)
{
}

// 无线禁用项
NetItemType NetWirelessDisabledItem::itemType() const
{
    return NetItemType::WirelessDisabledItem;
}

void NetWirelessDisabledItem::retranslateUi()
{
    updateName(tr("Wireless network off"));
}

// 飞行模式提示项
NetAirplaneModeTipsItem::NetAirplaneModeTipsItem(const QString &id, const QString &linkActivatedText, const bool tipsLinkEnabled)
    : NetTipsItem(id, linkActivatedText, tipsLinkEnabled)
{
}

NetItemType NetAirplaneModeTipsItem::itemType() const
{
    return NetItemType::AirplaneModeTipsItem;
}

void NetAirplaneModeTipsItem::retranslateUi()
{
    //updateName(tr("Disable <a style=\"text-decoration: none;\" href=\"Airplane Mode\">Airplane Mode</a> first if you want to connect to a wireless network"));
    // v23控制中心无“飞行模式”模块，因此不能跳转到控制中心模块
    updateName(tr("Disable Airplane Mode first if you want to connect to a wireless network"));
}

// 有线禁用项
NetItemType NetWiredDisabledItem::itemType() const
{
    return NetItemType::WiredDisabledItem;
}

void NetWiredDisabledItem::retranslateUi()
{
    updateName(tr("Wired network off"));
}

// 有线连接
NetItemType NetWiredItem::itemType() const
{
    return NetItemType::WiredItem;
}

NetConnectionStatus NetWiredItem::status() const
{
    return m_status;
}

void NetWiredItem::updateStatus(NetConnectionStatus status)
{
    if (status != m_status) {
        m_status = status;
        Q_EMIT statusChanged(m_status);
    }
}

NetWiredItem::NetWiredItem(const QString &id)
    : NetItem(id)
    , m_status(NetConnectionStatus::UnConnected)
{
}

NetWiredItem::~NetWiredItem() { }

// 隐藏网络
NetItemType NetWirelessHiddenItem::itemType() const
{
    return NetItemType::WirelessHiddenItem;
}

QVariant NetWirelessHiddenItem::sortValue() const
{
    return "ff";
}

void NetWirelessHiddenItem::retranslateUi()
{
    updateName(tr("Connect to hidden network"));
}

NetWirelessHiddenItem::NetWirelessHiddenItem(const QString &id)
    : NetItem(id)
{
}

NetWirelessHiddenItem::~NetWirelessHiddenItem() { }

// 无线网络
NetItemType NetWirelessItem::itemType() const
{
    return NetItemType::WirelessItem;
}

QVariant NetWirelessItem::sortValue() const
{
    return QString::number((m_status == NetConnectionStatus::Connected ? 140 : 250) - m_strengthLevel, 16) + name().toLower();
}

uint NetWirelessItem::flags() const
{
    return m_flags;
}

int NetWirelessItem::strength() const
{
    return m_strength;
}

int NetWirelessItem::strengthLevel() const
{
    return m_strengthLevel;
}

bool NetWirelessItem::isSecure() const
{
    return m_secure;
}

NetConnectionStatus NetWirelessItem::status() const
{
    return m_status;
}

bool NetWirelessItem::hasConnection() const
{
    return m_hasConnection;
}

void NetWirelessItem::updateFlags(uint flags)
{
    if (flags != m_flags) {
        m_flags = flags;
        Q_EMIT flagsChanged(m_flags);
    }
}

void NetWirelessItem::updateStrength(int strength)
{
    if (strength != m_strength) {
        m_strength = strength;
        Q_EMIT strengthChanged(m_strength);
        int level = NetManager::StrengthLevel(m_strength);
        if (m_strengthLevel != level) {
            m_strengthLevel = level;
            Q_EMIT strengthLevelChanged(m_strengthLevel);
            emitDataChanged();
        }
    }
}

void NetWirelessItem::updateSecure(bool secure)
{
    if (secure != m_secure) {
        m_secure = secure;
        Q_EMIT secureChanged(m_secure);
    }
}

void NetWirelessItem::updateStatus(NetConnectionStatus status)
{
    if (status != m_status) {
        m_status = status;
        Q_EMIT statusChanged(m_status);
        emitDataChanged();
    }
}

void NetWirelessItem::updateHasConnection(bool hasConnection)
{
    if (hasConnection != m_hasConnection) {
        m_hasConnection = hasConnection;
        Q_EMIT hasConnectionChanged(m_hasConnection);
    }
}

NetWirelessItem::NetWirelessItem(const QString &id)
    : NetItem(id)
    , m_flags(0)
    , m_strength(0)
    , m_strengthLevel(0)
    , m_secure(false)
    , m_status(NetConnectionStatus::UnConnected)
{
}

NetWirelessItem::~NetWirelessItem() { }

} // namespace network
} // namespace dde
