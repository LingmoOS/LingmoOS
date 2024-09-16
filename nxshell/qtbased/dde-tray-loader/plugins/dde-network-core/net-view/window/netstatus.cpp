// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
#include "netstatus.h"

#include "netitem.h"
#include "netmanager.h"
#include "networkconst.h"

#include <QDebug>
#include <QEvent>
#include <QJsonDocument>
#include <QTimer>
#include <QGuiApplication>

#define WIRELESS_DEVICE_INDEX 0
#define WIRED_DEVICE_INDEX 1
#define DEVICE_ITEM_COUNT 2

#define HAS_WIRELESS_DEVICE 0x10u
#define HAS_WIRED_DEVICE 0x20u
#define HAS_DEVICE_MASK 0x30u
#define ENABLED_WIRELESS_DEVICE 0x01u
#define ENABLED_WIRED_DEVICE 0x02u
#define ENABLED_DEVICE_MASK 0x03u
#define ENABLEDABLE_WIRELESS_DEVICE 0x04u
#define ENABLEDABLE_WIRED_DEVICE 0x08u
#define ENABLEDABLE_DEVICE_MASK 0x0Cu

namespace dde {
namespace network {

enum MenuItemKey : int {
    MenuSettings = 1,
    MenuEnabled,
    MenuDisabled,
    MenuWiredEnabled,
    MenuWiredDisabled,
    MenuWirelessEnabled,
    MenuWirelessDisabled,
    MenuVPNEnabled,
    MenuVPNDisabled,
    MenuProxyEnabled,
    MenuProxyDisabled,
};

NetStatus::NetStatus(NetManager *manager, QObject *parent)
    : QObject(parent)
    , m_manager(manager)
    , m_networkStatus(NetworkStatus::Unknown)
    , m_hasDevice(false)
    , m_deviceFlag(0)
    , m_vpnItem({"NetVPNControlItem", false, false, false})
    , m_proxyItem({"NetSystemProxyControlItem", false, false, false})
    , m_currentAnimation(0)
    , m_animationTimer(new QTimer(this))
    , m_quickCurrentAnimation(0)
    , m_quickAnimationTimer(nullptr)
    , m_statusTimer(new QTimer(this))
    , m_networkBut(nullptr)
    , m_vpnAndProxyBut(nullptr)
    , m_tipsLabel(nullptr)
    , m_dockIconWidgetlayout(nullptr)
    , m_vpnAndProxyIconVisibel(true)
{
    NetItem *root = m_manager->root();
    connect(root, &NetItem::childRemoved, this, &NetStatus::onChildRemoved);

    connect(this, &NetStatus::networkStatusChanged, this, &NetStatus::updateNetworkTips);
    connect(this, &NetStatus::networkStatusChanged, this, &NetStatus::updateNetworkIcon);
    connect(m_manager, &NetManager::languageChange, this, &NetStatus::updateNetworkTips);
    connect(m_manager, &NetManager::primaryConnectionTypeChanged, this, &NetStatus::updateNetworkIcon);
    connect(m_statusTimer, &QTimer::timeout, this, &NetStatus::doUpdateStatus);
    m_statusTimer->setInterval(100);
    m_statusTimer->setSingleShot(true);
    m_statusTimer->setTimerType(Qt::TimerType::CoarseTimer);

    m_animationTimer->setInterval(250);
    connect(m_animationTimer, &QTimer::timeout, this, &NetStatus::nextAnimation);
    onChildAdded(root);
}

NetStatus::~NetStatus() { }

void NetStatus::invokeMenuItem(const QString &menuId)
{
    switch (menuId.toInt()) {
    case MenuItemKey::MenuEnabled:
        m_manager->setWiredEnabled(true);
        m_manager->setWirelessEnabled(true);
        break;
    case MenuItemKey::MenuDisabled:
        m_manager->setWiredEnabled(false);
        m_manager->setWirelessEnabled(false);
        break;
    case MenuItemKey::MenuWiredEnabled:
        m_manager->setWiredEnabled(true);
        break;
    case MenuItemKey::MenuWiredDisabled:
        m_manager->setWiredEnabled(false);
        break;
    case MenuItemKey::MenuWirelessEnabled:
        m_manager->setWirelessEnabled(true);
        break;
    case MenuItemKey::MenuWirelessDisabled:
        m_manager->setWirelessEnabled(false);
        break;
    case MenuItemKey::MenuVPNEnabled:
        m_manager->setVPNEnabled(true);
        break;
    case MenuItemKey::MenuVPNDisabled:
        m_manager->setVPNEnabled(false);
        break;
    case MenuItemKey::MenuProxyEnabled:
        m_manager->setProxyEnabled(true);
        break;
    case MenuItemKey::MenuProxyDisabled:
        m_manager->setProxyEnabled(false);
        break;
    case MenuItemKey::MenuSettings:
        m_manager->exec(NetManager::GoToControlCenter, "");
        break;
    default:
        break;
    }
}

bool NetStatus::needShowControlCenter() const
{
    // 如果没有网络设备，则直接唤起控制中心
    if (!(m_deviceFlag & HAS_DEVICE_MASK))
        return true;
    if ((m_deviceFlag & HAS_DEVICE_MASK) == HAS_WIRED_DEVICE && getDeviceConnections(NET_WIRED, static_cast<unsigned>(NetConnectionStatus::AllConnectionStatus)).isEmpty()) // 只有有线,且无配置
        return true;
    return false;
}

const QString NetStatus::contextMenu(bool hasSetting) const
{
    QList<QVariant> items;
    if ((m_deviceFlag & HAS_DEVICE_MASK) == HAS_DEVICE_MASK) { // 无线有线都有
        QMap<QString, QVariant> wireEnable;
        if (m_deviceFlag & ENABLED_WIRED_DEVICE) {
            wireEnable["itemId"] = QString::number(MenuWiredDisabled);
            wireEnable["itemText"] = tr("Disable wired connection");
        } else {
            wireEnable["itemId"] = QString::number(MenuWiredEnabled);
            wireEnable["itemText"] = tr("Enable wired connection");
        }
        wireEnable["isActive"] = bool(m_deviceFlag & ENABLEDABLE_WIRED_DEVICE);
        items.push_back(wireEnable);
        QMap<QString, QVariant> wirelessEnable;
        if (m_deviceFlag & ENABLED_WIRELESS_DEVICE) {
            wirelessEnable["itemText"] = tr("Disable wireless connection");
            wirelessEnable["itemId"] = QString::number(MenuWirelessDisabled);
        } else {
            wirelessEnable["itemText"] = tr("Enable wireless connection");
            wirelessEnable["itemId"] = QString::number(MenuWirelessEnabled);
        }

        wirelessEnable["isActive"] = bool(m_deviceFlag & ENABLEDABLE_WIRELESS_DEVICE);
        items.push_back(wirelessEnable);
    } else if (m_deviceFlag & HAS_DEVICE_MASK) { // 只有一种设备

        QMap<QString, QVariant> enable;
        if (m_deviceFlag & ENABLED_DEVICE_MASK) {
            enable["itemId"] = QString::number(MenuDisabled);
            enable["itemText"] = tr("Disable network");
        } else {
            enable["itemId"] = QString::number(MenuEnabled);
            enable["itemText"] = tr("Enable network");
        }

        enable["isActive"] = bool(m_deviceFlag & ENABLEDABLE_DEVICE_MASK);
        items.push_back(enable);
    }
    if (!m_manager->isGreeterMode()) {
        if (m_vpnItem.Availabled) {
            QMap<QString, QVariant> vpnEnable;
            if (m_vpnItem.Enabled) {
                vpnEnable["itemId"] = QString::number(MenuVPNDisabled);
                vpnEnable["itemText"] = tr("Disable VPN");
            } else {
                vpnEnable["itemId"] = QString::number(MenuVPNEnabled);
                vpnEnable["itemText"] = tr("Enable VPN");
            }

            vpnEnable["isActive"] = m_vpnItem.Availabled;
            items.push_back(vpnEnable);
        }
        if (m_proxyItem.Availabled) {
            QMap<QString, QVariant> proxyEnable;
            if (m_proxyItem.Enabled) {
                proxyEnable["itemId"] = QString::number(MenuProxyDisabled);
                proxyEnable["itemText"] = tr("Disable system proxy");
            } else {
                proxyEnable["itemId"] = QString::number(MenuProxyEnabled);
                proxyEnable["itemText"] = tr("Enable system proxy");
            }

            proxyEnable["isActive"] = m_proxyItem.Availabled;
            items.push_back(proxyEnable);
        }
    }
    if (hasSetting) {
        QMap<QString, QVariant> settings;
        settings["itemId"] = QString::number(MenuSettings);
        settings["itemText"] = tr("Network settings");
        settings["isActive"] = true;
        items.push_back(settings);
    }
    QMap<QString, QVariant> menu;
    menu["items"] = items;
    menu["checkableMenu"] = false;
    menu["singleCheck"] = false;

    return QJsonDocument::fromVariant(menu).toJson();
}

QWidget *NetStatus::createItemTips() const
{
    QLabel *label = new QLabel(m_networkTips);
    connect(this, &NetStatus::networkTipsChanged, label, &QLabel::setText);
    connect(this, &NetStatus::networkTipsChanged, label, &QLabel::adjustSize, Qt::QueuedConnection);
    connect(qApp, &QGuiApplication::fontChanged, label, &QLabel::adjustSize, Qt::QueuedConnection);
    label->setContentsMargins(10, 0, 10, 0);
    label->setForegroundRole(QPalette::BrightText);
    return label;
}

QWidget *NetStatus::createDockItemTips()
{
    m_tipsLabel = new QLabel(m_networkTips);
    connect(this, &NetStatus::networkTipsChanged, this, [this] {
        if (m_hoverType == HoverType::Network)
            setHoverTips(HoverType::Network);
    });
    connect(this, &NetStatus::vpnAndProxyTipsChanged, this, [this] {
        if (m_hoverType == HoverType::vpnAndProxy)
            setHoverTips(HoverType::vpnAndProxy);
    });
    connect(qApp, &QGuiApplication::fontChanged, m_tipsLabel, &QLabel::adjustSize, Qt::QueuedConnection);
    m_tipsLabel->setContentsMargins(0, 0, 0, 0);
    m_tipsLabel->setForegroundRole(QPalette::BrightText);
    setHoverTips(m_hoverType);
    return m_tipsLabel;
}

QWidget *NetStatus::createIconWidget() const
{
    NetIconButton *iconBut = new NetIconButton();
    iconBut->setForegroundRole(QPalette::BrightText);
    iconBut->setIcon(m_networkIcon);
    iconBut->setFixedSize(20, 20);
    connect(this, &NetStatus::networkIconChanged, iconBut, &NetIconButton::setIcon);
    return iconBut;
}

QWidget *NetStatus::createDockIconWidget()
{
    QWidget *contentWidget = new QWidget;
    contentWidget->setMouseTracking(true);
    m_dockIconWidgetlayout = new QBoxLayout(QBoxLayout::LeftToRight, contentWidget);
    m_dockIconWidgetlayout->setMargin(0);
    m_dockIconWidgetlayout->setSpacing(10);
    m_networkBut = new NetIconButton(contentWidget);
    m_networkBut->setForegroundRole(QPalette::BrightText);
    m_networkBut->setIcon(m_networkIcon);
    m_networkBut->setFixedSize(16, 16);
    connect(this, &NetStatus::networkIconChanged, m_networkBut, &NetIconButton::setIcon);

    m_vpnAndProxyBut = new NetIconButton(contentWidget);
    m_vpnAndProxyBut->setForegroundRole(QPalette::BrightText);
    m_vpnAndProxyBut->setIcon(m_vpnAndProxyIcon);
    m_vpnAndProxyBut->setFixedSize(16, 16);
    connect(this, &NetStatus::vpnAndProxyIconChanged, m_vpnAndProxyBut, &NetIconButton::setIcon);

    m_dockIconWidgetlayout->addWidget(m_networkBut);
    m_dockIconWidgetlayout->addWidget(m_vpnAndProxyBut);
    updateVpnAndProxyStatus();
    return contentWidget;
}

void NetStatus::setDirection(QBoxLayout::Direction position)
{
    if (m_dockIconWidgetlayout) {
        m_dockIconWidgetlayout->setDirection(position);
        updateItemWidgetSize();
    }
}

void NetStatus::refreshIcon()
{
    updateStatus();
}

bool NetStatus::hasDevice() const
{
    return m_hasDevice;
}

QString NetStatus::networkTips() const
{
    return m_networkTips;
}

void NetStatus::setHoverTips(HoverType type)
{
    m_hoverType = type;
    if (!m_tipsLabel)
        return;
    switch (type) {
    case HoverType::Network: {
        m_tipsLabel->setText(m_networkTips);
    } break;
    case HoverType::vpnAndProxy: {
        m_tipsLabel->setText(m_vpnAndProxyTips);
    } break;
    default:
        break;
    }
    m_tipsLabel->adjustSize();
}

NetStatus::HoverType NetStatus::hoverType() const
{
    return m_hoverType;
}

QString NetStatus::vpnAndProxyTips() const
{
    return m_vpnAndProxyTips;
}

QIcon NetStatus::vpnAndProxyIcon() const
{
    return m_vpnAndProxyIcon;
}

QIcon NetStatus::networkIcon() const
{
    return m_networkIcon;
}

void NetStatus::initQuickData()
{
    if (m_quickAnimationTimer)
        return;
    m_quickAnimationTimer = new QTimer(this);
    m_quickAnimationTimer->setInterval(250);
    connect(m_quickAnimationTimer, &QTimer::timeout, this, &NetStatus::nextQuickAnimation);
}

bool NetStatus::networkActive() const
{
    return m_networkActive;
}

QString NetStatus::quickTitle() const
{
    return m_quickTitle;
}

QString NetStatus::quickDescription() const
{
    return m_quickDescription;
}

QIcon NetStatus::quickIcon() const
{
    return m_quickIcon;
}

void NetStatus::toggleNetworkActive()
{
    if (m_deviceFlag & HAS_WIRELESS_DEVICE) {
        if (m_deviceFlag & ENABLED_WIRELESS_DEVICE) {
            invokeMenuItem(QString::number(MenuItemKey::MenuWirelessDisabled));
        } else {
            invokeMenuItem(QString::number(MenuItemKey::MenuWirelessEnabled));
        }
    } else if (m_deviceFlag & HAS_WIRED_DEVICE) {
        if (m_deviceFlag & ENABLED_WIRED_DEVICE) {
            invokeMenuItem(QString::number(MenuItemKey::MenuWiredDisabled));
        } else {
            invokeMenuItem(QString::number(MenuItemKey::MenuWiredEnabled));
        }
    }
}

void NetStatus::onChildAdded(const NetItem *child)
{
    QList<const NetItem *> objs;
    objs.append(child);
    while (!objs.isEmpty()) {
        const NetItem *obj = objs.takeFirst();

        switch (obj->itemType()) {
        case NetItemType::VPNControlItem: {
            const NetVPNControlItem *deviceItem = qobject_cast<const NetVPNControlItem *>(obj);
            connect(deviceItem, &NetDeviceItem::statusChanged, this, &NetStatus::updateVpnAndProxyStatus);
            connect(deviceItem, &NetDeviceItem::ipsChanged, this, &NetStatus::updateVpnAndProxyStatus);
            connect(deviceItem, &NetDeviceItem::nameChanged, this, &NetStatus::updateVpnAndProxyStatus);
            connect(deviceItem, &NetDeviceItem::enabledableChanged, this, &NetStatus::updateVpnAndProxyStatus);
            connect(deviceItem, &NetDeviceItem::enabledChanged, this, &NetStatus::updateVpnAndProxyStatus);
            updateVpnAndProxyStatus();
        } break;
        case NetItemType::SystemProxyControlItem: {
            const NetSystemProxyControlItem *deviceItem = qobject_cast<const NetSystemProxyControlItem *>(obj);
            connect(deviceItem, &NetControlItem::enabledChanged, this, &NetStatus::updateVpnAndProxyStatus);
            connect(deviceItem, &NetControlItem::enabledableChanged, this, &NetStatus::updateVpnAndProxyStatus);
            updateVpnAndProxyStatus();
        } break;
        case NetItemType::WiredDeviceItem:
        case NetItemType::WirelessDeviceItem: {
            const NetDeviceItem *deviceItem = qobject_cast<const NetDeviceItem *>(obj);
            connect(deviceItem, &NetDeviceItem::statusChanged, this, &NetStatus::updateStatus);
            connect(deviceItem, &NetDeviceItem::ipsChanged, this, &NetStatus::updateStatus);
            connect(deviceItem, &NetDeviceItem::nameChanged, this, &NetStatus::updateStatus);
            updateStatus();
        } break;
        case NetItemType::WiredItem: {
            const NetWiredItem *item = qobject_cast<const NetWiredItem *>(obj);
            connect(item, &NetWiredItem::statusChanged, this, &NetStatus::updateStatus);
        } break;
        case NetItemType::WirelessItem: {
            const NetWirelessItem *item = qobject_cast<const NetWirelessItem *>(obj);
            connect(item, &NetWirelessItem::statusChanged, this, &NetStatus::updateStatus);
            connect(item, &NetWirelessItem::strengthLevelChanged, this, &NetStatus::onStrengthLevelChanged);
        } break;
        default:
            break;
        }

        connect(obj, &NetItem::childAdded, this, &NetStatus::onChildAdded);
        int i = obj->getChildrenNumber();
        while (i--) {
            objs.append(obj->getChild(i));
        }
    }
}

void NetStatus::onChildRemoved(const NetItem *child)
{
    disconnect(child, nullptr, this, nullptr);
    updateStatus();
}

void NetStatus::onStrengthLevelChanged()
{
    const NetWirelessItem *item = qobject_cast<const NetWirelessItem *>(sender());
    if (item && item->status() != NetConnectionStatus::UnConnected) {
        updateStatus();
    }
}

void NetStatus::updateVpnAndProxyStatus()
{
    NetItem *root = m_manager->root();
    NetVPNControlItem *vpnControlItem = nullptr;
    NetSystemProxyControlItem *systemProxyControlItem = nullptr;
    for (auto it = root->childBegin(); it != root->childEnd(); ++it) {
        if (NetItemType::VPNControlItem == (*it)->itemType()) {
            vpnControlItem = qobject_cast<NetVPNControlItem *>(*it);
        }
        if (NetItemType::SystemProxyControlItem == (*it)->itemType()) {
            systemProxyControlItem = qobject_cast<NetSystemProxyControlItem *>(*it);
        }
        if (vpnControlItem && systemProxyControlItem)
            break;
    }

    m_vpnItem.Availabled = vpnControlItem && vpnControlItem->enabledable();
    m_vpnItem.Enabled = vpnControlItem && vpnControlItem->isEnabled();
    m_vpnItem.Connected = vpnControlItem && vpnControlItem->status() == NetDeviceStatus::Connected;
    m_proxyItem.Availabled = systemProxyControlItem && systemProxyControlItem->enabledable();
    m_proxyItem.Enabled = systemProxyControlItem && systemProxyControlItem->isEnabled();
    QString tips;
    if (m_vpnItem.Connected && m_proxyItem.Enabled) {
        tips = tr("Multiple services started");
        m_vpnAndProxyIcon = QIcon::fromTheme("proxy");
    } else if (m_vpnItem.Connected && !m_proxyItem.Enabled && vpnControlItem->ips().size() > 0) {
        tips = tr("Connected to: %1").arg(vpnControlItem->ips().first());
        m_vpnAndProxyIcon = QIcon::fromTheme("vpn");
    } else if (!m_vpnItem.Connected && m_proxyItem.Enabled) {
        tips = tr("System proxy enabled");
        m_vpnAndProxyIcon = QIcon::fromTheme("proxy");
    }

    if (m_vpnAndProxyBut) {
        const bool visible = m_vpnItem.Connected || m_proxyItem.Enabled;
        if (m_vpnAndProxyIconVisibel != visible) {
            m_vpnAndProxyIconVisibel = visible;
            m_vpnAndProxyBut->setVisible(m_vpnAndProxyIconVisibel);
            updateItemWidgetSize();
            Q_EMIT vpnAndProxyIconVisibelChanged(m_vpnAndProxyIconVisibel);
        }
        m_vpnAndProxyBut->setIcon(m_vpnAndProxyIcon);
    }
    if (m_vpnAndProxyTips != tips) {
        m_vpnAndProxyTips = tips;
        Q_EMIT vpnAndProxyTipsChanged(m_vpnAndProxyTips);
    }
}

bool NetStatus::vpnAndProxyIconVisibel() const
{
    return m_vpnAndProxyIconVisibel;
}

void NetStatus::updateStatus()
{
    if (!m_statusTimer->isActive()) {
        m_statusTimer->start();
    }
}

void NetStatus::doUpdateStatus()
{
    NetItem *root = m_manager->root();
    QStringList iphtml[DEVICE_ITEM_COUNT];
    unsigned devStatus[DEVICE_ITEM_COUNT] = { NetDeviceStatus::Unknown, NetDeviceStatus::Unknown };
    m_deviceFlag = 0;
    // 获取各设备状态
    for (auto it = root->childBegin(); it != root->childEnd(); ++it) {
        int index = WIRELESS_DEVICE_INDEX;
        switch ((*it)->itemType()) {
        case NetItemType::WiredDeviceItem:
            index = WIRED_DEVICE_INDEX;
            [[clang::fallthrough]]; // 去无break警告
        case NetItemType::WirelessDeviceItem: {
            NetDeviceItem *devItem = qobject_cast<NetDeviceItem *>(*it);
            if (devItem && devItem->isEnabled() && devItem->status() == NetDeviceStatus::Connected && !devItem->ips().isEmpty()) {
                QStringList ips = devItem->ips();
                if (ips.count() > 3) {
                    ips.erase(ips.begin() + 3, ips.end());
                    ips.append("......");
                }
                iphtml[index] << QString("<tr><td>%1 : </td><td>%2</td></tr>").arg(devItem->name()).arg(ips.join("<br/>"));
            }
            devStatus[index] |= devItem->status();

            m_deviceFlag |= (HAS_WIRELESS_DEVICE << index);
            if (devItem->isEnabled())
                m_deviceFlag |= (ENABLED_WIRELESS_DEVICE << index);
            if (devItem->enabledable())
                m_deviceFlag |= (ENABLEDABLE_WIRELESS_DEVICE << index);

        } break;
        default:
            break;
        }
    }
    iphtml[WIRELESS_DEVICE_INDEX].sort();
    iphtml[WIRED_DEVICE_INDEX].sort();

    unsigned netStatus = devStatus[WIRELESS_DEVICE_INDEX] | devStatus[WIRED_DEVICE_INDEX];
    bool isWirelessStatus = netStatus == devStatus[WIRELESS_DEVICE_INDEX];
    NetworkStatus networkStatus = NetworkStatus::Unknown;
    // 汇总有线无线状态
    // 优先级: 连接中 ip冲突 已连接但无网络 已连接 未连接 禁用 无网线 未知
    switch (netStatus) {
    case NetDeviceStatus::Authenticating:
    case NetDeviceStatus::ObtainingIP:
    case NetDeviceStatus::Connecting:
        if (devStatus[WIRELESS_DEVICE_INDEX] == devStatus[WIRED_DEVICE_INDEX]) {
            networkStatus = NetworkStatus::Connecting;
        } else if (isWirelessStatus) {
            networkStatus = NetworkStatus::WirelessConnecting;
        } else {
            networkStatus = NetworkStatus::WiredConnecting;
        }
        break;
    case NetDeviceStatus::IpConflicted:
        if (devStatus[WIRELESS_DEVICE_INDEX] == devStatus[WIRED_DEVICE_INDEX]) {
            networkStatus = NetworkStatus::WirelessIpConflicted;
        } else if (isWirelessStatus) {
            networkStatus = NetworkStatus::WirelessIpConflicted;
        } else {
            networkStatus = NetworkStatus::WiredIpConflicted;
        }
        break;
    case NetDeviceStatus::ConnectNoInternet:
        if (devStatus[WIRELESS_DEVICE_INDEX] == devStatus[WIRED_DEVICE_INDEX]) {
            networkStatus = NetworkStatus::ConnectNoInternet;
        } else if (isWirelessStatus) {
            networkStatus = NetworkStatus::WirelessConnectNoInternet;
        } else {
            networkStatus = NetworkStatus::WiredConnectNoInternet;
        }
        break;
    case NetDeviceStatus::Connected:
        if (devStatus[WIRELESS_DEVICE_INDEX] == devStatus[WIRED_DEVICE_INDEX]) {
            networkStatus = NetworkStatus::Connected;
        } else if (isWirelessStatus) {
            networkStatus = NetworkStatus::WirelessConnected;
        } else {
            networkStatus = NetworkStatus::WiredConnected;
        }
        break;
    case NetDeviceStatus::Disconnected:
        if (devStatus[WIRELESS_DEVICE_INDEX] == devStatus[WIRED_DEVICE_INDEX]) {
            networkStatus = NetworkStatus::Disconnected;
        } else if (isWirelessStatus) {
            networkStatus = NetworkStatus::WirelessDisconnected;
        } else {
            networkStatus = NetworkStatus::WiredDisconnected;
        }
        break;
    case NetDeviceStatus::ConnectFailed:
        if (devStatus[WIRELESS_DEVICE_INDEX] == devStatus[WIRED_DEVICE_INDEX]) {
            networkStatus = NetworkStatus::Disconnected;
        } else if (isWirelessStatus) {
            networkStatus = NetworkStatus::WirelessDisconnected;
        } else {
            networkStatus = NetworkStatus::WiredDisconnected;
        }
        break;
    case NetDeviceStatus::ObtainIpFailed:
        if (devStatus[WIRELESS_DEVICE_INDEX] == devStatus[WIRED_DEVICE_INDEX]) {
            networkStatus = NetworkStatus::Failed;
        } else if (isWirelessStatus) {
            networkStatus = NetworkStatus::WirelessFailed;
        } else {
            networkStatus = NetworkStatus::WiredFailed;
        }
        break;
    case NetDeviceStatus::Disabled:
        if (devStatus[WIRELESS_DEVICE_INDEX] == devStatus[WIRED_DEVICE_INDEX]) {
            networkStatus = NetworkStatus::Disabled;
        } else if (isWirelessStatus) {
            networkStatus = NetworkStatus::WirelessDisabled;
        } else {
            networkStatus = NetworkStatus::WiredDisabled;
        }
        break;
    case NetDeviceStatus::Enabled:
        if (devStatus[WIRELESS_DEVICE_INDEX] == devStatus[WIRED_DEVICE_INDEX]) {
            networkStatus = NetworkStatus::Disconnected;
        } else if (isWirelessStatus) {
            networkStatus = NetworkStatus::WirelessDisconnected;
        } else {
            networkStatus = NetworkStatus::WiredDisconnected;
        }
        break;
    case NetDeviceStatus::NoCable:
        networkStatus = NetworkStatus::Nocable;
        break;
    case NetDeviceStatus::Unknown:
        networkStatus = NetworkStatus::Unknown;
        break;
    }

    if (m_networkStatus != networkStatus) {
        m_networkStatus = networkStatus;

        bool hasDevice = m_networkStatus != NetworkStatus::Unknown;
        if (m_hasDevice != hasDevice) {
            m_hasDevice = hasDevice;
            Q_EMIT hasDeviceChanged(m_hasDevice);
        }
        Q_EMIT networkStatusChanged(m_networkStatus);
    }
    QString ipTips = iphtml[WIRELESS_DEVICE_INDEX].join(QString()) + iphtml[WIRED_DEVICE_INDEX].join(QString());
    if (ipTips != m_iphtml) {
        m_iphtml = ipTips;
        updateNetworkTips();
    }
    updateQuick(devStatus[WIRELESS_DEVICE_INDEX], devStatus[WIRED_DEVICE_INDEX]);
    qCInfo(DNC) << "Update status, wireless network: " << NetDeviceStatus(devStatus[WIRELESS_DEVICE_INDEX]) << ", wired network: " << NetDeviceStatus(devStatus[WIRED_DEVICE_INDEX])
            << ", status: " << networkStatus << ", device flag: " << QString::number(m_deviceFlag, 16) << ", ip: " << m_iphtml;
}

void NetStatus::updateNetworkTips()
{
    QString networkTips;
    switch (networkStatus()) {
    case NetworkStatus::WirelessConnected:
    case NetworkStatus::WiredConnected:
    case NetworkStatus::Connected: {
        networkTips = m_iphtml;
        if (networkTips.isEmpty())
            networkTips = tr("Connected");
    } break;
    case NetworkStatus::Disabled:
    case NetworkStatus::WirelessDisabled:
    case NetworkStatus::WiredDisabled: {
        networkTips = tr("Network off");
    } break;
    case NetworkStatus::Unknown:
    case NetworkStatus::Nocable: {
        networkTips = tr("Network cable unplugged");
    } break;
    case NetworkStatus::Disconnected:
    case NetworkStatus::WirelessDisconnected:
    case NetworkStatus::WiredDisconnected: {
        networkTips = tr("Not connected");
    } break;
    case NetworkStatus::Connecting:
    case NetworkStatus::WirelessConnecting:
    case NetworkStatus::WiredConnecting: {
        networkTips = tr("Connecting");
    } break;
    case NetworkStatus::ConnectNoInternet:
    case NetworkStatus::WirelessConnectNoInternet:
    case NetworkStatus::WiredConnectNoInternet: {
        networkTips = tr("Connected but no Internet access");
    } break;
    case NetworkStatus::Failed:
    case NetworkStatus::WirelessFailed:
    case NetworkStatus::WiredFailed: {
        networkTips = tr("Connection failed");
    } break;
    case NetworkStatus::WiredIpConflicted:
    case NetworkStatus::WirelessIpConflicted: {
        networkTips = tr("IP conflict");
    } break;
    default:
        break;
    }

    if (networkTips != m_networkTips) {
        m_networkTips = networkTips;
        Q_EMIT networkTipsChanged(m_networkTips);
    }
}

void NetStatus::updateNetworkIcon()
{
    QString iconString;
    NetWirelessItem *activeItem = nullptr;
    m_animationIcon.clear();
    switch (networkStatus()) {
    case NetworkStatus::Disabled:
    case NetworkStatus::WirelessDisabled:
        iconString = "network-wireless-disabled-symbolic";
        break;
    case NetworkStatus::WiredDisabled:
        iconString = "network-disabled-symbolic";
        break;
    case NetworkStatus::Connected:
    case NetworkStatus::WiredConnected:
    case NetworkStatus::WirelessConnected: {
        auto type = m_manager->primaryConnectionType();
        switch (type) {
        case NetManager::ConnectionType::Wireless:
        case NetManager::ConnectionType::Wired:
            break;
        default:
            type = networkStatus() == NetworkStatus::WiredConnected ? NetManager::ConnectionType::Wired : NetManager::ConnectionType::Wireless;
            break;
        }
        if (type == NetManager::ConnectionType::Wireless) {
            QVector<NetItem *> items = getDeviceConnections(NET_WIRELESS, static_cast<unsigned>(NetConnectionStatus::Connected));
            if (items.isEmpty()) {
                iconString = QString("network-wireless-signal-full-symbolic");
            } else {
                activeItem = NetItem::toItem<NetWirelessItem>(items.first());
                iconString = QString("network-wireless%1-symbolic").arg((activeItem->flags() ? "-6" : "") + NetManager::StrengthLevelString(activeItem->strengthLevel()));
            }
        } else if (type == NetManager::ConnectionType::Wired) {
            iconString = "network-online-symbolic";
        }
    } break;
    case NetworkStatus::Disconnected:
    case NetworkStatus::WirelessDisconnected:
        iconString = "network-wireless-disconnect";
        break;
    case NetworkStatus::WiredDisconnected:
        iconString = "network-none-symbolic";
        break;
    case NetworkStatus::Connecting: {
        QStringList wirelessAnimation({
                "network-wireless-signal-no-symbolic",
                "network-wireless-signal-low-symbolic",
                "network-wireless-signal-medium-symbolic",
                "network-wireless-signal-high-symbolic",
                "network-wireless-signal-full-symbolic",
        });
        m_animationIcon = QStringList({
                "network-wired-symbolic-connecting1",
                "network-wired-symbolic-connecting2",
                "network-wired-symbolic-connecting3",
                "network-wired-symbolic-connecting4",
                "network-wired-symbolic-connecting5",
                "network-wired-symbolic-connecting1",
                "network-wired-symbolic-connecting2",
                "network-wired-symbolic-connecting3",
                "network-wired-symbolic-connecting4",
                "network-wired-symbolic-connecting5",

        });
        m_animationIcon.append(wirelessAnimation);
        m_animationIcon.append(wirelessAnimation);
        iconString = m_animationIcon.first();
    } break;
    case NetworkStatus::WirelessConnecting: {
        m_animationIcon = QStringList({
                "network-wireless-signal-no-symbolic",
                "network-wireless-signal-low-symbolic",
                "network-wireless-signal-medium-symbolic",
                "network-wireless-signal-high-symbolic",
                "network-wireless-signal-full-symbolic",
        });
        iconString = m_animationIcon.first();
    } break;
    case NetworkStatus::WiredConnecting: {
        m_animationIcon = QStringList({
                "network-wired-symbolic-connecting1",
                "network-wired-symbolic-connecting2",
                "network-wired-symbolic-connecting3",
                "network-wired-symbolic-connecting4",
                "network-wired-symbolic-connecting5",
        });
        iconString = m_animationIcon.first();
    } break;
    case NetworkStatus::ConnectNoInternet:
    case NetworkStatus::WirelessConnectNoInternet: {
        // 无线已连接但无法访问互联网 offline
        QVector<NetItem *> items = getDeviceConnections(NET_WIRELESS, static_cast<unsigned>(NetConnectionStatus::Connected));
        if (items.isEmpty()) {
            iconString = QString("network-wireless-offline-signal-full-symbolic");
        } else {
            activeItem = NetItem::toItem<NetWirelessItem>(items.first());
            iconString = QString("network-wireless%1-offline%2-symbolic").arg((activeItem->flags() ? "-6" : "")).arg(NetManager::StrengthLevelString(activeItem->strengthLevel()));
        }
        break;
    }
    case NetworkStatus::WiredConnectNoInternet: {
        iconString = "network-offline-symbolic";
        break;
    }
    case NetworkStatus::WiredFailed: {
        // 有线连接失败none变为offline
        iconString = "network-offline-symbolic";
        break;
    }
    case NetworkStatus::Unknown:
    case NetworkStatus::Nocable: {
        // 待图标 暂用错误图标
        iconString = "network-error-symbolic";
        break;
    }
    case NetworkStatus::WirelessIpConflicted: {
        iconString = "network-wireless-offline-signal-full-symbolic";
        break;
    }
    case NetworkStatus::WiredIpConflicted: {
        iconString = "network-offline-symbolic";
        break;
    }
    case NetworkStatus::WirelessFailed:
    case NetworkStatus::Failed: {
        // 无线连接失败改为 disconnect
        iconString = "network-wireless-disconnect";
        break;
    }
    default:
        iconString = "network-wireless-disconnect";
        break;
    }
    if (m_animationIcon.isEmpty()) {
        m_animationTimer->stop();
    } else {
        m_animationTimer->start();
    }
    updateIconStr(iconString);
}

void NetStatus::updateIconStr(const QString &iconstr)
{
    if (iconstr != m_networkIconStr) {
        m_networkIconStr = iconstr;
        m_networkIcon = QIcon::fromTheme(m_networkIconStr);
        Q_EMIT networkIconChanged(m_networkIcon);
    }
}

void NetStatus::nextAnimation()
{
    ++m_currentAnimation;
    if (m_currentAnimation < 0 || m_currentAnimation >= m_animationIcon.count())
        m_currentAnimation = 0;
    updateIconStr(m_animationIcon.at(m_currentAnimation));
}

void NetStatus::updateQuick(unsigned wirelessStatus, unsigned wiredStatus)
{
    if (!m_quickAnimationTimer)
        return;

    bool networkActive = true;
    QString quickTitle;
    QString quickDescription;
    QString quickIconStr;
    m_quickAnimationIcon.clear();
    if (wirelessStatus != NetDeviceStatus::Unknown) {
        quickTitle = tr("Wireless Network");
        switch (wirelessStatus) {
        case NetDeviceStatus::Authenticating:
        case NetDeviceStatus::ObtainingIP:
        case NetDeviceStatus::Connecting:
            quickDescription = tr("Connecting");
            m_quickAnimationIcon = QStringList({
                    "network-wireless-signal-no-symbolic",
                    "network-wireless-signal-low-symbolic",
                    "network-wireless-signal-medium-symbolic",
                    "network-wireless-signal-high-symbolic",
                    "network-wireless-signal-full-symbolic",
            });
            quickIconStr = m_quickAnimationIcon.first();
            break;
        case NetDeviceStatus::IpConflicted:
            quickDescription = tr("IP conflict");
            quickIconStr = "network-wireless-offline-signal-full-symbolic";
            break;
        case NetDeviceStatus::ConnectNoInternet: {
            quickDescription = tr("Connected but no Internet access");
            quickIconStr = QString("network-wireless-offline-signal-full-symbolic");
        } break;
        case NetDeviceStatus::Connected: {
            QVector<NetItem *> items = getDeviceConnections(NET_WIRELESS, static_cast<unsigned>(NetConnectionStatus::Connected));
            if (items.isEmpty()) {
                quickDescription = tr("Connected");
            } else if (items.size() == 1) {
                const NetWirelessItem *wirelessItem = NetItem::toItem<NetWirelessItem>(items.first());
                quickDescription = wirelessItem->name();
            } else {
                quickDescription = tr("Connected %1").arg(items.size());
            }
            quickIconStr = "network-wireless-signal-full-symbolic";
        } break;
        case NetDeviceStatus::Disconnected:
        case NetDeviceStatus::ConnectFailed:
        case NetDeviceStatus::ObtainIpFailed:
        case NetDeviceStatus::Enabled:
            quickDescription = tr("Not connected");
            quickIconStr = "network-wireless-disconnect";
            break;
        case NetDeviceStatus::Disabled:
            networkActive = false;
            quickDescription = tr("Off");
            quickIconStr = "network-wireless-disabled-symbolic";
            break;
        default:
            quickDescription = tr("Not connected");
            quickIconStr = "network-wireless-disconnect";
            break;
        }
    } else if (wiredStatus != NetDeviceStatus::Unknown) {
        quickTitle = tr("Wired Network");
        switch (wiredStatus) {
        case NetDeviceStatus::Authenticating:
        case NetDeviceStatus::ObtainingIP:
        case NetDeviceStatus::Connecting:
            quickDescription = tr("Connecting");
            m_quickAnimationIcon = QStringList({
                    "network-wired-symbolic-connecting1",
                    "network-wired-symbolic-connecting2",
                    "network-wired-symbolic-connecting3",
                    "network-wired-symbolic-connecting4",
                    "network-wired-symbolic-connecting5",
            });
            quickIconStr = m_quickAnimationIcon.first();
            break;
        case NetDeviceStatus::IpConflicted:
            quickDescription = tr("IP conflict");
            quickIconStr = "network-offline-symbolic";
            break;
        case NetDeviceStatus::ConnectNoInternet:
            quickDescription = tr("Connected but no Internet access");
            quickIconStr = "network-offline-symbolic";
            break;
        case NetDeviceStatus::Connected: {
            QVector<NetItem *> items = getDeviceConnections(NET_WIRED, static_cast<unsigned>(NetConnectionStatus::Connected));
            if (items.isEmpty()) {
                quickDescription = tr("Connected");
            } else if (items.size() == 1) {
                quickDescription = items.first()->name();
            } else {
                quickDescription = tr("Connected %1").arg(items.size());
            }
            quickIconStr = "network-online-symbolic";
        } break;
        case NetDeviceStatus::Disconnected:
        case NetDeviceStatus::ConnectFailed:
        case NetDeviceStatus::ObtainIpFailed:
        case NetDeviceStatus::Enabled:
            quickDescription = tr("Not connected");
            quickIconStr = "network-none-symbolic";
            break;
        case NetDeviceStatus::Disabled:
            networkActive = false;
            quickDescription = tr("Off");
            quickIconStr = "network-disabled-symbolic";
            break;
        case NetDeviceStatus::NoCable:
            quickDescription = tr("Network cable unplugged");
            quickIconStr = "network-error-symbolic";
            break;
        default:
            quickDescription = tr("Not connected");
            quickIconStr = "network-none-symbolic";
            break;
        }
    } else {
        quickTitle = tr("Network");
        quickDescription = tr("Not connected");
        quickIconStr = "network-error-symbolic";
    }
    if (m_quickAnimationIcon.isEmpty()) {
        m_quickAnimationTimer->stop();
    } else {
        m_quickAnimationTimer->start();
    }
    if (m_networkActive != networkActive) {
        m_networkActive = networkActive;
        Q_EMIT networkActiveChanged(m_networkActive);
    }
    if (m_quickTitle != quickTitle) {
        m_quickTitle = quickTitle;
        Q_EMIT quickTitleChanged(m_quickTitle);
    }
    if (m_quickDescription != quickDescription) {
        m_quickDescription = quickDescription;
        Q_EMIT quickDescriptionChanged(m_quickDescription);
    }
    updateQuickIconStr(quickIconStr);
}

void NetStatus::updateQuickIconStr(const QString &iconstr)
{
    if (iconstr != m_quickIconStr) {
        m_quickIconStr = iconstr;
        m_quickIcon = QIcon::fromTheme(m_quickIconStr);
        Q_EMIT quickIconChanged(m_quickIcon);
    }
}

void NetStatus::nextQuickAnimation()
{
    ++m_quickCurrentAnimation;
    if (m_quickCurrentAnimation < 0 || m_quickCurrentAnimation >= m_quickAnimationIcon.count())
        m_quickCurrentAnimation = 0;
    updateQuickIconStr(m_quickAnimationIcon.at(m_quickCurrentAnimation));
}

QVector<NetItem *> NetStatus::getDeviceConnections(unsigned type, unsigned connectType) const
{
    QVector<NetItem *> activeItems[5]; // 与NetConnectionStatus对应,取最大,activeItems[0]未用到,只是方便后续处理
    QVector<NetItem *> items;
    items.append(m_manager->root());
    while (!items.isEmpty()) {
        NetItem *item = items.takeFirst();
        switch (item->itemType()) {
        case NetItemType::WirelessItem: {
            if (NetItemType::WirelessItem & type) {
                NetWirelessItem *netItem = NetItem::toItem<NetWirelessItem>(item);
                unsigned status = static_cast<unsigned>(netItem->status());
                if (status & connectType)
                    activeItems[status].append(netItem);
            }
        } break;
        case NetItemType::WiredItem:
            if (NetItemType::WiredItem & type) {
                NetWiredItem *netItem = NetItem::toItem<NetWiredItem>(item);
                unsigned status = static_cast<unsigned>(netItem->status());
                if (status & connectType)
                    activeItems[status].append(netItem);
            }
            break;
        default:
            items.append(item->getchildren());
            break;
        }
    }
    // 排序
    auto sortFun = [](const NetItem *item1, const NetItem *item2) {
        return item1->sortValue() < item2->sortValue();
    };
    for (int i = 0; i < 5; i++) {
        qSort(activeItems[i].begin(), activeItems[i].end(), sortFun);
    }
    return activeItems[static_cast<unsigned>(NetConnectionStatus::Connecting)] + activeItems[static_cast<unsigned>(NetConnectionStatus::Connected)]
            + activeItems[static_cast<unsigned>(NetConnectionStatus::UnConnected)];
}

void NetStatus::updateItemWidgetSize()
{
    if (!m_dockIconWidgetlayout)
        return;

    if (auto itemWidget = m_dockIconWidgetlayout->parentWidget()) {
        itemWidget->setFixedSize(itemWidget->sizeHint());
    }
}

} // namespace network
} // namespace dde
