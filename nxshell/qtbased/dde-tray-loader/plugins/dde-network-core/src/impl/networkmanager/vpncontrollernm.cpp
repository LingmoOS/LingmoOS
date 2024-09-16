// SPDX-FileCopyrightText: 2018 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "vpncontrollernm.h"

#include <QDBusAbstractInterface>

#include <NetworkManagerQt/Manager>
#include <NetworkManagerQt/Settings>
#include <NetworkManagerQt/ConnectionSettings>
#include <NetworkManagerQt/VpnSetting>

#define NETWORKSERVICE "com.deepin.system.Network"
#define NETWORKPATH "/com/deepin/system/Network"
#define NETWORKINTERFACE "com.deepin.system.Network"
#define NETWORKPROPERTIESINTERFACE "org.freedesktop.DBus.Properties"
#define VPNENABLED "VpnEnabled"

using namespace dde::network;

VPNController_NM::VPNController_NM(QObject *parent)
    : VPNController (parent)
{
    initMember();
    initConnections();
}

VPNController_NM::~VPNController_NM()
{
}

void VPNController_NM::initMember()
{
    QList<VPNItem *> newItems;
    NetworkManager::Connection::List connections = NetworkManager::listConnections();
    for (NetworkManager::Connection::Ptr connection : connections) {
        VPNItem *newItem = addVpnConnection(connection);
        if (newItem)
            newItems << newItem;
    }

    sortVPNItems();

    Q_EMIT itemAdded(newItems);

    QMetaObject::invokeMethod(this, &VPNController_NM::onActiveConnectionsChanged);
}

void VPNController_NM::initConnections()
{
    connect(NetworkManager::settingsNotifier(), &NetworkManager::SettingsNotifier::connectionAdded, this, &VPNController_NM::onConnectionAdded);
    connect(NetworkManager::settingsNotifier(), &NetworkManager::SettingsNotifier::connectionRemoved, this, &VPNController_NM::onConnectionRemoved);
    connect(NetworkManager::notifier(), &NetworkManager::Notifier::activeConnectionsChanged, this, &VPNController_NM::onActiveConnectionsChanged);
    QDBusConnection::systemBus().connect(NETWORKSERVICE, NETWORKPATH, NETWORKPROPERTIESINTERFACE,
                                         "PropertiesChanged", this, SLOT(onPropertiesChanged(const QString &, const QVariantMap &)));
}

VPNItem *VPNController_NM::addVpnConnection(const NetworkManager::Connection::Ptr &connection)
{
    if (connection->settings()->connectionType() != NetworkManager::ConnectionSettings::ConnectionType::Vpn)
        return nullptr;

    auto createJson = [](const NetworkManager::Connection::Ptr &connection) {
        QJsonObject json;
        json.insert("Path", connection->path());
        json.insert("Uuid", connection->uuid());
        json.insert("Id", connection->settings()->id());
        json.insert("IfcName", connection->settings()->interfaceName());
        json.insert("HwAddress", QString());
        json.insert("ClonedAddress", QString());
        json.insert("Ssid", QString());
        json.insert("Hidden", false);
        return json;
    };
    // 从列表中查找是否存在path相同的连接
    QList<VPNItem *>::iterator itVpn = std::find_if(m_items.begin(), m_items.end(), [ connection ](VPNItem *item) {
         return (item->connection()->path() == connection->path() || item->connection()->uuid() == connection->uuid());
    });
    if (itVpn != m_items.end()) {
        VPNItem *vpnItem = *itVpn;
        vpnItem->setConnection(createJson(connection));
        return nullptr;
    }
    // 如果不存在相同的连接，则新建一个连接
    VPNItem *vpnItem = new VPNItem;
    vpnItem->setConnection(createJson(connection));
    vpnItem->updateTimeStamp(connection->settings()->timestamp());
    m_items << vpnItem;
    m_vpnConnectionsMap[vpnItem] = connection;
    connect(connection.data(), &NetworkManager::Connection::updated, vpnItem, [ connection, vpnItem, createJson, this ] {
        // 更新数据
        vpnItem->setConnection(createJson(connection));
        Q_EMIT activeConnectionChanged();
        Q_EMIT itemChanged(m_items);
    });

    return vpnItem;
}

void VPNController_NM::sortVPNItems()
{
    std::sort(m_items.begin(), m_items.end(), [](VPNItem *item1, VPNItem *item2) {
        return item1->connection()->id() < item2->connection()->id();
    });
}

QList<VPNItem *> VPNController_NM::findAutoConnectItems() const
{
    if (m_items.size() == 0)
        return QList<VPNItem *>();

    // 排序，先将最后连接的放到最前面,从列表中查找可以自动连接的
    QMap<QString, VPNItem *> vpnItemMap;
    for (VPNItem *vpnItem : m_items)
        vpnItemMap[vpnItem->connection()->path()] = vpnItem;
    QMap<QString, QList<QPair<VPNItem *, QDateTime>>> vpnItems;
    NetworkManager::Connection::List connections = NetworkManager::listConnections();
    for (NetworkManager::Connection::Ptr connection : connections) {
        // 只查找VPN的连接的类型
        if (connection->settings()->connectionType() != NetworkManager::ConnectionSettings::ConnectionType::Vpn)
            continue;

        // 过滤非自动连接的
        if (!vpnItemMap.contains(connection->path()) || !connection->settings()->autoconnect())
            continue;

        const QString &serviceType = connection->settings()->setting(NetworkManager::Setting::SettingType::Vpn).
                staticCast<NetworkManager::VpnSetting>()->serviceType();
        vpnItems[serviceType] << qMakePair(vpnItemMap[connection->path()], connection->settings()->timestamp());
    }
    // 如果没有自动连接的VPN，则返回false
    if (vpnItems.size() == 0)
        return QList<VPNItem *>();

    QList<VPNItem *> autoConnectItems;
    for (auto it = vpnItems.begin(); it != vpnItems.end(); it++) {
        QList<QPair<VPNItem *, QDateTime>> tmpItems = it.value();
        std::sort(tmpItems.begin(), tmpItems.end(), [ = ](const QPair<VPNItem *, QDateTime> &item1, const QPair<VPNItem *, QDateTime> &item2) {
            if (!item1.second.isValid() && !item2.second.isValid())
                return item1.first->connection()->id() > item2.first->connection()->id();
            if (!item1.second.isValid() && item2.second.isValid())
                return false;
            if (item1.second.isValid() && !item2.second.isValid())
                return true;

            return item1.second > item2.second;
        });
        autoConnectItems << tmpItems.first().first;
    }

    return autoConnectItems;
}

NetworkManager::ActiveConnection::List VPNController_NM::findActiveConnection() const
{
    NetworkManager::ActiveConnection::List activeConnections = NetworkManager::activeConnections();
    NetworkManager::ActiveConnection::List activeVpnConnections;
    for (NetworkManager::ActiveConnection::Ptr activeConnection : activeConnections) {
        if (activeConnection->connection()->settings()->connectionType() != NetworkManager::ConnectionSettings::ConnectionType::Vpn)
            continue;

        activeVpnConnections << activeConnection;
    }

    return activeVpnConnections;
}

NetworkManager::Connection::Ptr VPNController_NM::findConnectionByVPNItem(VPNItem *vpnItem) const
{
    NetworkManager::Connection::List connections = NetworkManager::listConnections();
    NetworkManager::Connection::List::iterator itVpn = std::find_if(connections.begin(), connections.end(), [ vpnItem, this ](NetworkManager::Connection::Ptr connection) {
        if (connection->settings()->connectionType() != NetworkManager::ConnectionSettings::ConnectionType::Vpn)
            return false;

        return vpnItem->connection()->path() == connection->path();
    });

    if (itVpn == connections.end())
        return NetworkManager::Connection::Ptr();

    return *itVpn;
}

void VPNController_NM::onConnectionAdded(const QString &path)
{
    qCInfo(DNC) << "On connection added, new vpn connection: " << path;
    NetworkManager::Connection::List connections = NetworkManager::listConnections();
    NetworkManager::Connection::List::iterator itConnection = std::find_if(connections.begin(), connections.end(), [ path ](NetworkManager::Connection::Ptr connection) {
        return connection->path() == path;
    });

    if (itConnection == connections.end())
        return;

    VPNItem *item = addVpnConnection(*itConnection);
    if (item) {
        sortVPNItems();
        Q_EMIT itemAdded({ item });
    }
}

void VPNController_NM::onConnectionRemoved(const QString &path)
{
    qCInfo(DNC) << "On connection removed, remove connection: " << path;
    for (VPNItem *item : m_items) {
        if (item->connection()->path() != path)
            continue;

        m_items.removeAll(item);
        m_vpnConnectionsMap.remove(item);
        Q_EMIT itemRemoved({ item });
        delete item;
        break;
    }
}

void VPNController_NM::onActiveConnectionsChanged()
{
    // 活动连接发生变化
    NetworkManager::ActiveConnection::List activeConnections = findActiveConnection();
    if (activeConnections.isEmpty())
        return;

    // 将所有的VPN项按照类型进行缓存
    QMap<QString, QList<VPNItem *>> vpnCategoryItems;
    for (auto it = m_vpnConnectionsMap.begin(); it != m_vpnConnectionsMap.end(); it++) {
        VPNItem *item = it.key();
        NetworkManager::Connection::Ptr itemConnection = it.value();
        if (itemConnection.isNull() || itemConnection->settings().isNull())
            continue;

        NetworkManager::Setting::Ptr setting = itemConnection->settings()->setting(NetworkManager::Setting::SettingType::Vpn);
        if (setting.isNull() || setting.dynamicCast<NetworkManager::VpnSetting>().isNull())
            continue;

        const QString serviceType = setting.dynamicCast<NetworkManager::VpnSetting>()->serviceType();
        vpnCategoryItems[serviceType] << item;
    }
    for (NetworkManager::ActiveConnection::Ptr activeConnection : activeConnections) {
        const QString &activeServiceType = activeConnection->connection()->settings()->setting(NetworkManager::Setting::SettingType::Vpn).
                staticCast<NetworkManager::VpnSetting>()->serviceType();
        if (!vpnCategoryItems.contains(activeServiceType))
            continue;

        // 监听当前活动连接的状态变化
        connect(activeConnection.data(), &NetworkManager::ActiveConnection::stateChanged, this, [ this, activeConnection ](NetworkManager::ActiveConnection::State state) {
            QList<VPNItem *>::iterator itVpnItem = std::find_if(m_items.begin(), m_items.end(), [ activeConnection ](VPNItem *item) { return item->connection()->path() == activeConnection->connection()->path(); });
            if (itVpnItem == m_items.end())
                return;

            VPNItem *activeItem = *itVpnItem;
            ConnectionStatus status = convertStateFromNetworkManager(state);
            activeItem->setConnectionStatus(status);
            if (status == ConnectionStatus::Activated) {
                activeConnection->connection()->settings()->setTimestamp(QDateTime::currentDateTime());
                activeItem->updateTimeStamp(activeConnection->connection()->settings()->timestamp());
                activeItem->setActiveConnection(activeConnection->path());
            }
            qCDebug(DNC) << "vpn connection state changed" << activeConnection->path();
            Q_EMIT activeConnectionChanged();
        });

        QList<VPNItem *> vpnItems = vpnCategoryItems[activeServiceType];
        for (VPNItem *vpnItem : vpnItems) {
            // 查找该类型的VPN活动连接，并且修改其状态
            if (vpnItem->connection()->path() == activeConnection->connection()->path()) {
                ConnectionStatus status = convertStateFromNetworkManager(activeConnection->state());
                vpnItem->setConnectionStatus(status);
                if (status == ConnectionStatus::Activated) {
                    activeConnection->connection()->settings()->setTimestamp(QDateTime::currentDateTime());
                    vpnItem->updateTimeStamp(activeConnection->connection()->settings()->timestamp());
                    vpnItem->setActiveConnection(activeConnection->path());
                }
            } else {
                vpnItem->setConnectionStatus(ConnectionStatus::Deactivated);
            }
        }
    }
}

void VPNController_NM::onPropertiesChanged(const QString &interfaceName, const QVariantMap &changedProperties)
{
    if (interfaceName != NETWORKINTERFACE)
        return;

    if (!changedProperties.contains(VPNENABLED))
        return;

    Q_EMIT enableChanged(changedProperties.value(VPNENABLED).toBool());
}

void VPNController_NM::setEnabled(const bool enabled)
{
    // 调用com.deepin.system.Network的相关接口来设置VPN
    QDBusInterface dbusInter(NETWORKSERVICE, NETWORKPATH, NETWORKINTERFACE, QDBusConnection::systemBus());
    dbusInter.setProperty(VPNENABLED, enabled);
    // 如果开启VPN，则让其自动连接
    if (!enabled)
        return;

    // 查找最近一次自动连接的VPN
    QList<VPNItem *> autoConnectionItems = findAutoConnectItems();
    for (VPNItem *autoConnectionItem : autoConnectionItems) {
        connectItem(autoConnectionItem);
    }
}

bool VPNController_NM::enabled() const
{
    QDBusInterface dbusInter(NETWORKSERVICE, NETWORKPATH, NETWORKINTERFACE, QDBusConnection::systemBus());
    return dbusInter.property(VPNENABLED).toBool();
}

void VPNController_NM::connectItem(VPNItem *item)
{
    if (!item)
        return;

    qCInfo(DNC) << QString("connect to Vpn:%1, path:%2").arg(item->connection()->id()).arg(item->connection()->path());
    // 查看当前已经连接的VPN
    NetworkManager::Connection::Ptr connection = findConnectionByVPNItem(item);
    if (connection.isNull()) {
        NetworkManager::activateConnection(item->connection()->path(), "/", "/");
        return;
    }

    const QString &serviceType = connection->settings()->setting(NetworkManager::Setting::SettingType::Vpn).
            staticCast<NetworkManager::VpnSetting>()->serviceType();
    // 如果是相同类型的连接，则只允许连接一个，在连接之前需要先把之前的已经连接的同类型的连接断开
    NetworkManager::ActiveConnection::Ptr sameTypeActiveConnection;
    NetworkManager::ActiveConnection::List activeConnections = findActiveConnection();
    for (NetworkManager::ActiveConnection::Ptr activeConnection : activeConnections) {
        if (connection->path() != activeConnection->connection()->path()
                && activeConnection->connection()->settings()->setting(NetworkManager::Setting::SettingType::Vpn).
                staticCast<NetworkManager::VpnSetting>()->serviceType() == serviceType) {

            sameTypeActiveConnection = activeConnection;
            break;
        }
    }
    if (sameTypeActiveConnection.isNull()) {
        NetworkManager::activateConnection(item->connection()->path(), "/", "/");
    } else {
        QDBusPendingCallWatcher *w = new QDBusPendingCallWatcher(NetworkManager::deactivateConnection(sameTypeActiveConnection->path()), this);
        connect(w, &QDBusPendingCallWatcher::finished, w, &QDBusPendingCallWatcher::deleteLater);
        connect(w, &QDBusPendingCallWatcher::finished, item, [ item ] {
            // 需要等断开连接的操作执行完毕后，才能执行连接操作，否则会出现连接失败的情况
            NetworkManager::activateConnection(item->connection()->path(), "/", "/");
        });
    }
}

void VPNController_NM::connectItem(const QString &uuid)
{
    QList<VPNItem *>::iterator itVpn = std::find_if(m_items.begin(), m_items.end(), [ uuid ](VPNItem *item) { return item->connection()->uuid() == uuid; });
    if (itVpn != m_items.end()) {
        qCInfo(DNC) << "connect connection, uuid:" << uuid;
        connectItem(*itVpn);
    } else {
        qCInfo(DNC) << "count found vpn item, uuid:" << uuid;
    }
}

void VPNController_NM::disconnectItem()
{
    NetworkManager::ActiveConnection::List activeConnections = findActiveConnection();
    for (NetworkManager::ActiveConnection::Ptr activeConnection : activeConnections) {
        qCInfo(DNC) << "disconnect vpn item:" << activeConnection->path();
        NetworkManager::deactivateConnection(activeConnection->path());
    }
}

QList<VPNItem *> VPNController_NM::items() const
{
    return m_items;
}
