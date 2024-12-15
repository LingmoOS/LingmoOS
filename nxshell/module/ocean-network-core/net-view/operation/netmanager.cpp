// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "netmanager.h"

#include "netitem.h"
#include "networkconst.h"
#include "private/netitemprivate.h"
#include "private/netmanager_p.h"
#include "private/netmanagerthreadprivate.h"

#include <QCoreApplication>
#include <QDebug>
#include <QFile>
#include <QTimer>
Q_LOGGING_CATEGORY(DNC, "org.lingmo.ocean.oceanui.network");

namespace ocean {
namespace network {

static void NetItemRegisterMetaType()
{
    qRegisterMetaType<NetType::NetItemType>("NetItemType");
    qRegisterMetaType<NetConnectionStatus>("NetConnectionStatus");
    qRegisterMetaType<NetType::NetDeviceStatus>("NetDeviceStatus");
    qRegisterMetaType<NetManager::CmdType>("NetManager::CmdType");
}

NetManager::NetManager(NetType::NetManagerFlags flags, QObject *parent)
    : QObject(parent)
    , d_ptrNetManager(new NetManagerPrivate(this))
{
    NetItemRegisterMetaType();
    connect(d_ptrNetManager.get(), &NetManagerPrivate::request, this, &NetManager::request, Qt::QueuedConnection);
    d_ptrNetManager->init(flags);
}

NetManager::~NetManager() { }

void NetManager::setServerKey(const QString &serverKey)
{
    Q_D(NetManager);
    d->setServerKey(serverKey);
}

bool NetManager::CheckPasswordValid(const QString &key, const QString &password)
{
    return NetManagerThreadPrivate::CheckPasswordValid(key, password);
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

NetItem *NetManager::root() const
{
    Q_D(const NetManager);
    return d->m_root->item();
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

NetManagerPrivate::NetManagerPrivate(NetManager *manager)
    : QObject(manager)
    , m_root(NetItemNew(AirplaneControlItem, "Root"))
    , m_deleteItem(NetItemNew(Item, "trash"))
    , m_isDeleting(false)
    , m_airplaneMode(false)
    // , m_primaryConnectionType(NetManager::ConnectionType::Wired)
    , m_monitorNetworkNotify(false)
    , m_secretAgent(false)
    , m_autoAddConnection(false)
    , m_managerThread(new NetManagerThreadPrivate)
    , m_passwordRequestData(nullptr)
    , q_ptr(manager)
{
    m_root->updateenabled(false);
    addItem(m_root, nullptr);
    addItem(m_deleteItem, nullptr);
    addItem(NetItemPrivate::New(NetType::NetItemType::WiredControlItem, "NetWiredControlItem"), m_deleteItem);
    addItem(NetItemPrivate::New(NetType::NetItemType::WirelessControlItem, "NetWirelessControlItem"), m_deleteItem);

    for (int i = 0; i < DeviceItemCount; ++i) {
        m_deviceCount[i] = 0;
    }
    connect(m_managerThread, &NetManagerThreadPrivate::itemAoceand, this, &NetManagerPrivate::onItemAoceand, Qt::QueuedConnection);
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
    delete m_deleteItem;
    delete m_root;
    m_root = nullptr;
    m_dataMap.clear();
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

void NetManagerPrivate::init(NetType::NetManagerFlags flags)
{
    m_managerThread->init(flags);
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
        NetItemPrivate *item = findItem(id);
        if (item && item->itemType() == NetType::NetItemType::WirelessDeviceItem) {
            m_managerThread->requestScan(id);
            // 点击刷新按钮，需要展开其他网络
            NetWirelessOtherItemPrivate *otherItem = NetItemPrivate::toItem<NetWirelessOtherItemPrivate>(findItem(item->id() + ":Other"));
            if (otherItem)
                otherItem->updateexpanded(true);
        }
    } break;
    case NetManager::Disconnect: {
        NetItemPrivate *item = findItem(id);
        while (item) {
            switch (item->itemType()) {
            case NetType::NetItemType::WirelessDeviceItem:
            case NetType::NetItemType::WiredDeviceItem: {
                m_managerThread->disconnectDevice(item->id());
                item = nullptr; // break while
            } break;
            case NetType::NetItemType::HotspotControlItem: {
                m_managerThread->connectHotspot(id, param, false);
                item = nullptr;
            } break;
            case NetType::NetItemType::ConnectionItem: {
                m_managerThread->disconnectConnection(id);
                item = nullptr;
            } break;
            default:
                item = item->getParentPrivate();
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
        NetItemPrivate *item = findItem(id);
        if (item) {
            switch (item->itemType()) {
            case NetType::NetItemType::WirelessHioceannItem: {
                if (param.contains("ssid")) {
                    m_managerThread->connectHioceann(id.split(":").first(), param.value("ssid").toString());
                    sendRequest(NetManager::CloseInput, id);
                } else {
                    QVariantMap param;
                    param.insert("secrets", { "ssid" });
                    sendRequest(NetManager::RequestPassword, id, param);
                }
            } break;
            case NetType::NetItemType::WiredItem: {
                NetWiredItem *wiredItem = NetItem::toItem<NetWiredItem>(item->item());
                if (wiredItem->status() == NetType::NetConnectionStatus::CS_UnConnected) {
                    m_managerThread->connectWired(id, param);
                }
            } break;
            case NetType::NetItemType::WirelessItem: {
                NetWirelessItem *wirelessItem = NetItem::toItem<NetWirelessItem>(item->item());
                if (wirelessItem->status() == NetType::NetConnectionStatus::CS_UnConnected || !param.isEmpty()) { // 连接中有可能请求密码
                    m_managerThread->connectWireless(id, param);
                }
            } break;
            case NetType::NetItemType::HotspotControlItem: {
                m_managerThread->connectHotspot(id, param, true);
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
            QVector<NetItem *> items = m_root->getChildren();
            NetItemPrivate *inputParent = nullptr;
            if (!m_showInputId.isEmpty()) {
                NetItemPrivate *inputItem = findItem(m_showInputId);
                if (inputItem) {
                    inputParent = inputItem->getParentPrivate();
                }
            }
            while (!items.isEmpty()) {
                NetItem *item = items.takeFirst();
                switch (item->itemType()) {
                case NetType::NetItemType::WirelessOtherItem: {
                    NetWirelessOtherItemPrivate *otherItem = NetItemPrivate::toItem<NetWirelessOtherItemPrivate>(item);
                    // 没有我的网络则展开其他网络
                    otherItem->updateexpanded(otherItem == inputParent || otherItem->getParentPrivate()->getChildrenNumber() == 1);
                } break;
                case NetType::NetItemType::WirelessDeviceItem:
                    items.append(item->getChildren());
                    break;
                default:
                    break;
                }
            }
        } else {
            NetItemPrivate *item = findItem(id);
            if (item && item->itemType() == NetType::NetItemType::WirelessOtherItem) {
                NetWirelessOtherItemPrivate *otherItem = NetItemPrivate::toItem<NetWirelessOtherItemPrivate>(item);
                otherItem->updateexpanded(!otherItem->isExpanded());
            }
        }
    } break;
    case NetManager::ConnectOrInfo: {
        NetItemPrivate *item = findItem(id);
        if (item) {
            m_managerThread->connectOrInfo(id, item->itemType(), param);
        } else {
            m_managerThread->connectOrInfo(id, NetType::WirelessDeviceItem, param);
        }
    } break;
    case NetManager::ConnectInfo: {
        NetItemPrivate *item = findItem(id);
        if (item) {
            m_managerThread->getConnectInfo(id, item->itemType(), param);
        }
    } break;
    case NetManager::SetConnectInfo: {
        NetItemPrivate *item = findItem(id);
        if (item) {
            m_managerThread->setConnectInfo(id, item->itemType(), param);
        } else {
            m_managerThread->setConnectInfo(id, NetType::WirelessDeviceItem, param);
        }
    } break;
    case NetManager::DeleteConnect: {
        QString uuid = param.value("uuid").toString();
        if (!uuid.isEmpty()) {
            m_managerThread->deleteConnect(uuid);
        }
    } break;
    case NetManager::ImportConnect: {
        QString file = param.value("file").toString();
        if (QFile::exists(file)) {
            m_managerThread->importConnect(id, file);
        } else {
            qCWarning(DNC) << "Import error: file on exists:" << file;
        }
    } break;
    case NetManager::ExportConnect: {
        QString file = param.value("file").toString();
        m_managerThread->exportConnect(id, file);
    } break;
    case NetManager::ShowPage: {
        if (!id.isEmpty()) {
            m_managerThread->showPage(id);
        }
    } break;
    default:
        break;
    }
}

void NetManagerPrivate::onItemAoceand(const QString &parentID, NetItemPrivate *item)
{
    QString itemParentID = parentID;
    if (item->itemType() == NetType::NetItemType::WirelessItem) {
        NetWirelessItemPrivate *wirelessItem = NetItemPrivate::toItem<NetWirelessItemPrivate>(item);
        itemParentID = parentID + (wirelessItem->hasConnection() ? ":Mine" : ":Other");
    }
    NetItemPrivate *parentItem = findItem(itemParentID);
    if (!parentItem) {
        qCWarning(DNC) << "Item aoceand, The item's parent not found, parent id: " << parentID << ", item: " << item;
        delete NetItemPrivate::toItem<NetItemPrivate>(item);
        return;
    }
    addItem(item, parentItem);
    switch (item->itemType()) {
    case NetType::NetItemType::WirelessDeviceItem: { // 无线设备添加隐藏网络
        addItem(NetItemNew(WirelessMineItem, item->id() + ":Mine"), nullptr);
        NetWirelessOtherItemPrivate *otherItem = NetItemNew(WirelessOtherItem, item->id() + ":Other");
        addItem(otherItem, item);
        addItem(NetItemNew(WirelessHioceannItem, item->id() + ":Hioceann"), otherItem);
        ++m_deviceCount[WirelessDeviceIndex];
    } break;
    case NetType::NetItemType::WiredDeviceItem: {
        ++m_deviceCount[WiredDeviceIndex];
    } break;
    case NetType::NetItemType::WirelessItem: {
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
    NetItemPrivate *item = findItem(id);
    if (!item) {
        qCWarning(DNC) << "Item removed, item: " << id << "not find!";
        return;
    }

    NetItemPrivate *mine = nullptr;
    switch (item->itemType()) {
    case NetType::NetItemType::WirelessDeviceItem: { // 无线设备添加隐藏网络
        --m_deviceCount[WirelessDeviceIndex];
        // 我的网络和其他网络要单独删除,可能在m_dataMap中但不在列表中
        removeAndDeleteItem(findItem(item->id() + ":Mine"));
        removeAndDeleteItem(findItem(item->id() + ":Other"));
        updateControl();
    } break;
    case NetType::NetItemType::WiredDeviceItem: {
        --m_deviceCount[WiredDeviceIndex];
        updateControl();
    } break;
    case NetType::NetItemType::WirelessItem: {
        NetWirelessItemPrivate *wirelessItem = NetItemPrivate::toItem<NetWirelessItemPrivate>(item);
        if (wirelessItem->hasConnection()) {
            mine = findItem(item->getParentPrivate()->id()); // Mine
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
        NetVPNControlItemPrivate *item = NetItemPrivate::toItem<NetVPNControlItemPrivate>(findItem("NetVPNControlItem"));
        if (item) {
            NetType::NetDeviceStatus state = value.value<NetType::NetDeviceStatus>();
            const bool showTips = (state != NetType::NetDeviceStatus::DS_Connected) && (state != NetType::NetDeviceStatus::DS_Connecting);
            item->updateexpanded(showTips && item->isEnabled() && item->enabledable());
            item->updatestatus(state);
            emit vpnStatusChanged();
        }
    } break;
    case NetManagerThreadPrivate::NameChanged: {
        NetItemPrivate *item = findItem(id);
        if (item)
            item->updatename(value.toString());
    } break;
    case NetManagerThreadPrivate::EnabledChanged: {
        NetControlItemPrivate *item = NetItemPrivate::toItem<NetControlItemPrivate>(findItem(id));
        if (item) {
            item->updateenabled(value.toBool());
            updateControlEnabled(item->itemType());
        }
    } break;
    case NetManagerThreadPrivate::DeviceAvailableChanged: {
        NetControlItemPrivate *item = NetItemPrivate::toItem<NetControlItemPrivate>(findItem(id));
        if (item) {
            item->updateenabledable(value.toBool());
            if (item->itemType() == NetType::SystemProxyControlItem || item->itemType() == NetType::VPNControlItem) {
                updateItemVisible(item->id(), value.toBool());
                return;
            }
            updateControlEnabled(item->itemType());
        }
    } break;
    case NetManagerThreadPrivate::ConnectionStatusChanged: {
        NetConnectionItemPrivate *item = NetItemPrivate::toItem<NetConnectionItemPrivate>(findItem(id));
        if (item)
            item->updatestatus(value.value<NetType::NetConnectionStatus>());
    } break;
    case NetManagerThreadPrivate::WirelessStatusChanged: {
        NetWirelessItemPrivate *item = NetItemPrivate::toItem<NetWirelessItemPrivate>(findItem(id));
        if (item) {
            NetType::NetConnectionStatus state = value.value<NetType::NetConnectionStatus>();
            item->updatestatus(state);
        }
    } break;
    case NetManagerThreadPrivate::StrengthChanged: {
        NetWirelessItemPrivate *item = NetItemPrivate::toItem<NetWirelessItemPrivate>(findItem(id));
        if (item)
            item->updatestrength(value.toInt());
    } break;
    case NetManagerThreadPrivate::SecuredChanged: {
        NetWirelessItemPrivate *item = NetItemPrivate::toItem<NetWirelessItemPrivate>(findItem(id));
        if (item)
            item->updatesecure(value.toBool());
    } break;
    case NetManagerThreadPrivate::IPChanged: {
        NetDeviceItemPrivate *item = NetItemPrivate::toItem<NetDeviceItemPrivate>(findItem(id));
        if (item)
            item->updateips(value.toStringList());
    } break;
    case NetManagerThreadPrivate::DeviceStatusChanged: {
        NetDeviceItemPrivate *item = NetItemPrivate::toItem<NetDeviceItemPrivate>(findItem(id));
        if (item) {
            NetType::NetDeviceStatus deviceStatus = value.value<NetType::NetDeviceStatus>();
            if (item->status() != NetType::NetDeviceStatus::DS_IpConflicted && deviceStatus == NetType::NetDeviceStatus::DS_IpConflicted) {
                // 如果IP冲突，需要发送横幅通知
                m_managerThread->sendNotify((item->itemType() & NET_WIRED ? "notification-network-wired-local" : "notification-network-wireless-local"),
                                            tr("IP conflict"),
                                            tr("Network"),
                                            "ocean-control-center",
                                            -1,
                                            {},
                                            {},
                                            3000);
            }
            item->updatestatus(deviceStatus);
        }
    } break;
    case NetManagerThreadPrivate::HotspotEnabledChanged: {
        NetWirelessDeviceItemPrivate *item = NetItemPrivate::toItem<NetWirelessDeviceItemPrivate>(findItem(id));
        if (item)
            item->updateapMode(value.toBool());
    } break;
    case NetManagerThreadPrivate::AvailableConnectionsChanged: {
        NetWirelessDeviceItemPrivate *item = NetItemPrivate::toItem<NetWirelessDeviceItemPrivate>(findItem(id));
        if (item) {
            const QStringList &connList = value.toStringList();
            NetItemPrivate *mine = findItem(id + ":Mine");
            NetItemPrivate *other = findItem(id + ":Other");
            QVector<NetItem *> items = mine->getChildren();
            items.append(other->getChildren());
            for (auto &&item : items) {
                NetWirelessItemPrivate *wirelessItem = NetItemPrivate::toItem<NetWirelessItemPrivate>(item);
                if (wirelessItem) {
                    if (connList.contains(wirelessItem->id())) {
                        wirelessItem->updatehasConnection(true);
                        if (wirelessItem->getParentPrivate() == other) {
                            other->removeChild(wirelessItem);
                        }
                        if (wirelessItem->getParentPrivate() != mine) {
                            mine->addChild(wirelessItem);
                        }
                    } else {
                        wirelessItem->updatehasConnection(false);
                        if (wirelessItem->getParentPrivate() == mine) {
                            mine->removeChild(wirelessItem);
                        }
                        if (wirelessItem->getParentPrivate() != other) {
                            other->addChild(wirelessItem);
                        }
                    }
                }
            }
            if (!mine->getParent() && mine->getChildrenNumber() != 0) {
                findItem(id)->addChild(mine);
            } else if (mine->getParent() && mine->getChildrenNumber() == 0) {
                mine->getParentPrivate()->removeChild(mine);
            }
        }
    } break;
    case NetManagerThreadPrivate::DetailsChanged: {
        NetDetailsInfoItemPrivate *item = NetItemPrivate::toItem<NetDetailsInfoItemPrivate>(findItem(id));
        if (item)
            item->updatedetails(value.value<QList<QStringList>>());
    } break;
    case NetManagerThreadPrivate::IndexChanged: {
        NetDetailsInfoItemPrivate *item = NetItemPrivate::toItem<NetDetailsInfoItemPrivate>(findItem(id));
        if (item)
            item->updateindex(value.toInt());
    } break;
    case NetManagerThreadPrivate::ProxyMethodChanged: {
        NetSystemProxyControlItemPrivate *item = NetItemPrivate::toItem<NetSystemProxyControlItemPrivate>(findItem(id));
        if (item)
            item->updatemethod(value.value<NetType::ProxyMethod>());
    } break;
    case NetManagerThreadPrivate::SystemAutoProxyChanged: {
        NetSystemProxyControlItemPrivate *item = NetItemPrivate::toItem<NetSystemProxyControlItemPrivate>(findItem(id));
        if (item)
            item->updateautoProxy(value.toString());
    } break;
    case NetManagerThreadPrivate::SystemManualProxyChanged: {
        NetSystemProxyControlItemPrivate *item = NetItemPrivate::toItem<NetSystemProxyControlItemPrivate>(findItem(id));
        if (item)
            item->updatemanualProxy(value.value<QVariantMap>());
    } break;
    case NetManagerThreadPrivate::AppProxyChanged: {
        NetAppProxyControlItemPrivate *item = NetItemPrivate::toItem<NetAppProxyControlItemPrivate>(findItem(id));
        if (item)
            item->updateconfig(value.value<QVariantMap>());
    } break;
    case NetManagerThreadPrivate::HotspotConfigChanged: {
        NetHotspotControlItemPrivate *item = NetItemPrivate::toItem<NetHotspotControlItemPrivate>(findItem(id));
        if (item)
            item->updateconfig(value.value<QVariantMap>());
    } break;
    case NetManagerThreadPrivate::HotspotOptionalDeviceChanged: {
        NetHotspotControlItemPrivate *item = NetItemPrivate::toItem<NetHotspotControlItemPrivate>(findItem(id));
        if (item)
            item->updateoptionalDevice(value.value<QStringList>());
    } break;
    case NetManagerThreadPrivate::HotspotShareDeviceChanged: {
        NetHotspotControlItemPrivate *item = NetItemPrivate::toItem<NetHotspotControlItemPrivate>(findItem(id));
        if (item)
            item->updateshareDevice(value.value<QStringList>());
    } break;
    default:
        break;
    }
}

void NetManagerPrivate::sendRequest(NetManager::CmdType cmd, const QString &id, const QVariantMap &param)
{
    qCInfo(DNC) << "Send request, cmd: " << cmd << ", id: " << id << ", param: " << param.keys();
    Q_EMIT request(cmd, id, param);
    switch (cmd) {
    case NetManager::RequestPassword: {
        if (!m_showInputId.isEmpty())
            qCWarning(DNC) << "Untreated request password:" << m_showInputId << ", new request password: " << id;
        m_showInputId = id;
        NetWirelessItemPrivate *item = NetItemPrivate::toItem<NetWirelessItemPrivate>(findItem(id));
        if (item && !item->hasConnection()) {
            NetWirelessOtherItemPrivate *otherItem = NetItemPrivate::toItem<NetWirelessOtherItemPrivate>(findItem(item->getParentPrivate()->getParentPrivate()->id() + ":Other"));
            if (otherItem)
                otherItem->updateexpanded(true);
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
            NetItemPrivate *item = findItem(m_showInputId);
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
        items.append(m_root->item());
        while (!items.isEmpty()) {
            NetItem *item = items.takeFirst();
            switch (item->itemType()) {
            case NetType::NetItemType::WirelessItem: {
                NetWirelessItem *wirelessitem = NetItem::toItem<NetWirelessItem>(item);
                if (wirelessitem->name() == m_passwordRequestData->id) {
                    // 如果查找15次连接状态还没更新就不判断状态
                    if (m_passwordRequestData->requestCount > 15)
                        itemId = wirelessitem->id();

                    if (wirelessitem->status() == NetType::NetConnectionStatus::CS_Connecting) {
                        itemId = wirelessitem->id();
                        break;
                    }
                }
            } break;
            default:
                items.append(item->getChildren());
                break;
            }
        }
    } else {
        NetItemPrivate *devItem = findItem(m_passwordRequestData->dev);
        if (devItem) {
            switch (devItem->itemType()) {
            case NetType::NetItemType::WirelessDeviceItem: {
                QVector<NetItem *> items = devItem->getChildren();
                while (!items.isEmpty()) {
                    NetItem *item = items.takeFirst();
                    switch (item->itemType()) {
                    case NetType::NetItemType::WirelessItem: {
                        if (item->name() == m_passwordRequestData->id) {
                            itemId = item->id();
                            break;
                        }
                    } break;
                    default:
                        items.append(item->getChildren());
                        break;
                    }
                }
            } break;
            case NetType::NetItemType::WiredDeviceItem:
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
    // for (auto &&item : m_dataMap) {
    //     item->retranslateUi();
    // }
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
    NetItemPrivate *item = findItem(id);
    if (!item)
        return;

    NetType::NetItemType itemType = item->itemType();
    switch (itemType) {
    case NetType::NetItemType::WiredControlItem:
    case NetType::NetItemType::WirelessControlItem: {
        int type = (itemType & NET_MASK_TYPE) | NET_DEVICE;
        int i = m_root->getChildrenNumber();
        while (i--) {
            NetItem *item = m_root->getChild(i);
            if (item->itemType() == type) {
                setDeviceEnabled(NetItemPrivate::toItem<NetControlItemPrivate>(item), enabled);
            }
        }
    } break;
    case NetType::NetItemType::WiredDeviceItem:
    case NetType::NetItemType::WirelessDeviceItem:
    case NetType::NetItemType::VPNControlItem:
    case NetType::NetItemType::HotspotControlItem:
    case NetType::NetItemType::SystemProxyControlItem: {
        setDeviceEnabled(NetItemPrivate::toItem<NetControlItemPrivate>(item), enabled);
    } break;
    default:
        break;
    }
}

void NetManagerPrivate::setDeviceEnabled(NetControlItemPrivate *controlItem, bool enabled)
{
    if (!controlItem)
        return;

    switch (controlItem->itemType()) {
    case NetType::NetItemType::WiredControlItem:
    case NetType::NetItemType::WirelessControlItem: {
        int type = (controlItem->itemType() & NET_MASK_TYPE) | NET_DEVICE;
        int i = m_root->getChildrenNumber();
        while (i--) {
            NetItem *item = m_root->getChild(i);
            if (item->itemType() == type) {
                setDeviceEnabled(NetItemPrivate::toItem<NetControlItemPrivate>(item), enabled);
            }
        }
    } break;
    case NetType::NetItemType::WiredDeviceItem:
    case NetType::NetItemType::WirelessDeviceItem: {
        if (controlItem->enabledable() && controlItem->isEnabled() != enabled) {
            controlItem->updateenabledable(false);
            m_managerThread->setDeviceEnabled(controlItem->id(), enabled);
        }
    } break;
    case NetType::NetItemType::HotspotControlItem:
    case NetType::NetItemType::VPNControlItem:
    case NetType::NetItemType::SystemProxyControlItem: {
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
    updateControlEnabled(NetType::NetItemType::WiredDeviceItem);
    updateControlEnabled(NetType::NetItemType::WirelessDeviceItem);
}

void NetManagerPrivate::updateItemVisible(const QString &id, bool visible)
{
    NetItemPrivate *item = findItem(id);
    if (!item) {
        qCWarning(DNC) << "Update item visible " << visible << ", The item: " << id << " not find!";
        return;
    }
    if (visible) {
        if (m_deleteItem == item->getParentPrivate()) {
            m_deleteItem->removeChild(item);
            m_root->addChild(item);
        }
    } else {
        NetItemPrivate *parentItem = item->getParentPrivate();
        if (parentItem && parentItem != m_deleteItem) {
            parentItem->removeChild(item);
            m_deleteItem->addChild(item);
        }
    }
}

void NetManagerPrivate::updateControlEnabled(int type)
{
    int devType = (type & NET_MASK_TYPE) | NET_DEVICE;
    int parentType = (type & NET_MASK_TYPE) | NET_CONTROL;
    NetControlItemPrivate *parentItem = nullptr;
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
            parentItem = NetItemPrivate::toItem<NetControlItemPrivate>(item);
        }
    }
    if (parentItem) {
        parentItem->updateenabled(pEnabled);
        parentItem->updateenabledable(pEnabledable);
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
    // if (m_primaryConnectionType != type) {
    //     m_primaryConnectionType = type;
    //     Q_Q(NetManager);
    //     Q_EMIT q->primaryConnectionTypeChanged(m_primaryConnectionType);
    // }
}

void NetManagerPrivate::addItem(NetItemPrivate *item, NetItemPrivate *parentItem)
{
    NetItemPrivate *oldItem = findItem(item->id());
    if (!oldItem) {
        m_dataMap.insert(item->id(), item);
        connect(item->item(), &NetItem::destroyed, this, &NetManagerPrivate::onItemDestroyed);
    }

    if (parentItem)
        parentItem->addChild(item);
}

void NetManagerPrivate::removeItem(NetItemPrivate *item)
{
    if (!item)
        return;
    NetItemPrivate *parentItem = item->getParentPrivate();
    if (!parentItem) {
        return;
    }
    parentItem->removeChild(item);
}

void NetManagerPrivate::removeAndDeleteItem(NetItemPrivate *item)
{
    if (!item)
        return;
    removeItem(item);
    QVector<NetItem *> items;
    items.append(item->item());
    while (!items.isEmpty()) {
        NetItem *item = items.takeFirst();
        m_dataMap.remove(item->id());
        items.append(item->getChildren());
    }
    delete item;
}

} // namespace network
} // namespace ocean
