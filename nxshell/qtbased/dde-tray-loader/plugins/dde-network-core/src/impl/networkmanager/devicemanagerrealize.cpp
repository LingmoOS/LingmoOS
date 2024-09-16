// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "devicemanagerrealize.h"
#include "netutils.h"
#include "networkdevicebase.h"
#include "networkmanagerprocesser.h"
#include "accesspointproxynm.h"
#include "configsetting.h"

#include <networkmanagerqt/wirelessdevice.h>
#include <networkmanagerqt/wireddevice.h>
#include <networkmanagerqt/manager.h>
#include <networkmanagerqt/wiredsetting.h>
#include <NetworkManagerQt/Security8021xSetting>
#include <NetworkManagerQt/WirelessSecuritySetting>
#include <NetworkManagerQt/WirelessSetting>
#include <NetworkManagerQt/Settings>
#include <NetworkManagerQt/IpConfig>

#include <wireddevice.h>
#include <wirelessdevice.h>

#include <QMetaEnum>

#include <stdlib.h>
#include <string.h>
#include <libudev.h>

using namespace dde::network;

using NetworkInter = com::deepin::daemon::Network;

const static QString networkService = "com.deepin.daemon.Network";
const static QString networkPath = "/com/deepin/daemon/Network";

#define MANUAL 1

class AccessPointInfo
{
public:
    AccessPointInfo(NetworkManager::WirelessDevice::Ptr device, NetworkManager::WirelessNetwork::Ptr network)
        : m_proxy(new AccessPointProxyNM(device, network))
        , m_accessPoint(new AccessPoints(m_proxy))
    {
    }

    AccessPointInfo(const AccessPointInfo &apInfo)
        : m_proxy(apInfo.m_proxy)
        , m_accessPoint(apInfo.m_accessPoint)
    {
    }

    const AccessPointInfo &operator=(const AccessPointInfo &apInfo)
    {
        this->m_proxy = apInfo.m_proxy;
        this->m_accessPoint = apInfo.m_accessPoint;
        return *this;
    }

    ~AccessPointInfo()
    {
        delete m_accessPoint;
        delete m_proxy;
    }

    AccessPointProxyNM *proxy() const
    {
        return m_proxy;
    }

    AccessPoints *accessPoint() const
    {
        return m_accessPoint;
    }

private:
    AccessPointProxyNM *m_proxy;
    AccessPoints *m_accessPoint;
};

#define SYS_NETWORK_INTER "com.deepin.system.Network"
#define SYS_NETWORK_PATH "/com/deepin/system/Network"

DeviceManagerRealize::DeviceManagerRealize(NetworkManager::Device::Ptr device, QObject *parent)
    : NetworkDeviceRealize(parent)
    , m_device(device)
    , m_isUsbDevice(false)
    , m_isEnabeld(true)
{
    // 初始化信号槽的连接
    initConnection();
    // 获取是否USB设备
    initUsbInfo();
    // 获取是否启用或者禁用
    initEnabeld();
    // 初始化所有的连接，此处需要异步执行，因为在构造函数中当前子类的对象尚未初始化完成，如果同步执行addConnection的虚函数，调用的还是基类的函数
    QMetaObject::invokeMethod(this, [ this ] {
        // 初始化设备的状态，这个放到异步来执行，因为会获取DeviceStatus，而这个deviceStatus在无线网络子类中有实现
        setDeviceStatus(deviceStatus());
        // 获取当前设备上所有的连接
        QStringList deviceConnectionPath;
        NetworkManager::Connection::List connections = m_device->availableConnections();
        // 拔掉网线后，仍然要显示有线项
        if (m_device->type() == NetworkManager::Device::Type::Ethernet && connections.isEmpty() && deviceStatus() == DeviceStatus::Unavailable) {
            updateWiredConnections();
        } else {
            for (NetworkManager::Connection::Ptr connection : connections) {
                addConnection(connection);
                deviceConnectionPath << connection->path();
            }
        }

        if (m_device->type() == NetworkManager::Device::Type::Wifi) {
            // 获取所有的连接,因为有一些网络连接在更改加密方式后,这个连接会脱离当前的设备,因此此处需要重新加载其他的连接
            connections = NetworkManager::listConnections();
            for (NetworkManager::Connection::Ptr connection : connections) {
                if (deviceConnectionPath.contains(connection->path()))
                    continue;
                NetworkManager::WirelessSetting::Ptr wirelessSetting = connection->settings()->setting(NetworkManager::Setting::SettingType::Wireless).dynamicCast<NetworkManager::WirelessSetting>();
                if (wirelessSetting.isNull())
                    continue;

                // 此处只过滤当前设备的interfaceName相同或者MAC地址相同的连接
                if (connection->settings()->interfaceName() != m_device->interfaceName()
                        && wirelessSetting->macAddress().toHex().toUpper() != realHwAdr().remove(":"))
                    continue;

                addConnection(connection);
            }
        }
        // 获取连接的状态
        onActiveConnectionChanged();
        QDBusInterface dbus("org.freedesktop.NetworkManager", m_device->uni(), "org.freedesktop.NetworkManager.Device", QDBusConnection::systemBus());
        resetConfig(dbus.property("Ip4Config").value<QDBusObjectPath>());
    }, Qt::QueuedConnection);
}

void DeviceManagerRealize::updateWiredConnections()
{
    for (NetworkManager::Connection::Ptr connection : NetworkManager::listConnections()) {
        NetworkManager::WiredSetting::Ptr wiredSetting = connection->settings()->setting(NetworkManager::Setting::SettingType::Wired).dynamicCast<NetworkManager::WiredSetting>();
        if (wiredSetting.isNull())
            continue;
        // 此处只过滤当前设备的interfaceName相同或者MAC地址相同的连接
        if (connection->settings()->interfaceName() != m_device->interfaceName()
                && wiredSetting->macAddress().toHex().toUpper() != realHwAdr().remove(":"))
            continue;
        addConnection(connection);
    }
}

void DeviceManagerRealize::initConnection()
{
    connect(m_device.data(), &NetworkManager::Device::availableConnectionAppeared, this, &DeviceManagerRealize::onConnectionAdded);
    connect(m_device.data(), &NetworkManager::Device::availableConnectionDisappeared, this, &DeviceManagerRealize::removeConnection);
    connect(m_device.data(), &NetworkManager::Device::activeConnectionChanged, this, &DeviceManagerRealize::onActiveConnectionChanged);
    connect(m_device.data(), &NetworkManager::Device::stateChanged, this, &DeviceManagerRealize::onDeviceStateChanged);
    connect(m_device.data(), &NetworkManager::Device::ipV4AddressChanged, this, &DeviceManagerRealize::ipV4Changed);
    connect(m_device.data(), &NetworkManager::Device::dhcp4ConfigChanged, this, &DeviceManagerRealize::ipV4Changed);
    QDBusConnection::systemBus().connect(SYS_NETWORK_INTER, SYS_NETWORK_PATH, SYS_NETWORK_INTER, "DeviceEnabled", this, SLOT(onDeviceEnabledChanged(QDBusObjectPath, bool)));
    connect(NetworkManager::settingsNotifier(), &NetworkManager::SettingsNotifier::connectionRemoved, this, &DeviceManagerRealize::removeConnection);
    connect(NetworkManager::settingsNotifier(), &NetworkManager::SettingsNotifier::connectionAdded, this, &DeviceManagerRealize::onConnectionAdded);
    QDBusConnection::systemBus().connect("org.freedesktop.NetworkManager", m_device->uni(), "org.freedesktop.DBus.Properties", "PropertiesChanged",
                                         this, SLOT(onDevicePropertiesChanged(const QString &, const QVariantMap &, const QStringList &)));
}

void DeviceManagerRealize::initUsbInfo()
{
    m_isUsbDevice = false;
    struct udev *udev = udev_new();
    if (!udev)
        return;

    struct udev_device *device = udev_device_new_from_syspath(udev, m_device->udi().toStdString().c_str());
    if (!device) {
        udev_unref(udev);
        return;
    }

    const char *id_bus = udev_device_get_property_value(device, "ID_BUS");
    if (id_bus && strncmp(id_bus, "usb", strlen(id_bus)) == 0)
        m_isUsbDevice = true;

    udev_device_unref(device);
    udev_unref(udev);
}

void DeviceManagerRealize::initEnabeld()
{
    QDBusInterface dbusInter(SYS_NETWORK_INTER, SYS_NETWORK_PATH, SYS_NETWORK_INTER, QDBusConnection::systemBus());
    QDBusPendingCall reply = dbusInter.asyncCall("IsDeviceEnabled", m_device->uni());
    reply.waitForFinished();
    QDBusPendingReply<bool> replyResult = reply.reply();
    m_isEnabeld = replyResult.argumentAt(0).toBool();
}

void DeviceManagerRealize::resetConfig(const QDBusObjectPath &ipv4ConfigPath)
{
    m_ipv4Config.reset(new IpConfig(m_device->ipV4Config(), ipv4ConfigPath.path()));
    connect(m_ipv4Config.data(), &IpConfig::ipChanged, this, &DeviceManagerRealize::ipV4Changed);
}

DeviceManagerRealize::~DeviceManagerRealize()
{
}

bool DeviceManagerRealize::isEnabled() const
{
    return m_isEnabeld;
}

QString DeviceManagerRealize::interface() const
{
    return m_device->interfaceName();
}

QString DeviceManagerRealize::driver() const
{
    return m_device->driver();
}

bool DeviceManagerRealize::managed() const
{
    return m_device->managed();
}

QString DeviceManagerRealize::vendor() const
{
    return QString();
}

QString DeviceManagerRealize::uniqueUuid() const
{
    return m_device->udi();
}

bool DeviceManagerRealize::usbDevice() const
{
    return m_isUsbDevice;
}

QString DeviceManagerRealize::path() const
{
    return m_device->uni();
}

QString DeviceManagerRealize::activeAp() const
{
    return QString();
}

bool DeviceManagerRealize::supportHotspot() const
{
    return false;
}

QString DeviceManagerRealize::usingHwAdr() const
{
    return QString();
}

const QStringList DeviceManagerRealize::ipv4()
{
    if (!device())
        return QStringList();

    NetworkManager::IpAddresses ipv4AddrList = m_device->ipV4Config().addresses();
    QStringList ipv4s;
    for (const NetworkManager::IpAddress &ipAddr : ipv4AddrList)
        ipv4s << ipAddr.ip().toString();

    return ipv4s;
}

const QStringList DeviceManagerRealize::ipv6()
{
    if (!device() || !device()->isConnected() || !isEnabled())
        return QStringList();

    NetworkManager::IpAddresses ipv6AddrList = m_device->ipV6Config().addresses();
    QStringList ipv6s;
    for (const NetworkManager::IpAddress &ipAddr : ipv6AddrList)
        ipv6s << ipAddr.ip().toString();

    return ipv6s;
}

QJsonObject DeviceManagerRealize::activeConnectionInfo() const
{
    return QJsonObject();
}

void DeviceManagerRealize::setEnabled(bool enabled)
{
    bool currentEnabled = isEnabled();
    if (currentEnabled == enabled)
        return;

    qCDebug(DNC) << QString("set Device %1, enabled: %2").arg(m_device->uni()).arg(enabled ? "true" : "false");
    QDBusInterface dbusInter(SYS_NETWORK_INTER, SYS_NETWORK_PATH, SYS_NETWORK_INTER, QDBusConnection::systemBus());
    QDBusReply<QDBusObjectPath> reply = dbusInter.call("EnableDevice", m_device->uni(), enabled);
    deviceEnabledAction(reply, enabled);
    if (enabled) {
        m_device->setAutoconnect(true);
    }
}

void DeviceManagerRealize::disconnectNetwork()
{
    NetworkManager::ActiveConnection::Ptr activeConnection = m_device->activeConnection();
    if (!activeConnection.isNull()) {
        NetworkManager::deactivateConnection(activeConnection->path());
        qCDebug(DNC) << "disconnect network" << activeConnection->path() << ",device:" << m_device->interfaceName();
    }
}

DeviceStatus DeviceManagerRealize::deviceStatus() const
{
    switch(m_device->state()) {
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

void DeviceManagerRealize::onDeviceEnabledChanged(QDBusObjectPath path, bool enabled)
{
    if (m_device && m_device->uni() == path.path()) {
        m_isEnabeld = enabled;
        deviceEnabledChanged(enabled);
        Q_EMIT enableChanged(enabled);
    }
}

void DeviceManagerRealize::onConnectionAdded(const QString &connectionUni)
{
    NetworkManager::Connection::List connections = m_device->availableConnections();
    NetworkManager::Connection::List::iterator itConnection = std::find_if(connections.begin(), connections.end(), [ connectionUni ](NetworkManager::Connection::Ptr connection) {
        return connection->path() == connectionUni;
    });

    if (itConnection == connections.end())
        return;

    addConnection(*itConnection);
}

void DeviceManagerRealize::onDeviceStateChanged()
{
    setDeviceStatus(deviceStatus());
}

void DeviceManagerRealize::onDevicePropertiesChanged(const QString &interfaceName, const QVariantMap &changedProperties, const QStringList &invalidatedProperties)
{
    Q_UNUSED(invalidatedProperties);

    if (!interfaceName.startsWith("org.freedesktop.NetworkManager.Device"))
        return;

    if (changedProperties.contains("Ip4Config")) {
        QDBusObjectPath ipv4ConfigPath = changedProperties.value("Ip4Config").value<QDBusObjectPath>();
        resetConfig(ipv4ConfigPath);
    }
}

static QJsonObject createJson(NetworkManager::Device::Ptr device, NetworkManager::Connection::Ptr connection)
{
    QJsonObject json;
    json.insert("Path", connection->path());
    json.insert("Uuid", connection->uuid());
    json.insert("Id", connection->settings()->id());
    json.insert("IfcName", connection->settings()->interfaceName());
    if(device->type() == NetworkManager::Device::Type::Ethernet) {
        NetworkManager::WiredDevice::Ptr wiredDevice = device.staticCast<NetworkManager::WiredDevice>();
        json.insert("HwAddress", wiredDevice->permanentHardwareAddress());
        json.insert("Ssid", QString());
    } else if (device->type() == NetworkManager::Device::Type::Wifi) {
        NetworkManager::WirelessDevice::Ptr wirelessDevice = device.staticCast<NetworkManager::WirelessDevice>();
        json.insert("HwAddress", wirelessDevice->permanentHardwareAddress());
        NetworkManager::WirelessSetting::Ptr wsSetting = connection->settings()->setting(NetworkManager::Setting::Wireless).dynamicCast<NetworkManager::WirelessSetting>();
        if (wsSetting.isNull())
            json.insert("Ssid", connection->settings()->id());
        else
            json.insert("Ssid", QString(wsSetting->ssid()));
    }

    json.insert("Hidden", false);
    return json;
}

// 有线网卡的相关的连接的数据变化
WiredDeviceManagerRealize::WiredDeviceManagerRealize(NetworkManager::WiredDevice::Ptr device, QObject *parent)
    : DeviceManagerRealize(device, parent)
    , m_device(device)
{
    connect(m_device.data(), &NetworkManager::WiredDevice::carrierChanged, this, &DeviceManagerRealize::carrierChanged);
}

WiredDeviceManagerRealize::~WiredDeviceManagerRealize()
{
}

QString WiredDeviceManagerRealize::realHwAdr() const
{
    return (m_device->permanentHardwareAddress().toUpper());
}

bool WiredDeviceManagerRealize::connectNetwork(WiredConnection *connection)
{
    if (!connection)
        return false;

    if (!isEnabled()) {
        setEnabled(true);
    }
    // 拔掉网线后，通过后端接口连接，弹通知
    if (deviceStatus() == DeviceStatus::Unavailable) {
        NetworkInter inter(networkService, networkPath, QDBusConnection::sessionBus(), this);
        inter.ActivateConnection(connection->connection()->uuid(), QDBusObjectPath(path()));
        return false;
    }
    QVariantMap options;
    options["flags"] = MANUAL;
    NetworkManager::activateConnection2(connection->connection()->path(), m_device->uni(), "", options);
    return true;
}

QList<WiredConnection *> WiredDeviceManagerRealize::wiredItems() const
{
    return m_wiredConnections;
}

void WiredDeviceManagerRealize::addConnection(const NetworkManager::Connection::Ptr &connection)
{
    if (connection->settings()->connectionType() != NetworkManager::ConnectionSettings::ConnectionType::Wired)
        return;

    QList<WiredConnection *>::iterator itItem = std::find_if(m_wiredConnections.begin(), m_wiredConnections.end(), [ connection ](WiredConnection *item) {
        return item->connection()->path() == connection->path();
    });

    WiredConnection *wiredConnection = nullptr;
    // 如果存在连接，更新，如果不存在连接，则新增
    if (itItem == m_wiredConnections.end()) {
        wiredConnection = new WiredConnection;
        wiredConnection->setConnection(createJson(m_device, connection));
        m_wiredConnections << wiredConnection;
        Q_EMIT connectionAdded( { wiredConnection } );
    } else {
        wiredConnection = *itItem;
        wiredConnection->setConnection(createJson(m_device, connection));
    }

    // 监听连接信息变化的信号
    connect(connection.data(), &NetworkManager::Connection::updated, this, [ this, connection ] {
        if (!m_device.isNull() && !connection.isNull()) {
            QList<WiredConnection *>::iterator itItem = std::find_if(m_wiredConnections.begin(), m_wiredConnections.end(), [ connection ](WiredConnection *item) {
                return item->connection()->path() == connection->path();
            });
            if (itItem != m_wiredConnections.end()) {
                WiredConnection *wiredConnection = *itItem;
                wiredConnection->setConnection(createJson(m_device, connection));
                Q_EMIT connectionPropertyChanged( { wiredConnection } );
            }
        }
    });
}

void WiredDeviceManagerRealize::removeConnection(const QString &connectionUni)
{
    QList<WiredConnection *> removeWiredConnection;
    for (WiredConnection *wiredConnection : m_wiredConnections) {
        if (wiredConnection->connection()->path() != connectionUni)
            continue;

        removeWiredConnection << wiredConnection;
    }

    if (removeWiredConnection.size() == 0)
        return;

    // 从现有列表中移除当前的连接
    for (WiredConnection *wiredConnection : removeWiredConnection) {
        m_wiredConnections.removeOne(wiredConnection);
    }

    Q_EMIT connectionRemoved(removeWiredConnection);

    // 释放内存
    for (WiredConnection *wiredConnection : removeWiredConnection) {
        delete wiredConnection;
    }

    // 刷新拔出网线后的有线项
    updateWiredConnections();
}

void WiredDeviceManagerRealize::onActiveConnectionChanged()
{
    NetworkManager::ActiveConnection::Ptr activeConnection = m_device->activeConnection();
    // 先将所有的当前的连接状态置为断开连接
    WiredConnection *activeWiredConnection = nullptr;
    for (WiredConnection *connection : m_wiredConnections) {
        if (activeConnection && connection->connection()->path() == activeConnection->connection()->path()) {
            activeWiredConnection = connection;
        } else {
            connection->setConnectionStatus(ConnectionStatus::Deactivated);
        }
    }

    // 监听当前的活动连接的状态的变化
    if (!activeWiredConnection)
        return;

    qCDebug(DNC) << QString("wiredDevice: %1, active connection id:%2, path:%3").arg(m_device->interfaceName()).arg(activeWiredConnection->connection()->id()).arg(activeWiredConnection->connection()->path());
    connect(activeConnection.data(), &NetworkManager::ActiveConnection::stateChanged, this, [ this, activeConnection ](NetworkManager::ActiveConnection::State state) {
        auto itConnection = std::find_if(m_wiredConnections.begin(), m_wiredConnections.end(), [ activeConnection ](WiredConnection *item) { return item->connection()->path() == activeConnection->connection()->path(); });
        if (itConnection == m_wiredConnections.end())
            return;

        WiredConnection *activeConnection = *itConnection;
        activeConnection->setConnectionStatus(convertStateFromNetworkManager(state));
        qCDebug(DNC) << "connection:" << activeConnection->connection()->id() << "state:" << activeConnection->status();
        Q_EMIT activeConnectionChanged();
    });
    connect(activeConnection.data(), &NetworkManager::ActiveConnection::default4Changed, this, &WiredDeviceManagerRealize::ipV4Changed, Qt::UniqueConnection);
    connect(activeConnection.data(), &NetworkManager::ActiveConnection::dhcp4ConfigChanged, this, &WiredDeviceManagerRealize::ipV4Changed, Qt::UniqueConnection);
    connect(activeConnection.data(), &NetworkManager::ActiveConnection::ipV4ConfigChanged, this, &WiredDeviceManagerRealize::ipV4Changed, Qt::UniqueConnection);

    activeWiredConnection->setConnectionStatus(convertStateFromNetworkManager(activeConnection->state()));
    Q_EMIT activeConnectionChanged();
}

void WiredDeviceManagerRealize::deviceEnabledAction(const QDBusReply<QDBusObjectPath> &reply, bool enabled)
{
    if (!enabled)
        return;

    // 如果是开启，则让其自动连接
    QString activeConnectionPath = reply.value().path();
    // 开启自动连接才需要回连
    bool isAutoConnect = false;
    for (const auto connection : m_device->availableConnections()) {
        if (activeConnectionPath == connection->path()) {
            isAutoConnect = connection->settings()->autoconnect();
        }
    }
    if (isAutoConnect) {
        NetworkManager::activateConnection(activeConnectionPath, m_device->uni(), QString());
        qCDebug(DNC) << "connected:" << activeConnectionPath;
    }
}

QString WiredDeviceManagerRealize::usingHwAdr() const
{
    return m_device->hardwareAddress().toUpper();
}

bool WiredDeviceManagerRealize::carrier() const
{
    return m_device->carrier();
}

/**
 * @brief WirelessDeviceManagerRealize::WirelessDeviceManagerRealize
 * @param ipChecker
 * @param device
 * @param parent
 * 无线网络设备
 */
WirelessDeviceManagerRealize::WirelessDeviceManagerRealize(NetworkManager::WirelessDevice::Ptr device, QObject *parent)
    : DeviceManagerRealize(device, parent)
    , m_device(device)
    , m_hotspotEnabled(getHotspotIsEnabled())
    , m_netProcesser(nullptr)
    , m_available(device->interfaceFlags() & DEVICE_INTERFACE_FLAG_UP)
{
    connect(device.data(), &NetworkManager::WirelessDevice::networkAppeared, this, &WirelessDeviceManagerRealize::onNetworkAppeared);
    connect(device.data(), &NetworkManager::WirelessDevice::networkDisappeared, this, &WirelessDeviceManagerRealize::onNetworkDisappeared);
    connect(device.data(), &NetworkManager::WirelessDevice::interfaceFlagsChanged, this, &WirelessDeviceManagerRealize::onInterfaceFlagsChanged);

    connect(device.data(), &NetworkManager::WirelessDevice::modeChanged, this, [ this ] {
        bool oldEnabled = m_hotspotEnabled;
        m_hotspotEnabled = getHotspotIsEnabled();
        qCInfo(DNC) << QString("wirelessModel changed, old hotspotEnabled:") << oldEnabled << "new hotspotEnabled: " << m_hotspotEnabled;
        if (m_hotspotEnabled)
            setDeviceStatus(DeviceStatus::Disconnected);
        if (oldEnabled != m_hotspotEnabled)
            Q_EMIT hotspotEnableChanged(m_hotspotEnabled);
    }, Qt::QueuedConnection);

    NetworkManager::WirelessNetwork::List networks = m_device->networks();
    for (NetworkManager::WirelessNetwork::Ptr network : networks)
        addNetwork(network);
}

WirelessDeviceManagerRealize::~WirelessDeviceManagerRealize()
{
    m_netProcesser = nullptr;
    for(WirelessConnection *wirelessConnection : m_wirelessConnections)
        delete wirelessConnection;

    for (AccessPointInfo *apInfo : m_accessPointInfos)
        delete apInfo ;
}

void WirelessDeviceManagerRealize::addProcesser(ProcesserInterface *processer)
{
    m_netProcesser = processer;
}

NetworkManager::WirelessSecuritySetting::KeyMgmt WirelessDeviceManagerRealize::getKeyMgmtByAp(const NetworkManager::AccessPoint::Ptr &accessPoint)
{
    if (accessPoint.isNull())
        return NetworkManager::WirelessSecuritySetting::WpaPsk;

    NetworkManager::AccessPoint::Capabilities capabilities = accessPoint->capabilities();
    NetworkManager::AccessPoint::WpaFlags wpaFlags = accessPoint->wpaFlags();
    NetworkManager::AccessPoint::WpaFlags rsnFlags = accessPoint->rsnFlags();

    NetworkManager::WirelessSecuritySetting::KeyMgmt keyMgmt = NetworkManager::WirelessSecuritySetting::KeyMgmt::WpaNone;

    if (capabilities.testFlag(NetworkManager::AccessPoint::Capability::Privacy) && !wpaFlags.testFlag(NetworkManager::AccessPoint::WpaFlag::KeyMgmtPsk) && !wpaFlags.testFlag(NetworkManager::AccessPoint::WpaFlag::KeyMgmt8021x)) {
        keyMgmt = NetworkManager::WirelessSecuritySetting::KeyMgmt::Wep;
    }

    if (wpaFlags.testFlag(NetworkManager::AccessPoint::WpaFlag::KeyMgmtSAE) || rsnFlags.testFlag(NetworkManager::AccessPoint::WpaFlag::KeyMgmtSAE)) {
        keyMgmt = NetworkManager::WirelessSecuritySetting::KeyMgmt::SAE;
    }

    if (wpaFlags.testFlag(NetworkManager::AccessPoint::WpaFlag::KeyMgmtPsk) || rsnFlags.testFlag(NetworkManager::AccessPoint::WpaFlag::KeyMgmtPsk)) {
        keyMgmt = NetworkManager::WirelessSecuritySetting::KeyMgmt::WpaPsk;
    }

    if (wpaFlags.testFlag(NetworkManager::AccessPoint::WpaFlag::KeyMgmt8021x) || rsnFlags.testFlag(NetworkManager::AccessPoint::WpaFlag::KeyMgmt8021x)) {
        keyMgmt = NetworkManager::WirelessSecuritySetting::KeyMgmt::WpaEap;
    }

    return keyMgmt;
}

bool WirelessDeviceManagerRealize::checkKeyMgmt(const NetworkManager::AccessPoint::Ptr &accessPoint, NetworkManager::WirelessSecuritySetting::KeyMgmt keyMgmt)
{
    if (accessPoint.isNull())
        return keyMgmt == NetworkManager::WirelessSecuritySetting::WpaPsk;

    NetworkManager::AccessPoint::Capabilities capabilities = accessPoint->capabilities();
    NetworkManager::AccessPoint::WpaFlags wpaFlags = accessPoint->wpaFlags();
    NetworkManager::AccessPoint::WpaFlags rsnFlags = accessPoint->rsnFlags();

    if ((capabilities.testFlag(NetworkManager::AccessPoint::Capability::Privacy) && !wpaFlags.testFlag(NetworkManager::AccessPoint::WpaFlag::KeyMgmtPsk)
         && !wpaFlags.testFlag(NetworkManager::AccessPoint::WpaFlag::KeyMgmt8021x))
        && keyMgmt == NetworkManager::WirelessSecuritySetting::KeyMgmt::Wep) {
        return true;
    }

    if ((wpaFlags.testFlag(NetworkManager::AccessPoint::WpaFlag::KeyMgmtSAE) || rsnFlags.testFlag(NetworkManager::AccessPoint::WpaFlag::KeyMgmtSAE))
        && keyMgmt == NetworkManager::WirelessSecuritySetting::KeyMgmt::SAE) {
        return true;
    }

    if ((wpaFlags.testFlag(NetworkManager::AccessPoint::WpaFlag::KeyMgmtPsk) || rsnFlags.testFlag(NetworkManager::AccessPoint::WpaFlag::KeyMgmtPsk))
        && keyMgmt == NetworkManager::WirelessSecuritySetting::KeyMgmt::WpaPsk) {
        return true;
    }

    if ((wpaFlags.testFlag(NetworkManager::AccessPoint::WpaFlag::KeyMgmt8021x) || rsnFlags.testFlag(NetworkManager::AccessPoint::WpaFlag::KeyMgmt8021x))
        && keyMgmt == NetworkManager::WirelessSecuritySetting::KeyMgmt::WpaEap) {
        return true;
    }

    return keyMgmt == NetworkManager::WirelessSecuritySetting::KeyMgmt::WpaNone;
}

bool WirelessDeviceManagerRealize::available() const
{
    return m_available;
}

bool WirelessDeviceManagerRealize::supportHotspot() const
{
    return (m_device->wirelessCapabilities().testFlag(NetworkManager::WirelessDevice::ApCap));
}

QString WirelessDeviceManagerRealize::activeAp() const
{
    NetworkManager::WirelessDevice::Ptr wirelessDevice = m_device.staticCast<NetworkManager::WirelessDevice>();
    NetworkManager::AccessPoint::Ptr ap = wirelessDevice->activeAccessPoint();
    if (ap)
        return ap->ssid();

    return QString();
}

QString WirelessDeviceManagerRealize::realHwAdr() const
{
    return (m_device->permanentHardwareAddress().toUpper());
}

QList<AccessPoints *> WirelessDeviceManagerRealize::accessPointItems() const
{
    // 如果开启了热点，就直接返回空的网络列表
    if (m_hotspotEnabled)
        return QList<AccessPoints *>();

    QList<AccessPoints *> accessPoints;
    for (AccessPointInfo *apInfo : m_accessPointInfos)
        accessPoints << apInfo->accessPoint();

    return accessPoints;
}

void WirelessDeviceManagerRealize::scanNetwork()
{
    m_device->requestScan();
    qCInfo(DNC) << "device:" << m_device->interfaceName() << "scan network";
}

void WirelessDeviceManagerRealize::connectNetwork(const AccessPoints *accessPoint)
{
    if (!accessPoint)
        return;

    NetworkManager::Connection::Ptr currentConnection;
    NetworkManager::Connection::List availableConnections = m_device->availableConnections();
    NetworkManager::Connection::List::iterator const itConnection =
            std::find_if(availableConnections.begin(), availableConnections.end(), [accessPoint](const NetworkManager::Connection::Ptr &connection) {
                NetworkManager::WirelessSetting::Ptr wSetting = connection->settings()->setting(NetworkManager::Setting::SettingType::Wireless).dynamicCast<NetworkManager::WirelessSetting>();
                if (wSetting.isNull())
                    return false;

                return wSetting->ssid() == accessPoint->ssid();
            });

    if (itConnection == availableConnections.end()) {
        NetworkManager::Connection::List const connections = NetworkManager::listConnections();
        for (NetworkManager::Connection::Ptr const& connection : connections) {
            if (availableConnections.contains(connection)) {
                continue;
            }
            NetworkManager::WirelessSetting::Ptr const wSetting = connection->settings()->setting(NetworkManager::Setting::SettingType::Wireless).dynamicCast<NetworkManager::WirelessSetting>();
            if (wSetting.isNull()) {
                continue;
            }

            // 如果Mac地址不为空并且不是当前设备的mac地址
            const QString configMacAddress = QString(wSetting->macAddress().toHex()).toUpper();
            if (!configMacAddress.isEmpty() && configMacAddress != realHwAdr().remove(":"))
                continue;

            if (wSetting->ssid() == accessPoint->ssid()) {
                currentConnection = connection;
                break;
            }
        }
    } else {
        currentConnection = *itConnection;
    }
    NetworkManager::WirelessSecuritySetting::KeyMgmt const keyMgmt = getKeyMgmtByAp(m_device->findAccessPoint(accessPoint->path()));
    if (currentConnection.isNull()) {
        // 如果是新的连接，则调用AddAndActivateConnection接口来实现
        NetworkManager::ConnectionSettings::Ptr settings =
                NetworkManager::ConnectionSettings::Ptr(new NetworkManager::ConnectionSettings(NetworkManager::ConnectionSettings::Wireless));
        settings->setId(accessPoint->ssid());
        settings->setUuid(NetworkManager::ConnectionSettings::createNewUuid());
        settings->setAutoconnect(!ConfigSetting::instance()->enableAccountNetwork());
        settings->setting(NetworkManager::Setting::Security8021x).staticCast<NetworkManager::Security8021xSetting>()->setPasswordFlags(NetworkManager::Setting::AgentOwned);
        NetworkManager::WirelessSecuritySetting::Ptr wsSetting = settings->setting(NetworkManager::Setting::WirelessSecurity).dynamicCast<NetworkManager::WirelessSecuritySetting>();
        if (keyMgmt != NetworkManager::WirelessSecuritySetting::KeyMgmt::WpaNone) {
            wsSetting->setKeyMgmt(keyMgmt);
            if (keyMgmt == NetworkManager::WirelessSecuritySetting::KeyMgmt::WpaEap) {
                // 如果是第一次连接的企业网，就让其自动连接失败，控制中心弹出详细信息的列表
                Q_EMIT connectionFailed(accessPoint);
                Q_EMIT deviceStatusChanged(DeviceStatus::Failed);
                return;
            }

            if (keyMgmt == NetworkManager::WirelessSecuritySetting::KeyMgmt::Wep) {
                wsSetting->setWepKeyFlags(NetworkManager::Setting::None);
            } else {
                wsSetting->setPskFlags(NetworkManager::Setting::None);
            }

            wsSetting->setInitialized(true);
        }

        NetworkManager::WirelessSetting::Ptr wirelessSetting = settings->setting(NetworkManager::Setting::Wireless).dynamicCast<NetworkManager::WirelessSetting>();
        wirelessSetting->setSsid(accessPoint->ssid().toUtf8());
        QString macAddress = m_device->permanentHardwareAddress();
        macAddress.remove(":");
        wirelessSetting->setMacAddress(QByteArray::fromHex(macAddress.toUtf8()));
        wirelessSetting->setInitialized(true);
        QVariantMap options;
        options.insert("persist", "memory");
        options.insert("flags", MANUAL);
        QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(NetworkManager::addAndActivateConnection2(settings->toMap(), m_device->uni(), accessPoint->path(), options));
        connect(watcher, &QDBusPendingCallWatcher::finished, watcher, &QDBusPendingCallWatcher::deleteLater);
        connect(watcher, &QDBusPendingCallWatcher::finished, this, [watcher, accessPoint, this](QDBusPendingCallWatcher *wSelf) {
            QDBusPendingReply<QDBusObjectPath> reply = *wSelf;
            if (watcher->isError() || reply.isError() || !reply.isValid()) {
                qCWarning(DNC) << "connection to " << accessPoint->ssid() << "faild:" << reply.error().message();
                Q_EMIT connectionFailed(accessPoint);
            }
        });
    } else {
        // 获取当前网络的加密类型,判断加密类型是否一致,如果不一致,就修改加密类型为正确的加密类型,否则就会出现在修改加密类型后无法连接的情况
        NetworkManager::WirelessSecuritySetting::Ptr wsSetting = currentConnection->settings()->setting(NetworkManager::Setting::SettingType::WirelessSecurity).dynamicCast<NetworkManager::WirelessSecuritySetting>();
        if (wsSetting && !checkKeyMgmt(m_device->findAccessPoint(accessPoint->path()), wsSetting->keyMgmt())) {
            if (keyMgmt == NetworkManager::WirelessSecuritySetting::KeyMgmt::WpaPsk)
                wsSetting->setAuthAlg(NetworkManager::WirelessSecuritySetting::AuthAlg::Open);
            else
                wsSetting->setAuthAlg(NetworkManager::WirelessSecuritySetting::AuthAlg::None);
            wsSetting->setKeyMgmt(keyMgmt);
            // 非WpaNone且非Unknown才需要初始化setting,参考SecretWirelessSection::saveSettings()
            wsSetting->setInitialized(keyMgmt != NetworkManager::WirelessSecuritySetting::KeyMgmt::WpaNone && keyMgmt != NetworkManager::WirelessSecuritySetting::KeyMgmt::Unknown);
            NMVariantMapMap settingMap = currentConnection->settings()->toMap();
            qCDebug(DNC) << "securit changed..." << settingMap;
            if (currentConnection->isUnsaved()) {
                currentConnection->updateUnsaved(settingMap);
            } else {
                currentConnection->update(settingMap);
                QDBusPendingReply<> reply = currentConnection->save();
                reply.waitForFinished();
            }
        }
        QVariantMap options;
        options["flags"] = MANUAL;
        QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(NetworkManager::activateConnection2(currentConnection->path(), m_device->uni(), QString("/"), options));
        connect(watcher, &QDBusPendingCallWatcher::finished, watcher, &QDBusPendingCallWatcher::deleteLater);
        connect(watcher, &QDBusPendingCallWatcher::finished, this, [ = ](QDBusPendingCallWatcher * wSelf) {
            QDBusPendingReply<QDBusObjectPath> reply = *wSelf;
            qCDebug(DNC) << "activateConnection, isError:" << reply.isError() << "error Name:" << reply.error().name()
                                << "error message:" << reply.error().message();
            if (watcher->isError() || reply.isError() || !reply.isValid()) {
                Q_EMIT connectionFailed(accessPoint);
            } else {
                Q_EMIT connectionSuccess(accessPoint);
            }
        });
    }
}

QList<WirelessConnection *> WirelessDeviceManagerRealize::wirelessItems() const
{
    return m_wirelessConnections;
}

AccessPoints *WirelessDeviceManagerRealize::activeAccessPoints() const
{
    if (!isEnabled())
        return nullptr;

    NetworkManager::AccessPoint::Ptr activeAccessPoint = m_device->activeAccessPoint();
    if (activeAccessPoint.isNull())
        return nullptr;

    QList<AccessPointInfo *>::const_iterator itActiveAccessPoint = std::find_if(m_accessPointInfos.begin(), m_accessPointInfos.end(), [ activeAccessPoint ](AccessPointInfo *accessPoint) {
        return accessPoint->proxy()->contains(activeAccessPoint->uni());
    });

    if (itActiveAccessPoint == m_accessPointInfos.end())
        return nullptr;

    return (*itActiveAccessPoint)->accessPoint();
}

void WirelessDeviceManagerRealize::addConnection(const NetworkManager::Connection::Ptr &connection)
{
    if (connection->settings()->connectionType() != NetworkManager::ConnectionSettings::ConnectionType::Wireless)
        return;

    // 获取无线连接的设置
    NetworkManager::WirelessSetting::Ptr wirelessSetting = connection->settings()->setting(NetworkManager::Setting::SettingType::Wireless)
            .dynamicCast<NetworkManager::WirelessSetting>();
    if (wirelessSetting.isNull())
        return;

    // 过滤掉热点的连接
    if (wirelessSetting->mode() == NetworkManager::WirelessSetting::NetworkMode::Ap)
        return;

    QList<WirelessConnection *>::iterator itConnection = std::find_if(m_wirelessConnections.begin(), m_wirelessConnections.end(), [ connection ](WirelessConnection *item) {
        return item->connection()->path() == connection->path();
    });

    if (itConnection == m_wirelessConnections.end()) {
        WirelessConnection *wirelessItem = new WirelessConnection;
        wirelessItem->setConnection(createJson(m_device, connection));
        wirelessItem->updateTimeStamp(connection->settings()->timestamp());
        m_wirelessConnections << wirelessItem;
        connect(connection.data(), &NetworkManager::Connection::unsavedChanged, this, [this](bool changed) {
            if (!changed)
                Q_EMIT activeConnectionChanged();
        });
        connect(connection.data(), &NetworkManager::Connection::updated, this, [this, connection] {
            QList<WirelessConnection *>::iterator itConnection = std::find_if(m_wirelessConnections.begin(), m_wirelessConnections.end(), [ connection ](WirelessConnection *item) {
                return item->connection()->path() == connection->path();
            });
            if (itConnection != m_wirelessConnections.end()) {
                WirelessConnection *wirelessItem = *itConnection;
                Q_EMIT wirelessConnectionPropertyChanged( { wirelessItem } );
            }
        });
        Q_EMIT wirelessConnectionAdded( { wirelessItem } );
        qCDebug(DNC) << "new Connection id:" << connection->settings()->id() << ", path:" << connection->path();
    } else {
        (*itConnection)->setConnection(createJson(m_device, connection));
    }
}

void WirelessDeviceManagerRealize::removeConnection(const QString &connectionUni)
{
    qCDebug(DNC) << "start remove connection" << connectionUni;
    // 如果存在更改加密方式后,例如从wpa2修改为wpa3后,此时也会触发remove信号,此时需要判断当前的移除的uni是否还存在,如果还存在,则无需移除
    NetworkManager::Connection::List connections = NetworkManager::listConnections();
    for (NetworkManager::Connection::Ptr connection : connections) {
        if (connection->path() != connectionUni)
            continue;

        NetworkManager::WirelessSetting::Ptr wirelessSetting = connection->settings()->setting(NetworkManager::Setting::SettingType::Wireless).dynamicCast<NetworkManager::WirelessSetting>();
        if (wirelessSetting.isNull())
            continue;

        // 如果当前连接的interfaceName或者macAddredd地址相同,则无需删除
        if (connection->settings()->interfaceName() == m_device->interfaceName()
                || wirelessSetting->macAddress().toHex().toUpper() == realHwAdr().remove(":"))
            return;
    }
    QList<WirelessConnection *> removedConnections;
    for (WirelessConnection *item : m_wirelessConnections) {
        if (item->connection()->path() != connectionUni)
            continue;

        removedConnections << item;
    }
    Q_EMIT wirelessConnectionRemoved(removedConnections);

    if (removedConnections.size() == 0)
        return;

    for (WirelessConnection *wirelessConnection : removedConnections) {
        m_wirelessConnections.removeAll(wirelessConnection);
        delete wirelessConnection;
    }
}

void WirelessDeviceManagerRealize::onActiveConnectionChanged()
{
    NetworkManager::ActiveConnection::Ptr activeConnection = m_device->activeConnection();

    auto findAccessPoints = [this](const NetworkManager::ActiveConnection::Ptr &activeConnection)->AccessPointProxyNM *{
        if (activeConnection.isNull())
            return nullptr;

        QList<AccessPointInfo *>::iterator itAccessPoint = std::find_if(m_accessPointInfos.begin(), m_accessPointInfos.end(), [ activeConnection ](AccessPointInfo *accessPoint) {
            NetworkManager::WirelessSetting::Ptr wirelessSetting = activeConnection->connection()->settings()->setting(NetworkManager::Setting::SettingType::Wireless).dynamicCast<NetworkManager::WirelessSetting>();
            if (!wirelessSetting.isNull())
                return accessPoint->accessPoint()->ssid() == wirelessSetting->ssid();

            return false;
        });

        if (itAccessPoint == m_accessPointInfos.end())
            return nullptr;

        return (*itAccessPoint)->proxy();
    };

    AccessPointProxyNM *activeAccessPoint = findAccessPoints(activeConnection);
    if (activeAccessPoint) {
        connect(activeConnection.data(), &NetworkManager::ActiveConnection::stateChanged, this, [ this, activeConnection, findAccessPoints ](NetworkManager::ActiveConnection::State state) {
            AccessPointProxyNM *activeAp = findAccessPoints(activeConnection);
            NetworkManager::Connection::Ptr conn = activeConnection->connection();
            if (activeAp && conn) {
                conn->settings()->setTimestamp(QDateTime::currentDateTime());
                if (state == NetworkManager::ActiveConnection::Activated && conn->isUnsaved()) {
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
                activeAp->updateStatus(convertStateFromNetworkManager(state));
                WirelessConnection *connection = findConnection(conn->path());
                if (connection)
                    connection->updateTimeStamp(conn->settings()->timestamp());

                qCDebug(DNC) << "active connection changed:" << activeAp->ssid() << "device:" << m_device->interfaceName() << "status:" << activeAp->status();
                Q_EMIT activeConnectionChanged();
            }
        });

        activeAccessPoint->updateStatus(convertStateFromNetworkManager(activeConnection->state()));
        WirelessConnection *connection = findConnection(activeConnection->connection()->path());
        if (connection)
            connection->updateTimeStamp(activeConnection->connection()->settings()->timestamp());

        qCDebug(DNC) << "active connection changed:" << activeAccessPoint->ssid() << "device:" << m_device->interfaceName() << "status:" << activeAccessPoint->status();
    }
    for (AccessPointInfo *apInfo : m_accessPointInfos) {
        if(apInfo->proxy() != activeAccessPoint)
            apInfo->proxy()->updateStatus(ConnectionStatus::Deactivated);
    }
    Q_EMIT activeConnectionChanged();
}

DeviceStatus WirelessDeviceManagerRealize::deviceStatus() const
{
    // 如果当前设备开了热点，则认为它为断开状态
    if (m_device->mode() == NetworkManager::WirelessDevice::OperationMode::ApMode || !m_available)
        return DeviceStatus::Disconnected;

    return DeviceManagerRealize::deviceStatus();
}

void WirelessDeviceManagerRealize::deviceEnabledChanged(bool enabled)
{
    Q_UNUSED(enabled);

    if (m_netProcesser)
        m_netProcesser->deviceEnabledChanged();
}

void WirelessDeviceManagerRealize::deviceEnabledAction(const QDBusReply<QDBusObjectPath> &reply, bool enabled)
{
    if (!enabled)
        return;

    QString enableConnectionPath = reply.value().path();
    // 获取所有的wlan的连接
    // 如果禁用之前的连接是热点，就让其不自动连接
    do {
        auto itLastActiveConnection = std::find_if(m_wirelessConnections.begin(), m_wirelessConnections.end(), [ enableConnectionPath ](WirelessConnection *item) {
            return item->connection()->path() == enableConnectionPath;
        });
        if (itLastActiveConnection != m_wirelessConnections.end())
            break;

        auto connectionPath = [ this ] {
            // 如果不在当前的连接列表中，说明连接的是热点，则让其连接当前的wlan即可
            NetworkManager::Connection::List connections = m_device->availableConnections();
            NetworkManager::Connection::List wirelessConnections;
            for (NetworkManager::Connection::Ptr conn : connections) {
                if (conn->settings()->connectionType() != NetworkManager::ConnectionSettings::ConnectionType::Wireless
                        || !conn->settings()->timestamp().isValid() || !conn->settings()->autoconnect())
                    continue;

                // 获取无线连接的设置
                NetworkManager::WirelessSetting::Ptr wirelessSetting = conn->settings()->setting(NetworkManager::Setting::SettingType::Wireless)
                        .dynamicCast<NetworkManager::WirelessSetting>();
                if (wirelessSetting.isNull())
                    continue;

                // 只有mode为Ap的连接才是自建的热点
                if (wirelessSetting->mode() == NetworkManager::WirelessSetting::NetworkMode::Ap)
                    continue;

                wirelessConnections << conn;
            }

            if (wirelessConnections.size() == 0)
                return QString();

            std::sort(wirelessConnections.begin(), wirelessConnections.end(), [](NetworkManager::Connection::Ptr item1, NetworkManager::Connection::Ptr item2 ) {
                return item1->settings()->timestamp() > item2->settings()->timestamp();
            });
            return wirelessConnections.first()->path();
        };
        enableConnectionPath = connectionPath();
        if (enableConnectionPath.isEmpty()) {
            connect(m_device.data(), &NetworkManager::WirelessDevice::availableConnectionAppeared, this, [ this, connectionPath ](const QString &connectionUni) {
                QString enablePath = connectionPath();
                if (enablePath != connectionUni)
                    return;

                // 自动连接
                NetworkManager::activateConnection(enablePath, m_device->uni(), QString());
                qCDebug(DNC) << "connected:" << enablePath;
            });
        }
    } while (0);

    if (!enableConnectionPath.isEmpty()) {
        // 自动连接
        NetworkManager::activateConnection(enableConnectionPath, m_device->uni(), QString());
        qCDebug(DNC) << "connected:" << enableConnectionPath;
    }
}

QString WirelessDeviceManagerRealize::usingHwAdr() const
{
    return m_device->hardwareAddress().toUpper();
}

bool WirelessDeviceManagerRealize::getHotspotIsEnabled() const
{
    NetworkManager::WirelessDevice::OperationMode mode = m_device->mode();
    return (mode == NetworkManager::WirelessDevice::OperationMode::ApMode);
}

bool WirelessDeviceManagerRealize::hotspotEnabled()
{
    return m_hotspotEnabled;
}

void WirelessDeviceManagerRealize::addNetwork(const NetworkManager::WirelessNetwork::Ptr &network)
{
    // 在当前的网络列表中查找同名SSID的网络，如果查找到了，就更新数据，没有查找到，就新增一条网络
    QList<AccessPointInfo *>::iterator itApInfo = std::find_if(m_accessPointInfos.begin(), m_accessPointInfos.end(), [ network ](AccessPointInfo *accessPoint) { return accessPoint->accessPoint()->ssid() == network->ssid(); });
    if (itApInfo == m_accessPointInfos.end()) {
        // 新增的无线网络
        AccessPointInfo *apInfo = new AccessPointInfo(m_device, network);
        m_accessPointInfos << apInfo;

        Q_EMIT networkAdded({ apInfo->accessPoint() });
    } else {
        // 已经存在该无线网络，只需要更新内部的数据即可
        AccessPointInfo *apInfo = *itApInfo;
        apInfo->proxy()->updateNetwork(network);
    }
}

WirelessConnection *WirelessDeviceManagerRealize::findConnection(const QString &path) const
{
    auto itConnection = std::find_if(m_wirelessConnections.begin(), m_wirelessConnections.end(), [ path ](WirelessConnection *item) { return item->connection()->path() == path; });
    if (itConnection != m_wirelessConnections.end())
        return *itConnection;

    return nullptr;
}

void WirelessDeviceManagerRealize::onNetworkAppeared(const QString &ssid)
{
    NetworkManager::WirelessNetwork::Ptr network = m_device->findNetwork(ssid);
    if (network.isNull())
        return;

    qCDebug(DNC) << "network appeared" << ssid;
    addNetwork(network);
    // 新增无线网络后，需要更新网络的连接状态
    onActiveConnectionChanged();
}

void WirelessDeviceManagerRealize::onNetworkDisappeared(const QString &ssid)
{
    // 查找移除的网络
    QList<AccessPointInfo *> removeAccessPoints;
    for (AccessPointInfo *apInfo : m_accessPointInfos) {
        if (apInfo->accessPoint()->ssid() != ssid)
            continue;

        removeAccessPoints << apInfo;
    }

    if (removeAccessPoints.size() == 0)
        return;

    // 将需要移除的网络从网络列表中删除
    QList<AccessPoints *> removeApItems;
    for (AccessPointInfo *aPInfo: removeAccessPoints) {
        m_accessPointInfos.removeOne(aPInfo);
        removeApItems << aPInfo->accessPoint();
    }

    qCDebug(DNC) << "network disappeared" << ssid;
    // 发送信号，通知该网络已经消失
    Q_EMIT networkRemoved(removeApItems);

    // 释放需要移除的网络的内存
    for (AccessPointInfo *apInfo : removeAccessPoints)
        delete apInfo ;

    // 删除无线网络后，需要更新网络的连接状态
    onActiveConnectionChanged();
}

void WirelessDeviceManagerRealize::onInterfaceFlagsChanged()
{
    bool available = (m_device->interfaceFlags() & DEVICE_INTERFACE_FLAG_UP);
    if (m_available == available)
        return;

    m_available = available;
    setDeviceStatus(deviceStatus());
    emit availableChanged(m_available);
}

// IP配置
IpConfig::IpConfig(NetworkManager::IpConfig config, const QString &uni, QObject *parent)
    : QObject (parent)
{
    qRegisterMetaType<QList<QVariantMap>>("QList<QVariantMap>");
    qDBusRegisterMetaType<QList<QVariantMap>>();
    if (!uni.isEmpty() && uni != "/") {
        QDBusConnection::systemBus().connect("org.freedesktop.NetworkManager", uni, "org.freedesktop.DBus.Properties",
                                              "PropertiesChanged", this, SLOT(onPropertiesChanged(QString, QVariantMap, QStringList)));
        NetworkManager::IpAddresses addresses = config.addresses();
        for (NetworkManager::IpAddress addr: addresses) {
            m_addresses << addr.ip().toString();
        }
    }
}

void IpConfig::onPropertiesChanged(const QString &interfaceName, const QVariantMap &changedProperties, const QStringList &invalidatedProperties)
{
    Q_UNUSED(invalidatedProperties);

    if (interfaceName != "org.freedesktop.NetworkManager.IP4Config")
        return;

    if (changedProperties.contains("AddressData")) {
        QStringList addresses;
        QList<QVariantMap> addressDatas = qdbus_cast<QList<QVariantMap>>(changedProperties.value("AddressData").value<QDBusArgument>());
        for (QVariantMap addressData : addressDatas) {
            addresses << addressData.value("address").toString();
        }
        if (addresses.size() != m_addresses.size()) {
            emit ipChanged();
        } else {
            bool changed = false;
            for (const QString &address : addresses) {
                if (m_addresses.contains(address))
                    continue;

                changed = true;
                break;
            }

            if (changed) {
                // 如果有IP不一致，则认为IP发生了变化，发送IP变化的信号
                m_addresses = addresses;
                emit ipChanged();
            }
        }
    }
}
