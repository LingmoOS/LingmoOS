// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "netmanager.h"

#include "netitem.h"
#include "networkconst.h"
#include "private/netmanager_p.h"
#include "private/netmanagerthreadprivate.h"

#include <QApplication>
#include <QDebug>
#include <QTimer>

namespace dde {
namespace network {

NetManager::NetManager(QObject *parent, bool tipsLinkEnabled)
    : QObject(parent)
    , d_ptrNetManager(new NetManagerPrivate(this, tipsLinkEnabled))
{
    NetItemRegisterMetaType();
    qRegisterMetaType<NetManager::CmdType>("NetManager::CmdType");
    connect(d_ptrNetManager.get(), &NetManagerPrivate::request, this, &NetManager::request, Qt::QueuedConnection);
}

NetManager::~NetManager() { }

void NetManager::setServiceLoadForNM(bool isNM)
{
    Q_D(NetManager);
    d->setServiceLoadForNM(isNM);
}

void NetManager::setMonitorNetworkNotify(bool monitor)
{
    Q_D(NetManager);
    d->setMonitorNetworkNotify(monitor);
}

void NetManager::setUseSecretAgent(bool enabled)
{
    Q_D(NetManager);
    d->setUseSecretAgent(enabled);
}

void NetManager::setServerKey(const QString &serverKey)
{
    Q_D(NetManager);
    d->setServerKey(serverKey);
}

void NetManager::init()
{
    Q_D(NetManager);
    d->init();
}

void NetManager::setNetwork8021XMode(NetManager::Network8021XMode mode)
{
    Q_D(NetManager);
    d->setNetwork8021XMode(mode);
}

void NetManager::setAutoScanInterval(int ms)
{
    Q_D(NetManager);
    d->setAutoScanInterval(ms);
}

void NetManager::setAutoScanEnabled(bool enabled)
{
    Q_D(NetManager);
    d->setAutoScanEnabled(enabled);
}

void NetManager::setEnabled(bool enabled)
{
    Q_D(NetManager);
    d->setEnabled(enabled);
}

const bool NetManager::isGreeterMode() const
{
    Q_D(const NetManager);
    return d->m_isGreeterMode;
}

NetItem *NetManager::root() const
{
    Q_D(const NetManager);
    return d->m_root;
}

bool NetManager::isAirplaneMode() const
{
    Q_D(const NetManager);
    return d->m_airplaneMode;
}

NetManager::ConnectionType NetManager::primaryConnectionType() const
{
    Q_D(const NetManager);
    return d->m_primaryConnectionType;
}

/**
 * @brief NetManager::exec
 * @param cmd 执行命令 @see NetManager::CmdType
 * @param id 执行对象id
 * @param param 参数
 *
 * @note
 * cmd                  id              param
 * 执行操作 ui=>manager
 * EnabledDevice       netItem.id      空
 * DisabledDevice      netItem.id      空
 * RequestScan         netItem.id      空
 * Disconnect          netItem.id      空
 * Connect             netItem.id      空/QMap<QString, QString> 对应secrets里项 如：{{"identity","uos"},{"password","12345678"}}
 * UserCancelRequest   netItem.id/空   空
 * GoToControlCenter   要跳转的页的page  空   对应控制中心的ShowPage()
 * CheckInput          netItem.id      同Connect参数,实时请求检查
 * 请求交互 manager=>ui
 * RequestPassword     netItem.id      secrets(必需 QStringList)请求的密码，影响显示顺序，
 *                                      prop(非必需 QMap<QString, QString>)id的值(),如：{{"secrets"，{"identity","password"}},{"prop",{"identity","uos"}}
 * CloseInput          netItem.id/空   空
 * InputError          netItem.id      QMap<QString, QString>，对应secrets里各项错误 如：{{"password","密码为空"}}
 * InputValid          netItem.id      同InputError参数,CheckInput的返回
 */
void NetManager::exec(NetManager::CmdType cmd, const QString &id, const QVariantMap &param)
{
    QMetaObject::invokeMethod(d_ptrNetManager.get(), "exec", Qt::QueuedConnection, Q_ARG(NetManager::CmdType, cmd), Q_ARG(QString, id), Q_ARG(QVariantMap, param));
}

void NetManager::setWiredEnabled(bool enabled)
{
    exec(enabled ? NetManager::EnabledDevice : NetManager::DisabledDevice, "NetWiredControlItem");
}

void NetManager::setWirelessEnabled(bool enabled)
{
    exec(enabled ? NetManager::EnabledDevice : NetManager::DisabledDevice, "NetWirelessControlItem");
}

void NetManager::setVPNEnabled(bool enabled)
{
    exec(enabled ? NetManager::EnabledDevice : NetManager::DisabledDevice, "NetVPNControlItem");
}

void NetManager::setProxyEnabled(bool enabled)
{
    exec(enabled ? NetManager::EnabledDevice : NetManager::DisabledDevice, "NetSystemProxyControlItem");
}

void NetManager::gotoControlCenter()
{
    exec(NetManager::GoToControlCenter, "");
}

void NetManager::gotoCheckNet()
{
    exec(NetManager::GoToSecurityTools, "net-check");
}

void NetManager::updateLanguage(const QString &locale)
{
    Q_EMIT languageChange(locale);
}

int NetManager::StrengthLevel(int strength)
{
    if (65 < strength) {
        return 4;
    } else if (55 < strength) {
        return 3;
    } else if (30 < strength) {
        return 2;
    } else if (5 < strength) {
        return 1;
    }
    return 0;
}

QString NetManager::StrengthLevelString(int level)
{
    static const QStringList c_levelString({ "-signal-no", "-signal-low", "-signal-medium", "-signal-high", "-signal-full" });
    Q_ASSERT(level >= 0);
    Q_ASSERT(level < c_levelString.length());
    return c_levelString.at(level);
}

bool NetManager::netCheckAvailable()
{
    Q_D(NetManager);
    return d->netCheckAvailable();
}

///////////////////////////////////////////////////////////////////////
struct PasswordRequest
{
    QString dev;
    QString id;
    QVariantMap param;
    int requestCount;
};

NetManagerPrivate::NetManagerPrivate(NetManager *manager, bool tipsLinkEnabled)
    : QObject(manager)
    , m_root(new NetItem("ROOT"))
    , m_isDeleting(false)
    , m_airplaneMode(false)
    , m_primaryConnectionType(NetManager::ConnectionType::Wired)
    , m_monitorNetworkNotify(false)
    , m_secretAgent(false)
    , m_autoAddConnection(false)
    , m_managerThread(new NetManagerThreadPrivate)
    , m_passwordRequestData(nullptr)
    , q_ptr(manager)
{
    m_isGreeterMode = qApp->applicationName().contains("greeter");
    addItem(m_root);
    addItem(new NetWiredControlItem("NetWiredControlItem"));
    addItem(new NetWirelessControlItem("NetWirelessControlItem"));
    addItem(new NetWirelessDisabledItem("NetWirelessDisabledItem"));
    addItem(new NetWiredDisabledItem("NetWiredDisabledItem"));
    addItem(new NetAirplaneModeTipsItem("NetAirplaneModeTipsItem", "Airplane Mode", tipsLinkEnabled), m_root);
    if (!m_isGreeterMode) {
        NetVPNControlItem *vpnControlItem = new NetVPNControlItem("NetVPNControlItem");
        addItem(vpnControlItem);
        addItem(new NetVPNTipsItem("NetVPNTipsItem", "networkVpn", tipsLinkEnabled), vpnControlItem);
        addItem(new NetSystemProxyControlItem("NetSystemProxyControlItem"), m_root);
    }
    for (int i = 0; i < DeviceItemCount; ++i) {
        m_deviceCount[i] = 0;
    }
    connect(m_managerThread, &NetManagerThreadPrivate::itemAdded, this, &NetManagerPrivate::onItemAdded, Qt::QueuedConnection);
    connect(m_managerThread, &NetManagerThreadPrivate::itemRemoved, this, &NetManagerPrivate::onItemRemoved, Qt::QueuedConnection);
    connect(m_managerThread, &NetManagerThreadPrivate::dataChanged, this, &NetManagerPrivate::onDataChanged, Qt::QueuedConnection);
    connect(m_managerThread, &NetManagerThreadPrivate::request, this, &NetManagerPrivate::sendRequest, Qt::QueuedConnection);
    connect(m_managerThread, &NetManagerThreadPrivate::requestInputPassword, this, &NetManagerPrivate::onRequestPassword, Qt::QueuedConnection);
    connect(m_managerThread, &NetManagerThreadPrivate::networkNotify, q_ptr, &NetManager::networkNotify, Qt::QueuedConnection);
    connect(q_ptr, &NetManager::languageChange, m_managerThread, &NetManagerThreadPrivate::retranslate, Qt::QueuedConnection);
    connect(q_ptr, &NetManager::languageChange, this, &NetManagerPrivate::retranslateUi);
    connect(m_managerThread, &NetManagerThreadPrivate::toControlCenter, q_ptr, &NetManager::toControlCenter, Qt::QueuedConnection);
    connect(m_managerThread, &NetManagerThreadPrivate::netCheckAvailableChanged, q_ptr, &NetManager::netCheckAvailableChanged, Qt::QueuedConnection);
}

NetManagerPrivate::~NetManagerPrivate()
{
    m_isDeleting = true;

    delete m_managerThread;
    for (auto &&item : m_dataMap) {
        item->deleteLater(); // item有的有父对象会关联delete,用deleteLater让其自己处理
    }
    m_dataMap.clear();
}

void NetManagerPrivate::setServiceLoadForNM(bool isNM)
{
    m_managerThread->setServiceLoadForNM(isNM);
}

void NetManagerPrivate::setMonitorNetworkNotify(bool monitor)
{
    m_managerThread->setMonitorNetworkNotify(monitor);
}

void NetManagerPrivate::setUseSecretAgent(bool enabled)
{
    m_managerThread->setUseSecretAgent(enabled);
}

void NetManagerPrivate::setNetwork8021XMode(NetManager::Network8021XMode mode)
{
    m_managerThread->setNetwork8021XMode(mode);
}

void NetManagerPrivate::setAutoScanInterval(int ms)
{
    m_managerThread->setAutoScanInterval(ms);
}

void NetManagerPrivate::setAutoScanEnabled(bool enabled)
{
    m_managerThread->setAutoScanEnabled(enabled);
}

void NetManagerPrivate::setEnabled(bool enabled)
{
    m_managerThread->setEnabled(enabled);
}

void NetManagerPrivate::setServerKey(const QString &serverKey)
{
    m_managerThread->setServerKey(serverKey);
}

void NetManagerPrivate::init()
{
    m_managerThread->init();
}

bool NetManagerPrivate::netCheckAvailable()
{
    return m_managerThread->NetCheckAvailable();
}

void NetManagerPrivate::exec(NetManager::CmdType cmd, const QString &id, const QVariantMap &param)
{
    qCInfo(DNC) << "UI request exec cmd: " << cmd << ", id: " << id << ", param keys: " << param.keys();
    if (!id.isEmpty() && !m_showInputId.isEmpty() && m_showInputId != id) {
        // 有显示输入框，新的请求不是原处理
        clearPasswordRequest(m_showInputId);
        sendRequest(NetManager::CloseInput, m_showInputId);
    }
    switch (cmd) {
    case NetManager::DisabledDevice:
    case NetManager::EnabledDevice: {
        setDeviceEnabled(id, cmd == NetManager::EnabledDevice);
    } break;
    case NetManager::RequestScan: {
        NetItem *item = findItem(id);
        if (item && item->itemType() == NetItemType::WirelessDeviceItem) {
            m_managerThread->requestScan(id);
            // 点击刷新按钮，需要展开其他网络
            NetWirelessOtherItem *otherItem = NetItem::toItem<NetWirelessOtherItem>(findItem(item->id() + ":Other"));
            if (otherItem)
                otherItem->updateExpanded(true);
        }
    } break;
    case NetManager::Disconnect: {
        NetItem *item = findItem(id);
        while (item) {
            switch (item->itemType()) {
            case NetItemType::WirelessDeviceItem:
            case NetItemType::WiredDeviceItem: {
                m_managerThread->disconnectDevice(item->id());
                item = nullptr; // break while
            } break;
            default:
                item = item->getParent();
                break;
            }
        }
    } break;
    case NetManager::Connect: {
        const QVariantMap validMap = NetManagerThreadPrivate::CheckParamValid(param);
        if (!validMap.isEmpty()) {
            sendRequest(NetManager::InputError, id, validMap);
            break;
        }
        NetItem *item = findItem(id);
        if (item) {
            switch (item->itemType()) {
            case NetItemType::WirelessHiddenItem: {
                if (param.contains("ssid")) {
                    m_managerThread->connectHidden(id.split(":").first(), param.value("ssid").toString());
                    sendRequest(NetManager::CloseInput, id);
                } else {
                    QVariantMap param;
                    param.insert("secrets", { "ssid" });
                    sendRequest(NetManager::RequestPassword, id, param);
                }
            } break;
            case NetItemType::WiredItem: {
                NetWiredItem *wiredItem = NetItem::toItem<NetWiredItem>(item);
                if (wiredItem->status() == NetConnectionStatus::UnConnected) {
                    m_managerThread->connectWired(id, param);
                }
            } break;
            case NetItemType::WirelessItem: {
                NetWirelessItem *wirelessItem = NetItem::toItem<NetWirelessItem>(item);
                if (wirelessItem->status() == NetConnectionStatus::UnConnected || !param.isEmpty()) { // 连接中有可能请求密码
                    m_managerThread->connectWireless(id, param);
                }
            } break;
            default:
                break;
            }
        }
    } break;
    case NetManager::CheckInput: {
        sendRequest(NetManager::InputValid, id, NetManagerThreadPrivate::CheckParamValid(param));
    } break;
    case NetManager::UserCancelRequest: {
        clearPasswordRequest(id);
        sendRequest(NetManager::CloseInput, id);
    } break;
    case NetManager::GoToControlCenter:
        m_managerThread->gotoControlCenter(id);
        break;
    case NetManager::GoToSecurityTools:
        m_managerThread->gotoSecurityTools(id);
        break;
    case NetManager::ToggleExpand: {
        if (id.isEmpty()) { // 初始化状态
            QVector<NetItem *> items = m_root->getchildren();
            NetItem *inputParent = nullptr;
            if (!m_showInputId.isEmpty()) {
                NetItem *inputItem = findItem(m_showInputId);
                if (inputItem) {
                    inputParent = inputItem->getParent();
                }
            }
            while (!items.isEmpty()) {
                NetItem *item = items.takeFirst();
                switch (item->itemType()) {
                case NetItemType::WirelessOtherItem: {
                    NetWirelessOtherItem *otherItem = NetItem::toItem<NetWirelessOtherItem>(item);
                    // 没有我的网络则展开其他网络
                    otherItem->updateExpanded(otherItem == inputParent || otherItem->getParent()->getChildrenNumber() == 1);
                } break;
                case NetItemType::WirelessDeviceItem:
                    items.append(item->getchildren());
                    break;
                default:
                    break;
                }
            }
        } else {
            NetItem *item = findItem(id);
            if (item && item->itemType() == NetItemType::WirelessOtherItem) {
                NetWirelessOtherItem *otherItem = NetItem::toItem<NetWirelessOtherItem>(item);
                otherItem->updateExpanded(!otherItem->isExpanded());
            }
        }
    } break;
    default:
        break;
    }
}

void NetManagerPrivate::onItemAdded(const QString &parentID, NetItem *item)
{
    QString itemParentID = parentID;
    if (item->itemType() == NetItemType::WirelessItem) {
        NetWirelessItem *wirelessItem = static_cast<NetWirelessItem *>(item);
        itemParentID = parentID + (wirelessItem->hasConnection() ? ":Mine" : ":Other");
    }
    NetItem *parentItem = findItem(itemParentID);
    if (!parentItem) {
        qCWarning(DNC) << "Item added, The item's parent not found, parent id: " << parentID << ", item: " << item;
        delete item;
        return;
    }
    addItem(item, parentItem);
    switch (item->itemType()) {
    case NetItemType::WirelessDeviceItem: { // 无线设备添加隐藏网络
        addItem(new NetWirelessMineItem(item->id() + ":Mine"), nullptr);
        NetWirelessOtherItem *otherItem = new NetWirelessOtherItem(item->id() + ":Other");
        addItem(otherItem, item);
        addItem(new NetWirelessHiddenItem(item->id() + ":Hidden"), otherItem);
        ++m_deviceCount[WirelessDeviceIndex];
    } break;
    case NetItemType::WiredDeviceItem: {
        ++m_deviceCount[WiredDeviceIndex];
    } break;
    case NetItemType::WirelessItem: {
        if (!parentItem->getParent()) {
            addItem(parentItem, findItem(parentID));
        }
    }
        return;
    default:
        return;
    }
    updateControl();
}

void NetManagerPrivate::onItemRemoved(const QString &id)
{
    NetItem *item = findItem(id);
    if (!item) {
        qCWarning(DNC) << "Item removed, item: " << id << "not find!";
        return;
    }

    NetItem *mine = nullptr;
    switch (item->itemType()) {
    case NetItemType::WirelessDeviceItem: { // 无线设备添加隐藏网络
        --m_deviceCount[WirelessDeviceIndex];
        // 我的网络和其他网络要单独删除,可能在m_dataMap中但不在列表中
        removeAndDeleteItem(findItem(item->id() + ":Mine"));
        removeAndDeleteItem(findItem(item->id() + ":Other"));
        updateControl();
    } break;
    case NetItemType::WiredDeviceItem: {
        --m_deviceCount[WiredDeviceIndex];
        updateControl();
    } break;
    case NetItemType::WirelessItem: {
        NetWirelessItem *wirelessItem = static_cast<NetWirelessItem *>(item);
        if (wirelessItem->hasConnection()) {
            mine = findItem(item->getParent()->id()); // Mine
        }
    } break;
    default:
        break;
    }
    removeAndDeleteItem(item);
    if (mine && mine->getChildrenNumber() == 0) // Mine的最后一个将被删除
        removeItem(mine);
}

void NetManagerPrivate::onDataChanged(int dataType, const QString &id, const QVariant &value)
{
    switch (dataType) {
    case NetManagerThreadPrivate::primaryConnectionTypeChanged: {
        updatePrimaryConnectionType(NetManager::ConnectionType(value.toInt()));
    } break;
    case NetManagerThreadPrivate::AirplaneModeEnabledChanged: {
        updateAirplaneMode(value.toBool());
    } break;
    case NetManagerThreadPrivate::VPNConnectionStateChanged: {
        NetVPNControlItem *item = NetItem::toItem<NetVPNControlItem>(findItem("NetVPNControlItem"));
        if (item) {
            NetDeviceStatus state = value.value<NetDeviceStatus>();
            const bool showTips = (state != NetDeviceStatus::Connected) && (state != NetDeviceStatus::Connecting);
            item->updateExpanded(showTips && item->isEnabled() && item->enabledable());
            item->updateStatus(state);
        }
    } break;
    case NetManagerThreadPrivate::NameChanged: {
        NetItem *item = findItem(id);
        if (item)
            item->updateName(value.toString());
    } break;
    case NetManagerThreadPrivate::EnabledChanged: {
        NetControlItem *item = NetItem::toItem<NetControlItem>(findItem(id));
        if (item) {
            item->updateEnabled(value.toBool());
            updateControlEnabled(item->itemType());
        }
    } break;
    case NetManagerThreadPrivate::DeviceAvailableChanged: {
        NetControlItem *item = NetItem::toItem<NetControlItem>(findItem(id));
        if (item) {
            item->updateEnabledable(value.toBool());
            if (item->itemType() == SystemProxyControlItem || item->itemType() == VPNControlItem) {
                updateItemVisible(item->id(), value.toBool());
                return;
            }
            updateControlEnabled(item->itemType());
        }
    } break;
    case NetManagerThreadPrivate::WiredStatusChanged: {
        NetWiredItem *item = NetItem::toItem<NetWiredItem>(findItem(id));
        if (item)
            item->updateStatus(value.value<NetConnectionStatus>());
    } break;
    case NetManagerThreadPrivate::WirelessStatusChanged: {
        NetWirelessItem *item = NetItem::toItem<NetWirelessItem>(findItem(id));
        if (item) {
            NetConnectionStatus state = value.value<NetConnectionStatus>();
            item->updateStatus(state);
        }
    } break;
    case NetManagerThreadPrivate::StrengthChanged: {
        NetWirelessItem *item = NetItem::toItem<NetWirelessItem>(findItem(id));
        if (item)
            item->updateStrength(value.toInt());
    } break;
    case NetManagerThreadPrivate::SecuredChanged: {
        NetWirelessItem *item = NetItem::toItem<NetWirelessItem>(findItem(id));
        if (item)
            item->updateSecure(value.toBool());
    } break;
    case NetManagerThreadPrivate::IPChanged: {
        NetDeviceItem *item = NetItem::toItem<NetDeviceItem>(findItem(id));
        if (item)
            item->updateIps(value.toStringList());
    } break;
    case NetManagerThreadPrivate::DeviceStatusChanged: {
        NetDeviceItem *item = NetItem::toItem<NetDeviceItem>(findItem(id));
        if (item) {
            NetDeviceStatus deviceStatus = value.value<NetDeviceStatus>();
            if (item->status() != NetDeviceStatus::IpConflicted && deviceStatus == NetDeviceStatus::IpConflicted) {
                // 如果IP冲突，需要发送横幅通知
                m_managerThread->sendNotify((item->itemType() & NET_WIRED ? "notification-network-wired-local" : "notification-network-wireless-local"),
                                            tr("IP conflict"),
                                            tr("Network"),
                                            "dde-control-center",
                                            -1,
                                            {},
                                            {},
                                            3000);
            }
            item->updateStatus(deviceStatus);
        }
    } break;
    case NetManagerThreadPrivate::HotspotEnabledChanged: {
        NetWirelessDeviceItem *item = NetItem::toItem<NetWirelessDeviceItem>(findItem(id));
        if (item)
            item->updateApMode(value.toBool());
    } break;
    case NetManagerThreadPrivate::AvailableConnectionsChanged: {
        NetWirelessDeviceItem *item = qobject_cast<NetWirelessDeviceItem *>(findItem(id));
        if (item) {
            const QStringList &connList = value.toStringList();
            NetItem *mine = findItem(id + ":Mine");
            NetItem *other = findItem(id + ":Other");
            QVector<NetItem *> items = mine->getchildren();
            items.append(other->getchildren());
            for (auto &&item : items) {
                NetWirelessItem *wirelessItem = qobject_cast<NetWirelessItem *>(item);
                if (wirelessItem) {
                    if (connList.contains(wirelessItem->id())) {
                        wirelessItem->updateHasConnection(true);
                        if (wirelessItem->getParent() == other) {
                            other->removeChild(wirelessItem);
                        }
                        if (wirelessItem->getParent() != mine) {
                            mine->addChild(wirelessItem);
                        }
                    } else {
                        wirelessItem->updateHasConnection(false);
                        if (wirelessItem->getParent() == mine) {
                            mine->removeChild(wirelessItem);
                        }
                        if (wirelessItem->getParent() != other) {
                            other->addChild(wirelessItem);
                        }
                    }
                }
            }
            if (!mine->getParent() && mine->getChildrenNumber() != 0) {
                findItem(id)->addChild(mine);
            } else if (mine->getParent() && mine->getChildrenNumber() == 0) {
                mine->getParent()->removeChild(mine);
            }
        }
    } break;
    default:
        break;
    }
}

void NetManagerPrivate::sendRequest(NetManager::CmdType cmd, const QString &id, const QVariantMap &param)
{
    qCInfo(DNC) << "Send request, cmd: " << cmd << ", id: " << id << ", param: " << param;
    Q_EMIT request(cmd, id, param);
    switch (cmd) {
    case NetManager::RequestPassword: {
        if (!m_showInputId.isEmpty())
            qCWarning(DNC) << "Untreated request password:" << m_showInputId << ", new request password: " << id;
        m_showInputId = id;
        NetWirelessItem *item = NetItem::toItem<NetWirelessItem>(findItem(id));
        if (item && !item->hasConnection()) {
            NetWirelessOtherItem *otherItem = NetItem::toItem<NetWirelessOtherItem>(findItem(item->getParent()->getParent()->id() + ":Other"));
            if (otherItem)
                otherItem->updateExpanded(true);
        }
    } break;
    case NetManager::CloseInput:
        if (id.isEmpty() || id == m_showInputId) {
            m_showInputId.clear();
        }
        break;
    default:
        break;
    }
}

void NetManagerPrivate::onRequestPassword(const QString &dev, const QString &id, const QVariantMap &param)
{
    if (m_passwordRequestData) {
        delete m_passwordRequestData;
        m_passwordRequestData = nullptr;
    }

    if (param.isEmpty()) { // 取消请求
        if (!m_showInputId.isEmpty()) {
            NetItem *item = findItem(m_showInputId);
            if (item && item->name() == id) {
                sendRequest(NetManager::CloseInput, m_showInputId);
            }
        }
        return;
    }

    m_passwordRequestData = new PasswordRequest;
    m_passwordRequestData->dev = dev;
    m_passwordRequestData->id = id;
    m_passwordRequestData->param = param;
    m_passwordRequestData->requestCount = 0;
    findPasswordRequestItem();
}

void NetManagerPrivate::findPasswordRequestItem()
{
    if (!m_passwordRequestData)
        return;
    QString itemId;
    m_passwordRequestData->requestCount++;
    // dev为空时，优先找id相等且为连接中的
    if (m_passwordRequestData->dev.isEmpty()) {
        QVector<NetItem *> items;
        items.append(m_root);
        while (!items.isEmpty()) {
            NetItem *item = items.takeFirst();
            switch (item->itemType()) {
            case NetItemType::WirelessItem: {
                NetWirelessItem *wirelessitem = NetItem::toItem<NetWirelessItem>(item);
                if (wirelessitem->name() == m_passwordRequestData->id) {
                    // 如果查找15次连接状态还没更新就不判断状态
                    if (m_passwordRequestData->requestCount > 15)
                        itemId = wirelessitem->id();

                    if (wirelessitem->status() == NetConnectionStatus::Connecting) {
                        itemId = wirelessitem->id();
                        break;
                    }
                }
            } break;
            default:
                items.append(item->getchildren());
                break;
            }
        }
    } else {
        NetItem *devItem = findItem(m_passwordRequestData->dev);
        if (devItem) {
            switch (devItem->itemType()) {
            case NetItemType::WirelessDeviceItem: {
                QVector<NetItem *> items = devItem->getchildren();
                while (!items.isEmpty()) {
                    NetItem *item = items.takeFirst();
                    switch (item->itemType()) {
                    case NetItemType::WirelessItem: {
                        if (item->name() == m_passwordRequestData->id) {
                            itemId = item->id();
                            break;
                        }
                    } break;
                    default:
                        items.append(item->getchildren());
                        break;
                    }
                }
            } break;
            case NetItemType::WiredDeviceItem:
                break;
            default:
                break;
            }
        }
    }
    if (!itemId.isEmpty()) {
        sendRequest(NetManager::RequestPassword, itemId, m_passwordRequestData->param);
        delete m_passwordRequestData;
        m_passwordRequestData = nullptr;
    } else {
        // 没找到则延时再进入
        QTimer::singleShot(200, this, &NetManagerPrivate::findPasswordRequestItem);
    }
}

void NetManagerPrivate::clearPasswordRequest(const QString &id)
{
    m_managerThread->userCancelRequest(id);
    if (m_passwordRequestData) {
        delete m_passwordRequestData;
        m_passwordRequestData = nullptr;
    }
}

void NetManagerPrivate::retranslateUi()
{
    for (auto &&item : m_dataMap) {
        item->retranslateUi();
    }
}

void NetManagerPrivate::onItemDestroyed(QObject *obj)
{
    if (m_isDeleting || !obj)
        return;
    // 此时NetItem的成员变量已delete,只能获取QObject里的数据
    m_dataMap.remove(obj->objectName());
}

void NetManagerPrivate::setDeviceEnabled(const QString &id, bool enabled)
{
    NetItem *item = findItem(id);
    if (!item)
        return;

    NetItemType itemType = item->itemType();
    switch (itemType) {
    case NetItemType::WiredControlItem:
    case NetItemType::WirelessControlItem: {
        int type = (itemType & NET_MASK_TYPE) | NET_DEVICE;
        int i = m_root->getChildrenNumber();
        while (i--) {
            NetItem *item = m_root->getChild(i);
            if (item->itemType() == type) {
                setDeviceEnabled(NetItem::toItem<NetControlItem>(item), enabled);
            }
        }
    } break;
    case NetItemType::WiredDeviceItem:
    case NetItemType::WirelessDeviceItem:
    case NetItemType::VPNControlItem:
    case NetItemType::SystemProxyControlItem: {
        setDeviceEnabled(NetItem::toItem<NetControlItem>(item), enabled);
    } break;
    default:
        break;
    }
}

void NetManagerPrivate::setDeviceEnabled(NetControlItem *controlItem, bool enabled)
{
    if (!controlItem)
        return;

    switch (controlItem->itemType()) {
    case NetItemType::WiredControlItem:
    case NetItemType::WirelessControlItem: {
        int type = (controlItem->itemType() & NET_MASK_TYPE) | NET_DEVICE;
        int i = m_root->getChildrenNumber();
        while (i--) {
            NetItem *item = m_root->getChild(i);
            if (item->itemType() == type) {
                setDeviceEnabled(NetItem::toItem<NetControlItem>(item), enabled);
            }
        }
    } break;
    case NetItemType::WiredDeviceItem:
    case NetItemType::WirelessDeviceItem: {
        if (controlItem->enabledable() && controlItem->isEnabled() != enabled) {
            controlItem->updateEnabledable(false);
            m_managerThread->setDeviceEnabled(controlItem->id(), enabled);
        }
    } break;
    case NetItemType::VPNControlItem:
    case NetItemType::SystemProxyControlItem: {
        if (controlItem->enabledable() && controlItem->isEnabled() != enabled) {
            m_managerThread->setDeviceEnabled(controlItem->id(), enabled);
        }
    } break;
    default:
        break;
    }
}

void NetManagerPrivate::updateControl()
{
    updateControlEnabled(NetItemType::WiredDeviceItem);
    updateControlEnabled(NetItemType::WirelessDeviceItem);
}

void NetManagerPrivate::updateItemVisible(const QString &id, bool visible)
{
    NetItem *item = findItem(id);
    if (!item) {
        qCWarning(DNC) << "Update item visible " << visible << ", The item: " << id << " not find!";
        return;
    }
    if (visible) {
        if (!item->getParent())
            m_root->addChild(item);
    } else {
        NetItem *parentItem = item->getParent();
        if (parentItem)
            parentItem->removeChild(item);
    }
}

void NetManagerPrivate::updateControlEnabled(int type)
{
    int devType = (type & NET_MASK_TYPE) | NET_DEVICE;
    int parentType = (type & NET_MASK_TYPE) | NET_CONTROL;
    NetControlItem *parentItem = nullptr;
    bool pEnabled = false;
    bool pEnabledable = false;
    for (int i = 0; i < m_root->getChildrenNumber(); ++i) {
        NetItem *item = m_root->getChild(i);
        if (item->itemType() == devType) {
            NetControlItem *dev = NetItem::toItem<NetDeviceItem>(item);
            if (dev) {
                pEnabled |= dev->isEnabled();
                pEnabledable |= dev->enabledable();
            }
        } else if (item->itemType() == parentType) {
            parentItem = NetItem::toItem<NetControlItem>(item);
        }
    }
    if (parentItem) {
        parentItem->updateEnabled(pEnabled);
        parentItem->updateEnabledable(pEnabledable);
    }

    bool devDisabled = false;
    int index = ((type & NET_MASK_TYPE) == NET_WIRELESS) ? WirelessDeviceIndex : WiredDeviceIndex;
    if (m_deviceCount[index] == 1 && (m_deviceCount[WiredDeviceIndex] + m_deviceCount[WirelessDeviceIndex] == 1)) {
        devDisabled = !pEnabled;
    }
    updateItemVisible(index == WirelessDeviceIndex ? "NetWirelessDisabledItem" : "NetWiredDisabledItem", devDisabled);
}

void NetManagerPrivate::updateAirplaneMode(bool enabled)
{
    if (m_airplaneMode != enabled) {
        m_airplaneMode = enabled;
        Q_Q(NetManager);
        Q_EMIT q->airplaneModeChanged(m_airplaneMode);
    }
    updateItemVisible("NetAirplaneModeTipsItem", enabled);
    if (enabled) {
        updateItemVisible("NetWirelessDisabledItem", false);
        updateItemVisible("NetWiredDisabledItem", false);
    }
}

void NetManagerPrivate::updatePrimaryConnectionType(NetManager::ConnectionType type)
{
    if (m_primaryConnectionType != type) {
        m_primaryConnectionType = type;
        Q_Q(NetManager);
        Q_EMIT q->primaryConnectionTypeChanged(m_primaryConnectionType);
    }
}

void NetManagerPrivate::addItem(NetItem *item, NetItem *parentItem)
{
    NetItem *oldItem = findItem(item->id());
    if (!oldItem) {
        m_dataMap.insert(item->id(), item);
        connect(item, &NetItem::destroyed, this, &NetManagerPrivate::onItemDestroyed);
    }

    if (parentItem)
        parentItem->addChild(item);
}

void NetManagerPrivate::removeItem(NetItem *item)
{
    if (!item)
        return;
    NetItem *parentItem = item->getParent();
    if (!parentItem) {
        return;
    }
    parentItem->removeChild(item);
}

void NetManagerPrivate::removeAndDeleteItem(NetItem *item)
{
    if (!item)
        return;
    removeItem(item);
    m_dataMap.remove(item->id());
    delete item;
}

} // namespace network
} // namespace dde
