/*
 * Copyright (C) 2020 Tianjin LINGMO Information Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/&gt;.
 *
 */

#include "lingmonetworkresourcemanager.h"
#include <QMetaType>
#include <QTimer>
#include "lingmoutil.h"

#define SIGNAL_DELAY 80000
#define EMIT_DELAY 10000

#define LOG_FLAG  "[KyNetworkResourceManager]"

QString enumToQstring(NetworkManager::AccessPoint::Capabilities cap, NetworkManager::AccessPoint::WpaFlags wpa_flags,NetworkManager::AccessPoint::WpaFlags rsn_flags)
{
    QString out;
    if (   (cap & NM_802_11_AP_FLAGS_PRIVACY)
           && (wpa_flags == NM_802_11_AP_SEC_NONE)
           && (rsn_flags == NM_802_11_AP_SEC_NONE)) {
        out += "WEP ";
   }
    if (wpa_flags != NM_802_11_AP_SEC_NONE) {
        out += "WPA1 ";
    }
    if ((rsn_flags & NM_802_11_AP_SEC_KEY_MGMT_PSK)
            || (rsn_flags & NM_802_11_AP_SEC_KEY_MGMT_802_1X)) {
        out += "WPA2 ";
    }
    if (rsn_flags & NM_802_11_AP_SEC_KEY_MGMT_SAE) {
        out += "WPA3 ";
    }
    if (   (wpa_flags & NM_802_11_AP_SEC_KEY_MGMT_802_1X)
           || (rsn_flags & NM_802_11_AP_SEC_KEY_MGMT_802_1X)) {
        out += "802.1X ";
    }
    return out;
}

KyNetworkResourceManager* KyNetworkResourceManager::m_pInstance = nullptr;

KyNetworkResourceManager* KyNetworkResourceManager::getInstance()
{
    if (m_pInstance == NULL) {
        m_pInstance = new KyNetworkResourceManager();
    }
    return m_pInstance;
}

void KyNetworkResourceManager::Release()
{
    if (m_pInstance != NULL) {
        delete m_pInstance;
        m_pInstance = NULL;
    }
}

KyNetworkResourceManager::KyNetworkResourceManager(QObject *parent) : QObject(parent)
{
    qRegisterMetaType<NetworkManager::ActiveConnection::State>("NetworkManager::ActiveConnection::State");
    qRegisterMetaType<NetworkManager::Connectivity>("NetworkManager::Connectivity");
    qRegisterMetaType<NetworkManager::ActiveConnection::Reason>("NetworkManager::ActiveConnection::Reason");
    qRegisterMetaType<NetworkManager::Device::Type>("NetworkManager::Device::Type");
    qRegisterMetaType<NetworkManager::Device::State>("NetworkManager::Device::State");
    qRegisterMetaType<NetworkManager::Device::StateChangeReason>("NetworkManager::Device::StateChangeReason");

    QDBusConnection::systemBus().connect(QString("org.freedesktop.DBus"),
                                             QString("/org/freedesktop/DBus"),
                                             QString("org.freedesktop.DBus"),
                                             QString("NameOwnerChanged"), this, SLOT(onServiceAppear(QString,QString,QString)));

    QDBusConnection::systemBus().connect(QString("org.freedesktop.NetworkManager"),
                                         QString("/org/freedesktop/NetworkManager"),
                                         QString("org.freedesktop.NetworkManager"),
                                         QString("PropertiesChanged"), this, SLOT(onPropertiesChanged(QVariantMap)));
}

void KyNetworkResourceManager::onInitNetwork()
{
    insertActiveConnections();
    insertConnections();
    insertDevices();
    insertWifiNetworks();

    //initialize NetworkManager signals
    connect(NetworkManager::notifier(), &NetworkManager::Notifier::deviceAdded, this, &KyNetworkResourceManager::onDeviceAdded);
    connect(NetworkManager::notifier(), &NetworkManager::Notifier::deviceRemoved, this, &KyNetworkResourceManager::onDeviceRemoved);
    connect(NetworkManager::notifier(), &NetworkManager::Notifier::activeConnectionAdded, this, &KyNetworkResourceManager::onActiveConnectionAdded);
    connect(NetworkManager::notifier(), &NetworkManager::Notifier::activeConnectionRemoved, this, &KyNetworkResourceManager::onActiveConnectionRemoved);
    connect(NetworkManager::settingsNotifier(), &NetworkManager::SettingsNotifier::connectionAdded, this, &KyNetworkResourceManager::onConnectionAdded);
    connect(NetworkManager::settingsNotifier(), &NetworkManager::SettingsNotifier::connectionRemoved, this, static_cast<void (KyNetworkResourceManager::*)(QString const &)>(&KyNetworkResourceManager::onConnectionRemoved));

    connect(NetworkManager::notifier(), &NetworkManager::Notifier::connectivityChanged, this, &KyNetworkResourceManager::connectivityChanged);
    //todo wifi开关信号
    connect(NetworkManager::notifier(), &NetworkManager::Notifier::wirelessEnabledChanged, this, &KyNetworkResourceManager::wifiEnabledChanged);

    // Note: the connectionRemoved is never emitted in case network-manager service stop,
    // we need remove the connections manually.
    connect(NetworkManager::notifier(), &NetworkManager::Notifier::serviceDisappeared, this, &KyNetworkResourceManager::clearConnections);
    connect(NetworkManager::notifier(), &NetworkManager::Notifier::serviceDisappeared, this, &KyNetworkResourceManager::clearWifiNetworks);

    qDebug() <<"[KyNetworkResourceManager]"
             << "active connections:" << m_activeConns.size()
             << "connections:" << m_connections.size()
             << "network device:" << m_devices.size();
    m_initFinished = true;

    return;
}

KyNetworkResourceManager::~KyNetworkResourceManager()
{

}

void KyNetworkResourceManager::removeActiveConnection(int pos)
{
    //active connections signals
    NetworkManager::ActiveConnection::Ptr conn = m_activeConns.takeAt(pos);
    conn->disconnect(this);
}

void KyNetworkResourceManager::clearActiveConnections()
{
    while (0 < m_activeConns.size())
        removeActiveConnection(0);
}

void KyNetworkResourceManager::addActiveConnection(NetworkManager::ActiveConnection::Ptr conn)
{
    m_activeConns.push_back(conn);
#if 0
    connect(conn.data(), &NetworkManager::ActiveConnection::connectionChanged, this, &KyNetworkResourceManager::onActiveConnectionUpdated);
    connect(conn.data(), &NetworkManager::ActiveConnection::default4Changed, this, &KyNetworkResourceManager::onActiveConnectionUpdated);
    connect(conn.data(), &NetworkManager::ActiveConnection::default6Changed, this, &KyNetworkResourceManager::onActiveConnectionUpdated);
    connect(conn.data(), &NetworkManager::ActiveConnection::dhcp4ConfigChanged, this, &KyNetworkResourceManager::onActiveConnectionUpdated);
    connect(conn.data(), &NetworkManager::ActiveConnection::dhcp6ConfigChanged, this, &KyNetworkResourceManager::onActiveConnectionUpdated);
    connect(conn.data(), &NetworkManager::ActiveConnection::ipV4ConfigChanged, this, &KyNetworkResourceManager::onActiveConnectionUpdated);
    connect(conn.data(), &NetworkManager::ActiveConnection::ipV6ConfigChanged, this, &KyNetworkResourceManager::onActiveConnectionUpdated);
    connect(conn.data(), &NetworkManager::ActiveConnection::idChanged, this, &KyNetworkResourceManager::onActiveConnectionUpdated);
    connect(conn.data(), &NetworkManager::ActiveConnection::typeChanged, this, &KyNetworkResourceManager::onActiveConnectionUpdated);
    connect(conn.data(), &NetworkManager::ActiveConnection::masterChanged, this, &KyNetworkResourceManager::onActiveConnectionUpdated);
    connect(conn.data(), &NetworkManager::ActiveConnection::specificObjectChanged, this, &KyNetworkResourceManager::onActiveConnectionUpdated);
#endif
    //connect(conn.data(), &NetworkManager::ActiveConnection::stateChangedReason, this, &KyNetworkResourceManager::onActiveConnectionChangedReason);
    connect(conn.data(), &NetworkManager::ActiveConnection::stateChanged, this, &KyNetworkResourceManager::onActiveConnectionChanged);
#if 0
    connect(conn.data(), &NetworkManager::ActiveConnection::vpnChanged, this, &KyNetworkResourceManager::onActiveConnectionUpdated);
    connect(conn.data(), &NetworkManager::ActiveConnection::uuidChanged, this, &KyNetworkResourceManager::onActiveConnectionUpdated);
    connect(conn.data(), &NetworkManager::ActiveConnection::devicesChanged, this, &KyNetworkResourceManager::onActiveConnectionUpdated);
#endif
    if (conn->vpn()) {
        connect(qobject_cast<NetworkManager::VpnConnection *>(conn.data()), &NetworkManager::VpnConnection::bannerChanged, this, &KyNetworkResourceManager::onActiveConnectionUpdated);
        connect(qobject_cast<NetworkManager::VpnConnection *>(conn.data()), &NetworkManager::VpnConnection::stateChanged, this, &KyNetworkResourceManager::onVpnActiveConnectChanagedReason);
    }
}

void KyNetworkResourceManager::insertActiveConnections()
{
    for (auto const & conn : NetworkManager::activeConnections()) {
        if (conn.isNull()) {
            continue;
        }

        addActiveConnection(conn);
    }
}

void KyNetworkResourceManager::removeConnection(int pos)
{
    //connections signals
    QString path = m_connections.at(pos)->path();
    NetworkManager::Connection::Ptr conn = m_connections.takeAt(pos);
    conn->disconnect(this);
    Q_EMIT connectionRemove(path);
}

void KyNetworkResourceManager::clearConnections()
{
    while (0 < m_connections.size()) {
        removeConnection(0);
    }
}

void KyNetworkResourceManager::addConnection(NetworkManager::Connection::Ptr conn)
{
    m_connections.push_back(conn);

    //connections signals
    connect(conn.data(), &NetworkManager::Connection::updated, this, &KyNetworkResourceManager::onConnectionUpdated);
    connect(conn.data(), &NetworkManager::Connection::removed, this, &KyNetworkResourceManager::onConnectionRemoved);
}

void KyNetworkResourceManager::insertConnections()
{
    for (auto const & connectPtr : NetworkManager::listConnections()) {
        if (connectPtr.isNull()) {
            continue;
        }

        if (connectPtr->name().isEmpty() || connectPtr->uuid().isEmpty()) {
            qWarning() <<"[KyNetworkResourceManager]" << " the name of connection is empty.";
            continue;
        }

        addConnection(connectPtr);
    }

    return;
}

void KyNetworkResourceManager::removeDevice(int pos)
{
    //connections signals
    NetworkManager::Device::Ptr device = m_devices.takeAt(pos);

    QDBusConnection::systemBus().disconnect(QString("org.freedesktop.NetworkManager"),
                                            device.data()->uni(),
                                            QString("org.freedesktop.NetworkManager.Device"),
                                            QString("AcdIpProbed"), this, SIGNAL(needShowDesktop(QString)));
    device->disconnect(this);
}

void KyNetworkResourceManager::clearDevices()
{
    while (0 < m_devices.size())
        removeDevice(0);
}

void KyNetworkResourceManager::addDevice(NetworkManager::Device::Ptr device)
{
    m_devices.push_back(device);
    //device signals
#if 0
    //connect(device.data(), &NetworkManager::Device::stateChanged, this, &KyNetworkResourceManager::onDeviceUpdated);
    connect(device.data(), &NetworkManager::Device::activeConnectionChanged, this, &KyNetworkResourceManager::onDeviceUpdated);
    connect(device.data(), &NetworkManager::Device::autoconnectChanged, this, &KyNetworkResourceManager::onDeviceUpdated);
    connect(device.data(), &NetworkManager::Device::availableConnectionChanged, this, &KyNetworkResourceManager::onDeviceUpdated);
    connect(device.data(), &NetworkManager::Device::availableConnectionAppeared, this, &KyNetworkResourceManager::onDeviceUpdated);
    connect(device.data(), &NetworkManager::Device::availableConnectionDisappeared, this, &KyNetworkResourceManager::onDeviceUpdated);
    connect(device.data(), &NetworkManager::Device::capabilitiesChanged, this, &KyNetworkResourceManager::onDeviceUpdated);
    connect(device.data(), &NetworkManager::Device::dhcp4ConfigChanged, this, &KyNetworkResourceManager::onDeviceUpdated);
    connect(device.data(), &NetworkManager::Device::dhcp6ConfigChanged, this, &KyNetworkResourceManager::onDeviceUpdated);
    connect(device.data(), &NetworkManager::Device::driverChanged, this, &KyNetworkResourceManager::onDeviceUpdated);
    connect(device.data(), &NetworkManager::Device::driverVersionChanged, this, &KyNetworkResourceManager::onDeviceUpdated);
    connect(device.data(), &NetworkManager::Device::firmwareMissingChanged, this, &KyNetworkResourceManager::onDeviceUpdated);
    connect(device.data(), &NetworkManager::Device::firmwareVersionChanged, this, &KyNetworkResourceManager::onDeviceUpdated);
#endif
    connect(device.data(), &NetworkManager::Device::activeConnectionChanged, this, &KyNetworkResourceManager::onDeviceActiveChanage);
    connect(device.data(), &NetworkManager::Device::interfaceNameChanged, this, &KyNetworkResourceManager::onDeviceUpdated);
    connect(device.data(), &NetworkManager::Device::managedChanged, this, &KyNetworkResourceManager::onDeviceManagedChange);
#if 0
    connect(device.data(), &NetworkManager::Device::ipV4AddressChanged, this, &KyNetworkResourceManager::onDeviceUpdated);
    connect(device.data(), &NetworkManager::Device::ipV4ConfigChanged, this, &KyNetworkResourceManager::onDeviceUpdated);
    connect(device.data(), &NetworkManager::Device::ipV6ConfigChanged, this, &KyNetworkResourceManager::onDeviceUpdated);
    connect(device.data(), &NetworkManager::Device::ipInterfaceChanged, this, &KyNetworkResourceManager::onDeviceUpdated);
    connect(device.data(), &NetworkManager::Device::physicalPortIdChanged, this, &KyNetworkResourceManager::onDeviceUpdated);
    connect(device.data(), &NetworkManager::Device::mtuChanged, this, &KyNetworkResourceManager::onDeviceUpdated);
    connect(device.data(), &NetworkManager::Device::nmPluginMissingChanged, this, &KyNetworkResourceManager::onDeviceUpdated);
    connect(device.data(), &NetworkManager::Device::meteredChanged, this, &KyNetworkResourceManager::onDeviceUpdated);
    connect(device.data(), &NetworkManager::Device::connectionStateChanged, this, &KyNetworkResourceManager::onDeviceUpdated);
    connect(device.data(), &NetworkManager::Device::stateReasonChanged, this, &KyNetworkResourceManager::onDeviceUpdated);

    connect(device.data(), &NetworkManager::Device::stateChanged, this, &KyNetworkResourceManager::onDeviceStateChanged);
    connect(device.data(), &NetworkManager::Device::udiChanged, this, &KyNetworkResourceManager::onDeviceUpdated);
#endif
    switch (device->type())
    {
        case NetworkManager::Device::Ethernet:
            connect(qobject_cast<NetworkManager::WiredDevice *>(device.data()), &NetworkManager::WiredDevice::bitRateChanged, this, &KyNetworkResourceManager::onDeviceBitRateChanage);
            connect(qobject_cast<NetworkManager::WiredDevice *>(device.data()), &NetworkManager::WiredDevice::carrierChanged, this, &KyNetworkResourceManager::onDeviceCarrierChanage);
            connect(qobject_cast<NetworkManager::WiredDevice *>(device.data()), &NetworkManager::WiredDevice::hardwareAddressChanged, this, &KyNetworkResourceManager::onDeviceMacAddressChanage);
           // connect(qobject_cast<NetworkManager::WiredDevice *>(device.data()), &NetworkManager::WiredDevice::permanentHardwareAddressChanged, this, &KyNetworkResourceManager::onDeviceUpdated);
            break;

        case NetworkManager::Device::Wifi:
#if 0
            connect(qobject_cast<NetworkManager::WirelessDevice *>(device.data()), &NetworkManager::WirelessDevice::bitRateChanged, this, &KyNetworkResourceManager::onDeviceUpdated);
            connect(qobject_cast<NetworkManager::WirelessDevice *>(device.data()), &NetworkManager::WirelessDevice::activeAccessPointChanged, this, &KyNetworkResourceManager::onDeviceUpdated);
            connect(qobject_cast<NetworkManager::WirelessDevice *>(device.data()), &NetworkManager::WirelessDevice::modeChanged, this, &KyNetworkResourceManager::onDeviceUpdated);
            connect(qobject_cast<NetworkManager::WirelessDevice *>(device.data()), &NetworkManager::WirelessDevice::wirelessCapabilitiesChanged, this, &KyNetworkResourceManager::onDeviceUpdated);
            connect(qobject_cast<NetworkManager::WirelessDevice *>(device.data()), &NetworkManager::WirelessDevice::hardwareAddressChanged, this, &KyNetworkResourceManager::onDeviceUpdated);
            connect(qobject_cast<NetworkManager::WirelessDevice *>(device.data()), &NetworkManager::WirelessDevice::permanentHardwareAddressChanged, this, &KyNetworkResourceManager::onDeviceUpdated);
            connect(qobject_cast<NetworkManager::WirelessDevice *>(device.data()), &NetworkManager::WirelessDevice::wirelessPropertiesChanged, this, &KyNetworkResourceManager::onDeviceUpdated);
            connect(qobject_cast<NetworkManager::WirelessDevice *>(device.data()), &NetworkManager::WirelessDevice::accessPointAppeared, this, &KyNetworkResourceManager::onDeviceUpdated);
            connect(qobject_cast<NetworkManager::WirelessDevice *>(device.data()), &NetworkManager::WirelessDevice::accessPointDisappeared, this, &KyNetworkResourceManager::onDeviceUpdated);
#endif
            connect(qobject_cast<NetworkManager::WirelessDevice *>(device.data()), &NetworkManager::WirelessDevice::networkAppeared, this, &KyNetworkResourceManager::onWifiNetworkAppeared);
            connect(qobject_cast<NetworkManager::WirelessDevice *>(device.data()), &NetworkManager::WirelessDevice::networkDisappeared, this, &KyNetworkResourceManager::onWifiNetworkDisappeared);
            connect(device.data(), &NetworkManager::Device::stateChanged, this, &KyNetworkResourceManager::stateChanged);
            break;
        default:
            //TODO: other device types!
            break;
    }
    QDBusConnection::systemBus().connect(QString("org.freedesktop.NetworkManager"),
                                         device.data()->uni(),
                                         QString("org.freedesktop.NetworkManager.Device"),
                                         QString("AcdIpProbed"), this, SIGNAL(needShowDesktop(QString)));
}

void KyNetworkResourceManager::insertDevices()
{
    for (auto const & device : NetworkManager::networkInterfaces()) {
        if (device.isNull()) {
            continue;
        }

        addDevice(device);
    }

    return;
}

void KyNetworkResourceManager::removeWifiNetwork(int pos)
{
    //network signals
    NetworkManager::WirelessNetwork::Ptr net = m_wifiNets.takeAt(pos);
    net->disconnect(this);
}

void KyNetworkResourceManager::clearWifiNetworks()
{
    while (0 < m_wifiNets.size())
        removeWifiNetwork(0);
}

void KyNetworkResourceManager::addWifiNetwork(NetworkManager::WirelessNetwork::Ptr net)
{
    m_wifiNets.push_back(net);
    //device signals
    connect(net.data(), &NetworkManager::WirelessNetwork::signalStrengthChanged, this, &KyNetworkResourceManager::onUpdateWirelessNet);
    connect(net.data(), &NetworkManager::WirelessNetwork::referenceAccessPointChanged, this, &KyNetworkResourceManager::onUpdateWirelessNet);
    connect(net.data(), &NetworkManager::WirelessNetwork::referenceAccessPointChanged, this, &KyNetworkResourceManager::onReferenceAccessPointChanged);
    connect(net.data(), &NetworkManager::WirelessNetwork::disappeared, this, &KyNetworkResourceManager::onUpdateWirelessNet);
    connect(net->referenceAccessPoint().data(), &NetworkManager::AccessPoint::wpaFlagsChanged, this, &KyNetworkResourceManager::onWifiNetworkSecuChang,
            Qt::UniqueConnection);
    connect(net->referenceAccessPoint().data(), &NetworkManager::AccessPoint::rsnFlagsChanged, this, &KyNetworkResourceManager::onWifiNetworkSecuChang,
            Qt::UniqueConnection);
}

void KyNetworkResourceManager::insertWifiNetworks()
{
    for (auto const & device : m_devices) {
        if (device.isNull()) {
            continue;
        }

        if (NetworkManager::Device::Wifi == device->type()) {
            NetworkManager::WirelessDevice::Ptr w_dev = device.objectCast<NetworkManager::WirelessDevice>();
            for (auto const & net : w_dev->networks()) {
                if (!net.isNull()) {
                    addWifiNetwork(net);
                    Q_EMIT wifiNetworkAdded(device->interfaceName(),net->ssid());
                }
            }
        }
    }
}

NetworkManager::ActiveConnection::Ptr KyNetworkResourceManager::findActiveConnection(QString const & path)
{
    auto i = std::find_if(m_activeConns.cbegin(), m_activeConns.cend(), [&path] (NetworkManager::ActiveConnection::Ptr const & conn) -> bool {
        return conn->path() == path;
    });
    return m_activeConns.cend() == i ? NetworkManager::ActiveConnection::Ptr{} : *i;
}

template <typename Predicate>
NetworkManager::Device::Ptr KyNetworkResourceManager::findDevice(Predicate const & pred)
{
    auto i = std::find_if(m_devices.cbegin(), m_devices.cend(), pred);
    return m_devices.cend() == i ? NetworkManager::Device::Ptr{} : *i;
}

NetworkManager::Device::Ptr KyNetworkResourceManager::findDeviceUni(QString const & uni)
{
    return findDevice([&uni] (NetworkManager::Device::Ptr const & dev) { return dev->uni() == uni; });
}

NetworkManager::Device::Ptr KyNetworkResourceManager::findDeviceInterface(QString const & interfaceName)
{
    return findDevice([&interfaceName] (NetworkManager::Device::Ptr const & dev) { return dev->interfaceName() == interfaceName; });
}

NetworkManager::WirelessNetwork::Ptr KyNetworkResourceManager::findWifiNetwork(QString const & ssid, QString const & devUni)
{
    auto i = std::find_if(m_wifiNets.cbegin(), m_wifiNets.cend(), [&ssid, &devUni] (NetworkManager::WirelessNetwork::Ptr const & net) -> bool {
        return net->ssid() == ssid && net->device() == devUni;
    });
    return m_wifiNets.cend() == i ? NetworkManager::WirelessNetwork::Ptr{} : *i;
}

NetworkManager::Device::Ptr KyNetworkResourceManager::getNetworkDevice(const QString ifaceName)
{
    NetworkManager::Device::Ptr devicePtr = nullptr;
    if (ifaceName.isEmpty()) {
        return nullptr;
    }

    for (int index = 0; index < m_devices.size(); ++index) {
        devicePtr = m_devices.at(index);
        if (devicePtr.isNull()) {
            continue;
        }

        if (ifaceName == devicePtr->interfaceName()) {
            return devicePtr;
        }
    }

    return nullptr;
}

NetworkManager::ActiveConnection::Ptr KyNetworkResourceManager::getActiveConnect(const QString activeConnectUuid)
{
    int index = 0;
    NetworkManager::ActiveConnection::Ptr activateConnectPtr = nullptr;

    qDebug()<<"[KyNetworkResourceManager]"<<"get activetate connect with uuid"<< activeConnectUuid;
    if (activeConnectUuid.isEmpty()) {
        return nullptr;
    }

    for (index = 0; index < m_activeConns.size(); ++index) {
        activateConnectPtr = m_activeConns.at(index);
        if (activateConnectPtr.isNull()) {
            continue;
        }

        if (activateConnectPtr->uuid() == activeConnectUuid) {
            return activateConnectPtr;
        }
    }

    qWarning()<<"[KyNetworkResourceManager]"<<"it can not find the activate connect with uuid" <<activeConnectUuid;

    return nullptr;
}

NetworkManager::Connection::Ptr KyNetworkResourceManager::getConnect(const QString connectUuid)
{
    int index = 0;
    NetworkManager::Connection::Ptr connectPtr = nullptr;

    if (connectUuid.isEmpty()) {
        qWarning() << "[KyNetworkResourceManager]" << "get connect with uuid is empty";
        return nullptr;
    }

    for (index = 0; index < m_connections.size(); ++index) {
        connectPtr = m_connections.at(index);
        if (connectPtr.isNull()) {
            continue;
        }

        if (connectPtr->uuid() == connectUuid) {
            return connectPtr;
        }
    }
    qWarning() << "[KyNetworkResourceManager]" << "it can not find connect with uuid" << connectUuid;

    return nullptr;
}

NetworkManager::WirelessNetwork::Ptr KyNetworkResourceManager::getWifiNetwork(const QString apName)
{
    return nullptr;
}

NetworkManager::ActiveConnection::List KyNetworkResourceManager::getActiveConnectList()
{
    return m_activeConns;
}

NetworkManager::Connection::List KyNetworkResourceManager::getConnectList()
{
    return m_connections;
}

NetworkManager::Device::List KyNetworkResourceManager::getNetworkDeviceList()
{
    return m_devices;
}

bool KyNetworkResourceManager::isActiveConnection(QString uuid)
{
    int index = 0;
    NetworkManager::ActiveConnection::Ptr activateConnectPtr = nullptr;

    if (uuid.isEmpty()) {
        return false;
    }

    for (index = 0; index < m_activeConns.size(); ++index) {
        activateConnectPtr = m_activeConns.at(index);

        if (activateConnectPtr.isNull()) {
            continue;
        }

        if (activateConnectPtr->uuid() == uuid
             && NetworkManager::ActiveConnection::State::Activated == activateConnectPtr->state()) {
            return true;
        }
    }

    return false;
}

bool KyNetworkResourceManager::isActivatingConnection(QString uuid)
{
    int index = 0;
    NetworkManager::ActiveConnection::Ptr activateConnectPtr = nullptr;

    if (uuid.isEmpty()) {
        return false;
    }

    for (index = 0; index < m_activeConns.size(); ++index) {
        activateConnectPtr = m_activeConns.at(index);
        if (activateConnectPtr.isNull()) {
            continue;
        }

        if (activateConnectPtr->uuid() == uuid
             && NetworkManager::ActiveConnection::State::Activating == activateConnectPtr->state()) {
            return true;
        }
    }

    return false;
}

void KyNetworkResourceManager::getConnectivity(NetworkManager::Connectivity &connectivity)
{
    connectivity = NetworkManager::connectivity();
}

bool KyNetworkResourceManager::NetworkManagerIsInited()
{
    return m_initFinished;
}

void KyNetworkResourceManager::requestScan(NetworkManager::WirelessDevice * dev)
{
    if (nullptr == dev) {
        qWarning() << LOG_FLAG << "request scan failed, wireless device is invalid.";
        return;
    }

    qDebug() <<"[KyNetworkResourceManager]"<< dev->interfaceName()<<"start scan wifi ap";
    QDBusPendingReply<> reply = dev->requestScan();
    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(reply, dev);
    connect(watcher, &QDBusPendingCallWatcher::finished, [dev] (QDBusPendingCallWatcher * watcher) {
        if (watcher->isError() || !watcher->isValid())
        {
            //TODO: in what form should we output the warning messages
            qWarning() << QStringLiteral("requestScan on device '%1' failed: %3").arg(dev->interfaceName())
                    .arg(watcher->error().message());
        }
        watcher->deleteLater();
    });

    return;
}

void KyNetworkResourceManager::onServiceAppear(QString interface, QString oldOwner, QString newOwner)
{
    if (interface == "org.freedesktop.NetworkManager"
            && oldOwner.isEmpty() && !newOwner.isEmpty()) {
        qDebug() << LOG_FLAG << "org.freedesktop.NetworkManager start";
        QTimer::singleShot(500,this,&KyNetworkResourceManager::insertWifiNetworks);
    }
}

void KyNetworkResourceManager::onPropertiesChanged(QVariantMap qvm)
{
    for(QString keyStr : qvm.keys()) {
        //收到wifi开关打开或关闭的信号后，进行处理
        if (keyStr == "WiredEnabled") {
            bool wiredEnable = qvm.value("WiredEnabled").toBool();
            Q_EMIT wiredEnabledChanged(wiredEnable);
        }
    }
    for(QString keyStr : qvm.keys()) {
        //内网检测地址变化
        if (keyStr == "ConnectivityCheckSpareUri") {
            Q_EMIT connectivityCheckSpareUriChanged();
        }
    }
}

void KyNetworkResourceManager::onConnectionUpdated()
{
    NetworkManager::Connection *connectPtr =
            qobject_cast<NetworkManager::Connection *>(sender());
    if (nullptr != connectPtr && connectPtr->isValid()) {
        qDebug()<< LOG_FLAG <<"connection will Update, connection name"<<connectPtr->name()
                << "connection uuid" << connectPtr->uuid();
        Q_EMIT connectionUpdate(connectPtr->uuid());
    } else {
        qWarning()<< LOG_FLAG
                  <<"onConnectionUpdate failed, the connect is invalid";
    }

    return;
}

void KyNetworkResourceManager::onActiveConnectionUpdated()
{
    //Q_EMIT activeConnectionUpdate(qobject_cast<NetworkManager::ActiveConnection *>(sender()));
}

void KyNetworkResourceManager::onActiveConnectionChangedReason(
                                      NetworkManager::ActiveConnection::State state,
                                      NetworkManager::ActiveConnection::Reason reason)
{
    NetworkManager::ActiveConnection * activeConnect =
        qobject_cast<NetworkManager::ActiveConnection *>(sender());
    if (nullptr != activeConnect && activeConnect->isValid()) {
        qDebug()<< LOG_FLAG <<"connect uuid"<<activeConnect->uuid()
                <<"state change"<<state << "chanage reason:"<<reason;
        Q_EMIT activeConnectStateChangeReason(activeConnect->uuid(), state, reason);
    } else {
        qWarning() << LOG_FLAG << "onActiveConnectionChangedReason failed, the connection is invalid.";
    }

    return;
}

void KyNetworkResourceManager::onActiveConnectionChanged(
                                      NetworkManager::ActiveConnection::State state)
{
    NetworkManager::ActiveConnection * activeConnect =
        qobject_cast<NetworkManager::ActiveConnection *>(sender());
    if (nullptr != activeConnect && activeConnect->isValid()) {
        qDebug()<< LOG_FLAG << "connection uuid" << activeConnect->uuid()
                <<"state change "<<state;
        while(activeConnect->state() != state) {
            qDebug() << "[KyNetworkResourceManager]"
                     <<"connect real state"<<activeConnect->state() <<"change state"<<state;
            ::usleep(EMIT_DELAY);
        }

        Q_EMIT activeConnectStateChangeReason(activeConnect->uuid(), state,
                                            NetworkManager::ActiveConnection::Reason::UknownReason);
    } else {
        qWarning() << LOG_FLAG << "onActiveConnectionChanged failed, the connection is invalid.";
    }

    return;
}

void KyNetworkResourceManager::onVpnActiveConnectChanagedReason(NetworkManager::VpnConnection::State state,
                                      NetworkManager::VpnConnection::StateChangeReason reason)
{
    NetworkManager::ActiveConnection *activeConnect =
        qobject_cast<NetworkManager::ActiveConnection *>(sender());

    if (nullptr != activeConnect && activeConnect->isValid()) {
        qDebug()<<"vpn connect uuid" << activeConnect->uuid()
                <<"state change " <<state <<"reason " << reason;
        Q_EMIT vpnActiveConnectStateChangeReason(activeConnect->uuid(), state, reason);
    } else {
        qWarning() << LOG_FLAG << "onVpnActiveConnectChanagedReason failed, the connection is invalid.";
    }

    return;
}

void KyNetworkResourceManager::onDeviceActiveChanage()
{
    NetworkManager::Device *p_device = qobject_cast<NetworkManager::Device *>(sender());
    if (nullptr == p_device) {
        return;
    }

    QString deviceName = p_device->interfaceName();
    //此处需要取反，因为激活连接的网卡状态是false，断开连接的网卡状态是true
    bool isActive = !p_device->isActive();

    qDebug()<< LOG_FLAG << "device active change, device name " << deviceName
            << "active state" << isActive;

    Q_EMIT deviceActiveChanage(deviceName, isActive);

    return;
}

void KyNetworkResourceManager::onDeviceUpdated()
{
    NetworkManager::Device *p_device = qobject_cast<NetworkManager::Device *>(sender());
    if (nullptr == p_device) {
        return;
    }

    QString deviceName = p_device->interfaceName();
    QString deviceUni = p_device->uni();

    Q_EMIT deviceUpdate(deviceName, deviceUni);

    return;
}

void KyNetworkResourceManager::onDeviceManagedChange()
{
    NetworkManager::Device *p_device = qobject_cast<NetworkManager::Device *>(sender());
    if (nullptr == p_device) {
        return;
    }

    QString deviceName = p_device->interfaceName();
    bool managed = p_device->managed();

    Q_EMIT deviceManagedChange(deviceName, managed);
}

void KyNetworkResourceManager::onDeviceCarrierChanage(bool pluged)
{
    NetworkManager::WiredDevice * networkDevice
            = qobject_cast<NetworkManager::WiredDevice *>(sender());

    qDebug()<< LOG_FLAG<<"device carrier chanage"<< pluged;
    if (nullptr !=networkDevice && networkDevice->isValid()) {
        Q_EMIT deviceCarrierChanage(networkDevice->interfaceName(), pluged);
    } else {
        qWarning()<< LOG_FLAG<<"onDeviceCarrierChanage failed.";
    }

    return;
}

void KyNetworkResourceManager::onDeviceBitRateChanage(int bitRate)
{
    NetworkManager::WiredDevice * networkDevice
            = qobject_cast<NetworkManager::WiredDevice *>(sender());

    if (nullptr != networkDevice
            && networkDevice->isValid()) {
        Q_EMIT deviceBitRateChanage(networkDevice->interfaceName(), bitRate);
    } else {
        qWarning()<< LOG_FLAG <<"the device is not invalid with bitrate" << bitRate;
    }

    return;
}

void KyNetworkResourceManager::onDeviceMacAddressChanage(const QString &hwAddress)
{
    NetworkManager::WiredDevice * networkDevice
            = qobject_cast<NetworkManager::WiredDevice *>(sender());

    if (nullptr !=networkDevice && networkDevice->isValid()) {
        Q_EMIT deviceMacAddressChanage(networkDevice->interfaceName(), hwAddress);
    } else {
        qWarning()<< LOG_FLAG <<"the device is not invalid with mac" << hwAddress;
    }

    return;
}

void KyNetworkResourceManager::onDeviceStateChanged(
                    NetworkManager::Device::State newstate,
                    NetworkManager::Device::State oldstate,
                    NetworkManager::Device::StateChangeReason reason)
{
    qWarning() << LOG_FLAG <<"the device state "<<oldstate << "to" <<newstate << "reason"<< reason;
    return;
}


void KyNetworkResourceManager::onWifiNetworkAdd(NetworkManager::Device * dev, QString const & ssid)
{
    if (nullptr == dev) {
        return;
    }

    qDebug()<< LOG_FLAG << "wifiNetworkAdd" << dev << dev->interfaceName() << ssid;

    NetworkManager::WirelessDevice * w_dev = qobject_cast<NetworkManager::WirelessDevice *>(dev);
    NetworkManager::WirelessNetwork::Ptr net = w_dev->findNetwork(ssid);
    if (!net.isNull()) {
        if (0 > m_wifiNets.indexOf(net)){
            addWifiNetwork(net);
        } else {
            //TODO: onWifiNetworkUpdate
            qDebug()<< LOG_FLAG << "add but already exist";
        }

        NetworkManager::AccessPoint::Ptr accessPoitPtr = net->referenceAccessPoint();
        QByteArray rawSsid = accessPoitPtr->rawSsid();
        QString wifiSsid = getSsidFromByteArray(rawSsid);
        Q_EMIT wifiNetworkAdded(dev->interfaceName(), wifiSsid);
    }

    return;
}

void KyNetworkResourceManager::onWifiNetworkUpdate(NetworkManager::WirelessNetwork * net)
{
    if (nullptr == net) {
        return;
    }

    bool bFlag = false;
    QString devIface;
    NetworkManager::Device::Ptr dev = findDeviceUni(net->device());
    if(dev.isNull()) {
        qDebug()<< LOG_FLAG << "device invalid";
        bFlag = true;
    } else {
        devIface = dev->interfaceName();
    }
    if(bFlag) {
        //device invalid
        qDebug() << LOG_FLAG << "wifiNetworkDeviceDisappear";
        Q_EMIT wifiNetworkDeviceDisappear();
        return;
    }


    auto index = std::find(m_wifiNets.cbegin(), m_wifiNets.cend(), net);
    if (m_wifiNets.cend() != index) {
        if (net->accessPoints().isEmpty()) {
            //remove
            auto pos = index - m_wifiNets.cbegin();
            removeWifiNetwork(pos);
            qDebug()<< LOG_FLAG  << "wifiNetwork disappear" << net << net->ssid();
            NetworkManager::AccessPoint::Ptr accessPoitPtr = net->referenceAccessPoint();
            QByteArray rawSsid = accessPoitPtr->rawSsid();
            QString wifiSsid = getSsidFromByteArray(rawSsid);
            Q_EMIT wifiNetworkRemoved(devIface, wifiSsid);
        } else {
            NetworkManager::AccessPoint::Ptr accessPointPtr = net->referenceAccessPoint();
            if (accessPointPtr.isNull()) {
                return;
            }
            QByteArray rawSsid = accessPointPtr->rawSsid();
            QString wifiSsid = getSsidFromByteArray(rawSsid);
            QString bssid = accessPointPtr->hardwareAddress();
            QString secuType = enumToQstring(accessPointPtr->capabilities(),
                                             accessPointPtr->wpaFlags(),
                                             accessPointPtr->rsnFlags());
            Q_EMIT wifiNetworkPropertyChange(devIface, wifiSsid, net->signalStrength(), bssid, secuType);
        }
    }

    return;
}

void KyNetworkResourceManager::onWifiNetworkRemove(NetworkManager::Device * dev, QString const & ssid)
{
    if (nullptr == dev) {
        return;
    }

    qDebug() << "wifiNetworkRemove" << dev << dev->interfaceName() << ssid;

    NetworkManager::WirelessNetwork::Ptr net = findWifiNetwork(ssid, dev->uni());
    if (!net.isNull()) {
        auto pos = m_wifiNets.indexOf(net);
        if (0 <= pos) {
            removeWifiNetwork(pos);
            NetworkManager::AccessPoint::Ptr accessPoitPtr = net->referenceAccessPoint();
            QByteArray rawSsid = accessPoitPtr->rawSsid();
            QString wifiSsid = getSsidFromByteArray(rawSsid);
            Q_EMIT wifiNetworkRemoved(dev->interfaceName(), wifiSsid);
        }
    }

    return;
}

void KyNetworkResourceManager::onWifiNetworkAppeared(QString const & ssid)
{
    NetworkManager::Device * p_device = qobject_cast<NetworkManager::Device *>(sender());
    if (nullptr != p_device) {
        onWifiNetworkAdd(p_device, ssid);

        QString deviceName = p_device->interfaceName();
        QString deviceUni = p_device->uni();

        Q_EMIT deviceUpdate(deviceName, deviceUni);
    } else {
        qWarning()<< LOG_FLAG << "onWifiNetworkAppeared failed.";
    }

    return;
}

void KyNetworkResourceManager::onWifiNetworkDisappeared(QString const & ssid)
{
    NetworkManager::Device *p_device = qobject_cast<NetworkManager::Device *>(sender());
    if (nullptr != p_device) {
        onWifiNetworkRemove(p_device, ssid);

        QString deviceName = p_device->interfaceName();
        QString deviceUni = p_device->uni();

        Q_EMIT deviceUpdate(deviceName, deviceUni);
    } else {
        qWarning()<< LOG_FLAG << "onWifiNetworkDisappeared failed.";
    }

    return;
}

void KyNetworkResourceManager::onReferenceAccessPointChanged()
{
    NetworkManager::WirelessNetwork *p_wirelessNet =
                    qobject_cast<NetworkManager::WirelessNetwork *>(sender());
    if (nullptr != p_wirelessNet) {
        onAccessPointUpdate(p_wirelessNet);
    }
}

void KyNetworkResourceManager::onAccessPointUpdate(NetworkManager::WirelessNetwork * net)
{
    if (nullptr == net) {
        return;
    }
    auto index = std::find(m_wifiNets.cbegin(), m_wifiNets.cend(), net);
    if (m_wifiNets.cend() != index) {
        if (!net->referenceAccessPoint().isNull()) {
            connect(net->referenceAccessPoint().data(), &NetworkManager::AccessPoint::wpaFlagsChanged, this, &KyNetworkResourceManager::onWifiNetworkSecuChang,
                    Qt::UniqueConnection);
            connect(net->referenceAccessPoint().data(), &NetworkManager::AccessPoint::rsnFlagsChanged, this, &KyNetworkResourceManager::onWifiNetworkSecuChang,
                    Qt::UniqueConnection);
        }
    }
}

void KyNetworkResourceManager::onUpdateWirelessNet()
{
    NetworkManager::WirelessNetwork *p_wirelessNet =
                    qobject_cast<NetworkManager::WirelessNetwork *>(sender());
    if (nullptr != p_wirelessNet) {
        onWifiNetworkUpdate(p_wirelessNet);
    }

    return;
}

void KyNetworkResourceManager::onWifiNetworkSecuChang()
{
    NetworkManager::AccessPoint *p_wirelessNet =
                    qobject_cast<NetworkManager::AccessPoint *>(sender());
    Q_EMIT wifiNetworkSecuChange(p_wirelessNet);
}

void KyNetworkResourceManager::onDeviceAdded(QString const & uni)
{
    qDebug()<< "onDeviceAdded"<<uni;

    NetworkManager::Device::Ptr networkDevice = NetworkManager::findNetworkInterface(uni);
    if (networkDevice.isNull()) {
        qWarning()<<"the device is not exist in networkmanager."<< uni;
        return;
    }

    if (!networkDevice->isValid()) {
        qWarning() << uni << " is currently not invalid";
        return;
    }

    if (0 > m_devices.indexOf(networkDevice)) {
        addDevice(networkDevice);
        Q_EMIT deviceAdd(networkDevice->interfaceName(), networkDevice->uni(), networkDevice->type());
    } else {
        qWarning() << networkDevice->interfaceName() <<"the device is exist in network device list.";
    }

    return;
}

void KyNetworkResourceManager::onDeviceRemoved(QString const & uni)
{
    NetworkManager::Device::Ptr networkDevice = findDeviceUni(uni);
    if (networkDevice.isNull()) {
        qWarning()<<"the device is not exist in network device list ."<< uni;
        return;
    }

    qDebug()<<"remove network device"<< networkDevice->interfaceName() <<"uni:" << uni;

    auto index = std::find(m_devices.cbegin(), m_devices.cend(), networkDevice);
    if (m_devices.cend() != index) {
        const int pos = index - m_devices.cbegin();
        removeDevice(pos);
        Q_EMIT deviceRemove(networkDevice->interfaceName(), networkDevice->uni());
    }

    return;
}

void KyNetworkResourceManager::onActiveConnectionAdded(QString const & path)
{
    NetworkManager::ActiveConnection::Ptr activeConnectPtr = NetworkManager::findActiveConnection(path);
    if (activeConnectPtr.isNull()) {
        qWarning() << "[KyNetworkResourceManager]" << "the active connect is null, so do not add it."<<path;
        return;
    }

    if (!activeConnectPtr->isValid()) {
        qWarning() << "[KyNetworkResourceManager]" << path << " connect is currently not valid";
        return;
    }

    qDebug()<<"add active connect"<<activeConnectPtr->connection()->name();

    if (0 > m_activeConns.indexOf(activeConnectPtr)) {
        addActiveConnection(activeConnectPtr);
        Q_EMIT activeConnectionAdd(activeConnectPtr->uuid());
    } else {
        //TODO: onActiveConnectionUpdate
        qWarning() << "[KyNetworkResourceManager]" << "update active connection to do";
        //Q_EMIT activeConnectionUpdate(conn->uuid());
    }

    return;
}

void KyNetworkResourceManager::onActiveConnectionRemoved(QString const & path)
{
    NetworkManager::ActiveConnection::Ptr activeConnectPtr = findActiveConnection(path);//XXX: const QString &uni
    if (activeConnectPtr.isNull()) {
        qWarning() <<"[KyNetworkResourceManager]"
                  <<"the active connect is null, so do not remove it. "<< path;
        return;
    }

    qDebug() <<"[KyNetworkResourceManager]"<<"remove active connect"<<activeConnectPtr->uuid();

    auto index = std::find(m_activeConns.cbegin(), m_activeConns.cend(), activeConnectPtr);
    if (m_activeConns.cend() != index) {
        const int pos = index - m_activeConns.cbegin();
        removeActiveConnection(pos);
        Q_EMIT activeConnectionRemove(activeConnectPtr->uuid());
    }

    return;
}

void KyNetworkResourceManager::onConnectionAdded(QString const & path)
{
    NetworkManager::Connection::Ptr connectPtr = NetworkManager::findConnection(path);
    if (connectPtr.isNull()) {
        qWarning() <<"[KyNetworkResourceManager]" <<"it can not find in networkmanager."<<path;
        return;
    }

    if (!connectPtr->isValid()) {
        qWarning() <<"[KyNetworkResourceManager]" << path << " is currently not invalid";
        return;
    }

    qDebug() <<"[KyNetworkResourceManager]" <<"add connect "<< connectPtr->name() << connectPtr->path();

    if (connectPtr->name().isEmpty() || connectPtr->uuid().isEmpty()) {
        qWarning() <<"[KyNetworkResourceManager]" << "the name or uuid of connection is empty";
        return;
    }

    if (0 > m_connections.indexOf(connectPtr)) {
        addConnection(connectPtr);
        Q_EMIT connectionAdd(connectPtr->uuid());
    } else {
        //TODO::updateconnect
        qWarning() << "[KyNetworkResourceManager]" << connectPtr->uuid() <<" connect is exist in connect list.";
    }

    return;
}

void KyNetworkResourceManager::onConnectionRemoved(QString const & path)
{
    if (path.isEmpty()) {
        qDebug() <<"[KyNetworkResourceManager]" <<"the connect path is empty";
        return;
    }

    qDebug() <<"[KyNetworkResourceManager]" << "remove connection path" << path;
    NetworkManager::Connection::Ptr connectionPtr = nullptr;
    for (int index = 0; index < m_connections.size(); ++index) {
        connectionPtr = m_connections.at(index);
        if (connectionPtr->path() == path) {
            removeConnection(index);
            return;
        }
    }

    qWarning() <<"[KyNetworkResourceManager]" << path <<" connect is no exist in connect list";

    return;
}

void KyNetworkResourceManager::removeConnection(QString const & uuid)
{
    NetworkManager::Connection::Ptr conn = this->getConnect(uuid);
    if(!conn.isNull()) {
        conn->disconnect(this);
        conn->remove();
    }
}

void KyNetworkResourceManager::setWirelessNetworkEnabled(bool enabled)
{
    NetworkManager::setWirelessEnabled(enabled);
}

void KyNetworkResourceManager::connectionDump()
{
    NetworkManager::Connection::Ptr connectionPtr = nullptr;
    for (int index = 0; index < m_connections.size(); ++index) {
        connectionPtr = m_connections.at(index);
        qDebug()<<"connection info**********************";
        qDebug()<<"connection name"<< connectionPtr->name();
        qDebug()<<"connection uuid"<< connectionPtr->uuid();
        qDebug()<<"connection path"<< connectionPtr->path();
    }
}
