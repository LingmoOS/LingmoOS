// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "deviceinterrealize.h"
#include "networkmanagerqt/settings.h"
#include "wireddevice.h"
#include "wirelessdevice.h"
#include "accesspointsproxyinter.h"

#include <QHostAddress>

#include <NetworkManagerQt/WiredDevice>
#include <NetworkManagerQt/WirelessSetting>
#include <NetworkManagerQt/Manager>

using namespace dde::network;

class AccessPointInfo
{
public:
    AccessPointInfo(const QJsonObject &json, const QString &path)
        : m_apProxy(new AccessPointsProxyInter(json, path))
        , m_accessPoint(new AccessPoints(m_apProxy))
    {
    }

    AccessPointInfo(const AccessPointInfo &apInfo)
        : m_apProxy(apInfo.m_apProxy)
        , m_accessPoint(apInfo.m_accessPoint)
    {
    }

    const AccessPointInfo &operator=(const AccessPointInfo &apInfo)
    {
        this->m_apProxy = apInfo.m_apProxy;
        this->m_accessPoint = apInfo.m_accessPoint;
        return *this;
    }

    ~AccessPointInfo()
    {
        delete m_apProxy;
        delete m_accessPoint;
    }

    AccessPointsProxyInter *proxy() const
    {
        return m_apProxy;
    }

    AccessPoints *accessPoint() const
    {
        return m_accessPoint;
    }

    QString ssid() const
    {
        return m_apProxy->ssid();
    }

private:
    AccessPointsProxyInter *m_apProxy;
    AccessPoints *m_accessPoint;
};

#define UNKNOW_MODE 0
#define ADHOC_MODE  1
#define INFRA_MODE 2
#define AP_MODE 3

#define ACTIVATING 1
#define ACTIVATED 2
#define DEACTIVATING 3
#define DEACTIVATED 4

const QStringList DeviceInterRealize::ipv4()
{
    if (!isConnected() || !isEnabled())
        return QStringList();

    if (m_activeInfoData.contains("IPv4")) {
        QJsonObject ipv4TopObject = m_activeInfoData["IPv4"].toObject();
        QJsonArray ipv4Array = ipv4TopObject.value("Addresses").toArray();
        QStringList ipv4s;
        for (const QJsonValue ipv4Value : ipv4Array) {
            const QJsonObject ipv4Object = ipv4Value.toObject();
            QString ip = ipv4Object.value("Address").toString();
            ip = ip.remove("\"");
            ipv4s << ip;
        }
        ipv4s = getValidIPV4(ipv4s);
        return ipv4s;
    }

    // 返回IPv4地址
    QJsonValue ipJsonData = m_activeInfoData["Ip4"];
    QJsonObject objIpv4 = ipJsonData.toObject();
    return { objIpv4.value("Address").toString() };
}

const QStringList DeviceInterRealize::ipv6()
{
    if (!isConnected() || !isEnabled() || !m_activeInfoData.contains("Ip6"))
        return QStringList();

    if (m_activeInfoData.contains("IPv6")) {
        QJsonObject ipv6TopObject = m_activeInfoData["IPv6"].toObject();
        QJsonArray ipv6Array = ipv6TopObject.value("Addresses").toArray();
        QStringList ipv6s;
        for (const QJsonValue ipv6Value : ipv6Array) {
            const QJsonObject ipv6Object = ipv6Value.toObject();
            QString ip = ipv6Object.value("Address").toString();
            ip = ip.remove("\"");
            ipv6s << ip;
        }
        return ipv6s;
    }

    // 返回IPv4地址
    QJsonValue ipJsonData = m_activeInfoData["Ip6"];
    QJsonObject objIpv6 = ipJsonData.toObject();
    return { objIpv6.value("Address").toString() };
}

QJsonObject DeviceInterRealize::activeConnectionInfo() const
{
    return m_activeInfoData;
}

QStringList DeviceInterRealize::getValidIPV4(const QStringList &ipv4s)
{
    if (ipv4s.size() > 1 || ipv4s.size() == 0)
        return ipv4s;

    // 检查IP列表，如果发现有IP为0.0.0.0，则让其重新获取一次，保证IP获取正确
    // 这种情况一般发生在关闭热点后，因此在此处处理
    if (isIpv4Address(ipv4s[0]))
        return ipv4s;

    QDBusPendingReply<QString> reply = m_networkInter->GetActiveConnectionInfo();
    const QString activeConnInfo = reply.value();
    QJsonParseError error;
    QJsonDocument json = QJsonDocument::fromJson(activeConnInfo.toUtf8(), &error);
    if (error.error != QJsonParseError::NoError)
        return ipv4s;

    if (!json.isArray())
        return ipv4s;

    QJsonArray infoArray = json.array();
    for (const QJsonValue ipInfo : infoArray) {
        const QJsonObject ipObject = ipInfo.toObject();
        if (ipObject.value("Device").toString() != this->path())
            continue;

        if (!ipObject.contains("IPv4"))
            return ipv4s;

        QJsonObject ipV4Object = ipObject.value("IPv4").toObject();
        if (!ipV4Object.contains("Addresses"))
            return ipv4s;

        QStringList ipAddresses;
        QJsonArray ipv4Addresses = ipV4Object.value("Addresses").toArray();
        for (const QJsonValue addr : ipv4Addresses) {
            const QJsonObject addressObject = addr.toObject();
            QString ip = addressObject.value("Address").toString();
            if (isIpv4Address(ip))
                ipAddresses << ip;
        }
        if (ipAddresses.size() > 0) {
            m_activeInfoData = ipObject;
            return ipAddresses;
        }
    }

    return ipv4s;
}

bool DeviceInterRealize::isIpv4Address(const QString &ip) const
{
    QHostAddress ipAddr(ip);
    if (ipAddr == QHostAddress(QHostAddress::Null) || ipAddr == QHostAddress(QHostAddress::AnyIPv4)
            || ipAddr.protocol() != QAbstractSocket::NetworkLayerProtocol::IPv4Protocol) {
        return false;
    }

    QRegExp regExpIP("((25[0-5]|2[0-4][0-9]|1[0-9][0-9]|[1-9][0-9]|[0-9])[\\.]){3}(25[0-5]|2[0-4][0-9]|1[0-9][0-9]|[1-9][0-9]|[0-9])");
    return regExpIP.exactMatch(ip);
}

bool DeviceInterRealize::isConnected() const
{
    NetworkManager::Device::Ptr device = NetworkManager::findNetworkInterface(path());
    if (!device)
        return false;

    return device->state() == NetworkManager::Device::State::Activated;
}

void DeviceInterRealize::setEnabled(bool enabled)
{
    m_networkInter->EnableDevice(QDBusObjectPath(path()), enabled);
}

Connectivity DeviceInterRealize::connectivity()
{
    return m_connectivity;
}

DeviceStatus DeviceInterRealize::deviceStatus() const
{
    if (ipConflicted())
        return DeviceStatus::IpConflict;

    if (mode() == AP_MODE || !available())
        return DeviceStatus::Disconnected;

    NetworkManager::Device::Ptr dev = NetworkManager::findNetworkInterface(path());
    if (dev.isNull()) {
        dev.reset(new NetworkManager::Device(path()));
    }
    if (dev.isNull())
        return DeviceStatus::Unknown;

    switch(dev->state()) {
    case NetworkManager::Device::State::UnknownState:
        return DeviceStatus::Unknown;
    case NetworkManager::Device::State::Unmanaged:
        return DeviceStatus::Unmanaged;
    case NetworkManager::Device::State::Unavailable:
        return DeviceStatus::Unavailable;
    case NetworkManager::Device::State::Disconnected:
        return DeviceStatus::Disconnected;
    case NetworkManager::Device::State::Preparing:
        return DeviceStatus::Prepare;
    case NetworkManager::Device::State::ConfiguringHardware:
    case NetworkManager::Device::State::ConfiguringIp:
        return DeviceStatus::Config;
    case NetworkManager::Device::State::NeedAuth:
        return DeviceStatus::Needauth;
    case NetworkManager::Device::State::CheckingIp:
        return DeviceStatus::IpCheck;
    case NetworkManager::Device::State::WaitingForSecondaries:
        return DeviceStatus::Secondaries;
    case NetworkManager::Device::State::Activated:
        return DeviceStatus::Activated;
    case NetworkManager::Device::State::Deactivating:
        return DeviceStatus::Deactivation;
    case NetworkManager::Device::State::Failed:
        return DeviceStatus::Failed;
    }

    return DeviceStatus::Unknown;
}

DeviceInterRealize::DeviceInterRealize(NetworkInter *networkInter, QObject *parent)
    : NetworkDeviceRealize(parent)
    , m_networkInter(networkInter)
    , m_enabled(true)
    , m_connectivity(Connectivity::Full)
{
}

DeviceInterRealize::~DeviceInterRealize()
{
}

NetworkInter *DeviceInterRealize::networkInter()
{
    return m_networkInter;
}

void DeviceInterRealize::updateDeviceInfo(const QJsonObject &info)
{
    m_data = info;
    NetworkManager::Device::List devices = NetworkManager::networkInterfaces();
    auto itDevices = std::find_if(devices.begin(), devices.end(), [ this ](NetworkManager::Device::Ptr device) {
        return device->uni() == this->path();
    });
    if (itDevices != devices.end() && m_device != *itDevices) {
        if (m_device) {
            disconnect(m_device.data(), nullptr, this, nullptr);
        }
        m_device = *itDevices;
        connect(m_device.data(), &NetworkManager::Device::stateChanged, this, [ this ] {
            NetworkDeviceRealize::setDeviceStatus(deviceStatus());
        });
        onActiveConnectionChanged();
        connect(m_device.data(), &NetworkManager::Device::activeConnectionChanged, this, &DeviceInterRealize::onActiveConnectionChanged);
    }
    updateDeviceData(info);
    NetworkDeviceRealize::setDeviceStatus(deviceStatus());
}

void DeviceInterRealize::initDeviceInfo()
{
    if (m_networkInter) {
        // 状态发生变化后，获取设备的实时信息
        QDBusPendingReply<bool> netEnabled = m_networkInter->IsDeviceEnabled(QDBusObjectPath(path()));
        m_enabled = netEnabled.value();
    }
}

void DeviceInterRealize::setDeviceEnabledStatus(const bool &enabled)
{
    m_enabled = enabled;
    Q_EMIT enableChanged(enabled);
}

void DeviceInterRealize::updateActiveConnectionInfo(const QList<QJsonObject> &infos)
{
    const QStringList oldIpv4 = ipv4();
    m_activeInfoData = QJsonObject();
    for (const QJsonObject &info : infos) {
        if (info.value("ConnectionType").toString() == deviceKey()) {
            // 如果找到了当前硬件地址的连接信息，则直接使用这个数据
            m_activeInfoData = info;
            break;
        }
    }

    // 获取到完整的IP地址后，向外发送连接改变的信号
    if (!m_activeInfoData.isEmpty())
        Q_EMIT connectionChanged();

    QStringList ipv4s = ipv4();
    bool ipChanged = false;
    if (oldIpv4.size() != ipv4s.size()) {
        ipChanged = true;
    } else {
        for (const QString &ip : ipv4s) {
            if (!oldIpv4.contains(ip)) {
                ipChanged = true;
                break;
            }
        }
    }
    if (ipChanged)
        Q_EMIT ipV4Changed();
}

void DeviceInterRealize::setDeviceStatus(const DeviceStatus &status)
{
    Q_UNUSED(status);
    NetworkDeviceRealize::setDeviceStatus(deviceStatus());
}

int DeviceInterRealize::mode() const
{
    if (m_data.contains("Mode"))
        return m_data.value("Mode").toInt();

    return UNKNOW_MODE;
}

void DeviceInterRealize::onActiveConnectionChanged()
{
    if (m_device.isNull())
        return;

    NetworkManager::ActiveConnection::Ptr activeConnection = m_device->activeConnection();
    if (activeConnection.isNull() || activeConnection == m_activeConnection)
        return;

    if (m_activeConnection)
        disconnect(activeConnection.data(), nullptr, this, nullptr);

    updateActiveStatus(activeConnection->connection(), convertStateFromNetworkManager(activeConnection->state()));
    connect(activeConnection.data(), &NetworkManager::ActiveConnection::stateChanged, this, [ activeConnection, this ](NetworkManager::ActiveConnection::State state) {
        if (state == NetworkManager::ActiveConnection::State::Activated) {
            NetworkManager::Connection::Ptr  conn = activeConnection->connection();
            const NetworkManager::Setting::SettingType settingType[] = { NetworkManager::Setting::Security8021x, NetworkManager::Setting::WirelessSecurity };
            for (auto type : settingType) {
                NetworkManager::Setting::Ptr setting = conn->settings()->setting(type);
                if (setting) {
                    conn->secrets(setting->name());
                }
            }
            conn->save();
            connect(conn.data(), &NetworkManager::Connection::unsavedChanged, this, [this] {
                    Q_EMIT activeConnectionChanged();
            });
        }
        updateActiveStatus(activeConnection->connection(), convertStateFromNetworkManager(state));
    });
}

/**
 * @brief 有线设备类的具体实现
 */

WiredDeviceInterRealize::WiredDeviceInterRealize(NetworkInter *networkInter, QObject *parent)
    : DeviceInterRealize(networkInter, parent)
{
}

WiredDeviceInterRealize::~WiredDeviceInterRealize()
{
    for (auto item : m_connections) {
        delete item;
    }
    m_connections.clear();
}

bool WiredDeviceInterRealize::connectNetwork(WiredConnection *connection)
{
    if (!connection)
        return false;

    qCDebug(DNC) << "connection ssid:" << connection->connection()->ssid();
    networkInter()->ActivateConnection(connection->connection()->uuid(), QDBusObjectPath(path()));
    return true;
}

void WiredDeviceInterRealize::disconnectNetwork()
{
    qCInfo(DNC) << "Disconnected Network";
    networkInter()->DisconnectDevice(QDBusObjectPath(path()));
}

QList<WiredConnection *> WiredDeviceInterRealize::wiredItems() const
{
    return m_connections;
}

bool WiredDeviceInterRealize::carrier() const
{
    if (m_wiredDevice) {
        return m_wiredDevice->carrier();
    }
    return deviceStatus() > DeviceStatus::Unavailable;
}

WiredConnection *WiredDeviceInterRealize::findConnection(const QString &path)
{
    for (WiredConnection *conn : m_connections) {
        if (conn->connection()->path() == path)
            return conn;
    }

    return Q_NULLPTR;
}

void WiredDeviceInterRealize::setDeviceEnabledStatus(const bool &enabled)
{
    if (!enabled) {
        // 禁用网卡的情况下，先清空连接信息
        for (WiredConnection *connection : m_connections)
            connection->setConnectionStatus(ConnectionStatus::Deactivated);
    }

    DeviceInterRealize::setDeviceEnabledStatus(enabled);

    emit ipV4Changed();
}

void WiredDeviceInterRealize::updateActiveStatus(const QSharedPointer<NetworkManager::Connection> &connection, ConnectionStatus state)
{
    for (WiredConnection *conn : m_connections) {
        conn->setConnectionStatus(conn->connection()->uuid() == connection->uuid() ? state : ConnectionStatus::Deactivated);
    }

    emit activeConnectionChanged();
    emit ipV4Changed();
}

void WiredDeviceInterRealize::updateDeviceData(const QJsonObject &data)
{
    Q_UNUSED(data);
    if (m_device.isNull())
        return;

    m_wiredDevice = m_device.dynamicCast<NetworkManager::WiredDevice>();
    if (m_wiredDevice.isNull())
        return;

    connect(m_wiredDevice.data(), &NetworkManager::WiredDevice::carrierChanged, this, &WiredDeviceInterRealize::carrierChanged, Qt::UniqueConnection);
}

void WiredDeviceInterRealize::updateConnection(const QJsonArray &info)
{
    QList<WiredConnection *> newWiredConnections;
    QList<WiredConnection *> changedWiredConnections;
    QStringList connPaths;
    for (const QJsonValue &jsonValue : info) {
        const QJsonObject &jsonObj = jsonValue.toObject();
        const QString IfcName = jsonObj.value("IfcName").toString();
        if (!IfcName.isEmpty() && IfcName != interface())
            continue;

        const QString path = jsonObj.value("Path").toString();
        WiredConnection *conn = findConnection(path);
        if (!conn) {
            conn = new WiredConnection;
            m_connections << conn;
            newWiredConnections << conn;
            qCDebug(DNC) << "new connection ssid: " << jsonObj.value("Ssid").toString();
        } else {
            if (conn->connection()->id() != jsonObj.value("Id").toString()
                    || conn->connection()->ssid() != jsonObj.value("Ssid").toString())
                changedWiredConnections << conn;
        }

        conn->setConnection(jsonObj);
        if (!connPaths.contains(path))
            connPaths << path;
    }

    QList<WiredConnection *> rmConns;
    for (WiredConnection *connection : m_connections) {
        if (!connPaths.contains(connection->connection()->path()))
            rmConns << connection;
    }

    for (WiredConnection *connection : rmConns)
        m_connections.removeOne(connection);

    if (changedWiredConnections.size())
        Q_EMIT connectionPropertyChanged(changedWiredConnections);

    if (newWiredConnections.size() > 0)
        Q_EMIT connectionAdded(newWiredConnections);

    if (rmConns.size() > 0)
        Q_EMIT connectionRemoved(rmConns);

    // 提交改变信号后，删除不在的连接
    for (WiredConnection *connection : rmConns)
        delete connection;

    // 排序
    sortWiredItem(m_connections);
}

WiredConnection *WiredDeviceInterRealize::findWiredConnectionByUuid(const QString &uuid)
{
    for (WiredConnection *connection : m_connections) {
        if (connection->connection()->uuid() == uuid)
            return connection;
    }

    return Q_NULLPTR;
}

static ConnectionStatus convertStatus(int status)
{
    if (status == ACTIVATING)
        return ConnectionStatus::Activating;

    if (status == ACTIVATED)
        return ConnectionStatus::Activated;

    if (status == DEACTIVATING)
        return ConnectionStatus::Deactivating;

    if (status == DEACTIVATED)
        return ConnectionStatus::Deactivated;

    return ConnectionStatus::Unknown;
}

void WiredDeviceInterRealize::updateActiveInfo(const QList<QJsonObject> &info)
{
    bool changeStatus = false;
    // 根据返回的UUID找到对应的连接，找到State=2的连接变成连接成功状态
    for (const QJsonObject &activeInfo : info) {
        const QString uuid = activeInfo.value("Uuid").toString();
        WiredConnection *connection = findWiredConnectionByUuid(uuid);
        if (!connection)
            continue;

        ConnectionStatus status = convertStatus(activeInfo.value("State").toInt());
        if (connection->status() != status) {
            connection->setConnectionStatus(status);
            changeStatus = true;
        }
    }

    // 调用基类的函数，更改设备的状态，同时向外发送信号
    DeviceInterRealize::updateActiveInfo(info);
    if (changeStatus)
        Q_EMIT activeConnectionChanged();

    emit ipV4Changed();
}

QString WiredDeviceInterRealize::deviceKey()
{
    return "wired";
}

/**
 * @brief WirelessDeviceInterRealize::WirelessDeviceInterRealize
 * @param networkInter
 * @param parent
 */

QList<AccessPoints *> WirelessDeviceInterRealize::accessPointItems() const
{
    if (needShowAccessPoints()) {
        QList<AccessPoints *> apItems;
        for (AccessPointInfo *apItem : m_accessPointInfos)
            apItems << apItem->accessPoint();

        return apItems;
    }

    return QList<AccessPoints *>();
}

void WirelessDeviceInterRealize::scanNetwork()
{
    networkInter()->RequestWirelessScan();
}

void WirelessDeviceInterRealize::connectNetwork(const AccessPoints *item)
{
    WirelessConnection *wirelessConn = findConnectionByAccessPoint(item);
    if (!wirelessConn)
        return;

    const QString uuid = wirelessConn->connection()->uuid();
    const QString apPath = item->path();
    const QString devPath = path();

    qCDebug(DNC) << "connect Network:" << item->ssid();

    QDBusPendingCallWatcher *w = new QDBusPendingCallWatcher(networkInter()->ActivateAccessPoint(uuid, QDBusObjectPath(apPath), QDBusObjectPath(devPath)));

    connect(w, &QDBusPendingCallWatcher::finished, this, [ = ](QDBusPendingCallWatcher * wSelf) {
        QDBusPendingReply<QDBusObjectPath> reply = *wSelf;

        if (reply.value().path().isEmpty()) {
            // 连接失败
            qCInfo(DNC) << "Failure";
            Q_EMIT connectionFailed(item);
            Q_EMIT deviceStatusChanged(DeviceStatus::Failed);
        } else {
            qCInfo(DNC) << "Success";
            Q_EMIT deviceStatusChanged(DeviceStatus::Activated);
        }
        w->deleteLater();
    });
}

AccessPoints *WirelessDeviceInterRealize::activeAccessPoints() const
{
    // 如果网卡是关闭的状态下，肯定是没有连接
    if (!isEnabled())
        return Q_NULLPTR;

    for (AccessPointInfo *ap : m_accessPointInfos) {
        if (ap->accessPoint()->connected())
            return ap->accessPoint();
    }

    return Q_NULLPTR;
}

bool WirelessDeviceInterRealize::available() const
{
    return m_available;
}

void WirelessDeviceInterRealize::disconnectNetwork()
{
    // 使用DeactivateConnection而不是DisconnectDevice，是为了能在断开当前网络后仍能自动回连其他热点
    WirelessConnection *wirelessConn = findConnectionByAccessPoint(activeAccessPoints());
    if (!wirelessConn)
        return;

    const QString uuid = wirelessConn->connection()->uuid();
    networkInter()->DeactivateConnection(uuid);
}

QList<WirelessConnection *> WirelessDeviceInterRealize::items() const
{
    QList<WirelessConnection *> lstItems;
    for (WirelessConnection *item : m_connections) {
        if (item->accessPoints())
            lstItems << item;
    }

    return lstItems;
}

WirelessDeviceInterRealize::WirelessDeviceInterRealize(NetworkInter *networkInter, QObject *parent)
    : DeviceInterRealize(networkInter, parent)
    , m_available(true)
{
}

WirelessDeviceInterRealize::~WirelessDeviceInterRealize()
{
    clearListData(m_accessPointInfos);
    clearListData(m_connections);
}

WirelessConnection *WirelessDeviceInterRealize::findConnectionByPath(const QString &path)
{
    for (WirelessConnection *conn : m_connections) {
        if (conn->connection()->path() == path)
            return conn;
    }

    return Q_NULLPTR;
}

AccessPointInfo *WirelessDeviceInterRealize::findAccessPoint(const QString &ssid)
{
    for (AccessPointInfo *accessPoint : m_accessPointInfos) {
        if (accessPoint->ssid() == ssid)
            return accessPoint;
    }

    return Q_NULLPTR;
}

WirelessConnection *WirelessDeviceInterRealize::findConnectionByAccessPoint(const AccessPoints *accessPoint)
{
    if (!accessPoint)
        return Q_NULLPTR;

    for (WirelessConnection *connection : m_connections) {
        if (connection->accessPoints() == accessPoint)
            return connection;

        if (connection->connection()->ssid() == accessPoint->ssid())
            return connection;
    }

    return Q_NULLPTR;
}

/**
 * @brief 同步热点和连接的信息
 */
void WirelessDeviceInterRealize::syncConnectionAccessPoints()
{
    if (m_accessPointInfos.isEmpty()) {
        clearListData(m_connections);
        qCDebug(DNC) << "can't found accesspoint";
        return;
    }

    QList<WirelessConnection *> connections;
    QList<WirelessConnection *> newConnection;
    // 找到每个热点对应的Connection，并将其赋值
    for (AccessPointInfo *accessPoint : m_accessPointInfos) {
        WirelessConnection *connection = findConnectionByAccessPoint(accessPoint->accessPoint());
        if (!connection) {
            connection = WirelessConnection::createConnection(accessPoint->accessPoint());
            m_connections << connection;
            newConnection << connection;
        }

        connection->m_accessPoints = accessPoint->accessPoint();
        connections << connection;
    }
    updateActiveInfo();
    // 删除列表中没有AccessPoints的Connection，让两边保持数据一致
    QList<WirelessConnection *> rmConns;
    for (WirelessConnection *connection : m_connections) {
        if (!connections.contains(connection))
            rmConns << connection;
    }
    if (!rmConns.isEmpty()) {
        Q_EMIT wirelessConnectionRemoved(rmConns);
    }

    for (WirelessConnection *rmConnection : rmConns) {
        m_connections.removeOne(rmConnection);
        delete rmConnection;
    }
    if (!newConnection.isEmpty()) {
        Q_EMIT wirelessConnectionAdded(newConnection);
    }
}

void WirelessDeviceInterRealize::updateActiveInfo()
{
    if (m_activeAccessPoints.isEmpty())
        return;

    QString activeSsid;
    ConnectionStatus activeStatus = ConnectionStatus::Unknown;
    if (!m_device.isNull()) {
        NetworkManager::ActiveConnection::Ptr activeConnection = m_device->activeConnection();
        if (!activeConnection.isNull()) {
            NetworkManager::WirelessSetting::Ptr wsSetting = activeConnection->connection()->settings()->setting(NetworkManager::Setting::SettingType::Wireless)
                    .dynamicCast<NetworkManager::WirelessSetting>();
            if (!wsSetting.isNull()) {
                activeSsid = wsSetting->ssid();
                activeStatus = convertStateFromNetworkManager(activeConnection->state());
            }
        }
    }
    QList<AccessPointInfo *> tmpApList = m_accessPointInfos;
    // 遍历活动连接列表，找到对应的wlan，改变其连接状态，State赋值即可
    bool changed = false;
    AccessPoints *activeAp = Q_NULLPTR;
    for (const QJsonObject &aapInfo : m_activeAccessPoints) {
        int connectionStatus = aapInfo.value("State").toInt();
        QString ssid = aapInfo.value("Id").toString();
        AccessPointInfo *ap = findAccessPoint(ssid);
        if (!ap)
            continue;

        tmpApList.removeAll(ap);
        ConnectionStatus status = convertConnectionStatus(connectionStatus);
        if (!ssid.isEmpty() && activeStatus != ConnectionStatus::Unknown) {
            // 如果当前活动的ssid不为空，则获取NM中的活动连接的状态
            if (ssid == activeSsid)
                status = activeStatus;
            else
                status = ConnectionStatus::Deactivated;
        }
        if (ap->accessPoint()->status() == status)
            continue;

        ap->proxy()->updateConnectionStatus(status);
        changed = true;
        if (ap->accessPoint()->status() == ConnectionStatus::Activated)
            activeAp = ap->accessPoint();
    }

    // 将其它连接变成普通状态
    for (AccessPointInfo *ap : tmpApList)
        ap->proxy()->updateConnectionStatus(ConnectionStatus::Unknown);

    if (changed) {
        qCDebug(DNC) << "accessPoint Status Changed";
        Q_EMIT activeConnectionChanged();
    }

    // 如果发现其中一个连接成功，将这个连接成功的信号移到最上面，然后则向外发送连接成功的信号
    if (activeAp) {
        AccessPointInfo *activeApInfo = nullptr;
        for (AccessPointInfo *info : m_accessPointInfos) {
            if (info->accessPoint() == activeAp) {
                m_accessPointInfos.removeAll(info);
                activeApInfo = info;
                break;
            }
        }
        if (activeApInfo) {
            m_accessPointInfos << activeApInfo;
            Q_EMIT connectionSuccess(activeAp);
        }
    }

    // 调用基类的方法触发连接发生变化，同时向外抛出连接变化的信号
    DeviceInterRealize::updateActiveInfo(m_activeAccessPoints);
}

QList<WirelessConnection *> WirelessDeviceInterRealize::wirelessItems() const
{
    return m_connections;
}

bool WirelessDeviceInterRealize::needShowAccessPoints() const
{
    if (mode() == AP_MODE || !m_available)
        return false;

    // 如果当前设备热点为空(关闭热点)，则让显示所有的网络列表
    return m_hotspotInfo.isEmpty();
}

void WirelessDeviceInterRealize::updateActiveConnectionInfo(const QList<QJsonObject> &infos)
{
    bool enabledHotspotOld = hotspotEnabled();

    m_hotspotInfo = QJsonObject();
    for (const QJsonObject &info : infos) {
        const QString devicePath = info.value("Device").toString();
        const QString connectionType = info.value("ConnectionType").toString();
        if (devicePath == this->path() && connectionType == "wireless-hotspot") {
            m_hotspotInfo = info;
            setDeviceStatus(DeviceStatus::Disconnected);
            break;
        }
    }

    bool enabledHotspot = hotspotEnabled();
    if (enabledHotspotOld != enabledHotspot)
        Q_EMIT hotspotEnableChanged(enabledHotspot);

    DeviceInterRealize::updateActiveConnectionInfo(infos);
}

bool dde::network::WirelessDeviceInterRealize::hotspotEnabled()
{
    return !m_hotspotInfo.isEmpty();
}

void WirelessDeviceInterRealize::updateDeviceData(const QJsonObject &data)
{
    if (data.value("InterfaceFlags").isUndefined())
        return;

    bool available = (data.value("InterfaceFlags").toInt() & DEVICE_INTERFACE_FLAG_UP);
    if (available == m_available)
        return;

    m_available = available;
    emit availableChanged(m_available);
}

void WirelessDeviceInterRealize::updateActiveStatus(const QSharedPointer<NetworkManager::Connection> &connection, ConnectionStatus state)
{
    NetworkManager::WirelessSetting::Ptr wsSetting = connection->settings()->setting(NetworkManager::Setting::SettingType::Wireless).dynamicCast<NetworkManager::WirelessSetting>();
    if (wsSetting.isNull())
        return;

    QString ssid = wsSetting->ssid();
    for (AccessPointInfo *accessPoint : m_accessPointInfos) {
        accessPoint->proxy()->updateConnectionStatus(accessPoint->ssid() == ssid ? state : ConnectionStatus::Deactivated);
    }
    emit activeConnectionChanged();
}

void WirelessDeviceInterRealize::updateAccesspoint(const QJsonArray &json)
{
    auto isWifi6 = [](const QJsonObject &json) {
        if (json.contains("Flags")) {
            int flag = json.value("Flags").toInt();
            if (flag & AP_FLAGS_HE)
                return true;
        }

        return false;
    };

    // 先过滤相同的ssid，找出信号强度最大的那个
    QMap<QString, int> ssidMaxStrength;
    QMap<QString, QString> ssidPath;
    QMap<QString, int> wifi6Ssids;
    for (const QJsonValue &jsonValue : json) {
        const QJsonObject obj = jsonValue.toObject();
        const QString ssid = obj.value("Ssid").toString();
        const int strength = obj.value("Strength").toInt();
        const QString path = obj.value("Path").toString();
        if (ssidMaxStrength.contains(ssid)) {
            const int nOldStrength = ssidMaxStrength.value(ssid);
            if (nOldStrength < strength) {
                // 找到了对应的热点，更新热点的信号强度
                ssidMaxStrength[ssid] = strength;
                ssidPath[ssid] = path;
            }
        } else {
            // 第一次直接插入SSID和信号强度和路径
            ssidMaxStrength[ssid] = strength;
            ssidPath[ssid] = path;
        }
        if (isWifi6(obj))
            wifi6Ssids[ssid] = obj.value("Flags").toInt();
    }

    QList<AccessPoints *> newAp;
    QList<AccessPoints *> changedAp;
    QStringList ssids;
    for (const QJsonValue &jsonValue : json) {
        QJsonObject accessInfo = jsonValue.toObject();
        const QString ssid = accessInfo.value("Ssid").toString();
        const QString maxSsidPath = ssidPath.value(ssid);
        const QString path = accessInfo.value("Path").toString();
        if (path != maxSsidPath)
            continue;

        // 如果当前的SSID存在WiFi6,就让其显示WiFi6的属性
        if (wifi6Ssids.contains(ssid))
            accessInfo["extendFlags"] = wifi6Ssids[ssid];

        // 从网络列表中查找现有的网络
        AccessPointInfo *accessPoint = findAccessPoint(ssid);
        if (!accessPoint) {
            // 如果没有找到这个网络，就新建一个网络，添加到网络列表
            AccessPointInfo *apInfo = new AccessPointInfo(accessInfo, this->path());
            m_accessPointInfos << apInfo;
            newAp << apInfo->accessPoint();
        } else {
            int strength = accessInfo.value("Strength").toInt();
            if (accessPoint->accessPoint()->strength() != strength)
                changedAp << accessPoint->accessPoint();

            accessPoint->proxy()->updateAccessPoints(accessInfo);
        }

        if (!ssids.contains(ssid))
            ssids << ssid;
    }

    if (changedAp.size())
        Q_EMIT accessPointInfoChanged(changedAp);

    if (newAp.size() > 0)
        Q_EMIT networkAdded(newAp);

    // 更新网络和连接的关系
    QList<AccessPointInfo *> rmAccessPoints;
    for (AccessPointInfo *ap : m_accessPointInfos) {
        if (!ssids.contains(ap->ssid()))
            rmAccessPoints << ap;
    }

    if (rmAccessPoints.size() > 0) {
        QList<AccessPoints *> rmApItems;
        for (AccessPointInfo *ap : rmAccessPoints) {
            m_accessPointInfos.removeOne(ap);
            rmApItems << ap->accessPoint();
        }

        Q_EMIT networkRemoved(rmApItems);
    }

    for (AccessPointInfo *ap : rmAccessPoints)
        delete ap;

    createConnection(m_connectionJson);
    syncConnectionAccessPoints();
}

void WirelessDeviceInterRealize::setDeviceEnabledStatus(const bool &enabled)
{
    if (!enabled) {
        // 禁用网卡的情况下，直接清空原来的连接信息
        m_activeAccessPoints.clear();
        // 这里暂时不清空AccessPoints列表，防止再打开网卡的时候重复创建
    }

    DeviceInterRealize::setDeviceEnabledStatus(enabled);
}

void WirelessDeviceInterRealize::updateConnection(const QJsonArray &info)
{
    m_connectionJson = info;

    createConnection(info);

    syncConnectionAccessPoints();
}

void WirelessDeviceInterRealize::createConnection(const QJsonArray &info)
{
    QStringList connPaths;
    for (const QJsonValue &jsonValue : info) {
        const QJsonObject &jsonObj = jsonValue.toObject();
        const QString hwAddress = jsonObj.value("HwAddress").toString();
        if (!hwAddress.isEmpty() && hwAddress != realHwAdr())
            continue;

        const QString path = jsonObj.value("Path").toString();
        WirelessConnection *connection = findConnectionByPath(path);
        if (!connection) {
            connection = new WirelessConnection;
            auto conPtr = NetworkManager::findConnection(path);
            if (!conPtr.isNull()) {
                connect(conPtr.data(), &NetworkManager::Connection::updated, this, [ this, connection] {
                    Q_EMIT wirelessConnectionPropertyChanged( { connection } );
                });
            }
            m_connections << connection;
        }

        connection->setConnection(jsonObj);
        if (!connPaths.contains(path))
            connPaths << path;
    }

    QList<WirelessConnection *> rmConns;
    for (WirelessConnection *conn : m_connections) {
        if (!connPaths.contains(conn->connection()->path()))
            rmConns << conn;
    }

    // 提交改变信号后，删除不在的连接
    for (WirelessConnection *conn : rmConns) {
        m_connections.removeOne(conn);
        delete conn;
    }
}

void WirelessDeviceInterRealize::updateActiveInfo(const QList<QJsonObject> &info)
{
    m_activeAccessPoints = info;
    updateActiveInfo();
}

QString WirelessDeviceInterRealize::deviceKey()
{
    return "wireless";
}
