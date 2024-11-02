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

#ifndef LINGMONETWORKRESOURCEMANAGER_H
#define LINGMONETWORKRESOURCEMANAGER_H

#include <NetworkManagerQt/Manager>
#include <NetworkManagerQt/Settings>
#include <NetworkManagerQt/WirelessDevice>
#include <NetworkManagerQt/GenericTypes>
#include <NetworkManagerQt/VpnConnection>
#include <NetworkManagerQt/WirelessDevice>
#include <NetworkManagerQt/AdslDevice>
#include <NetworkManagerQt/WiredDevice>
#include <NetworkManagerQt/WimaxDevice>
#include <NetworkManagerQt/VlanDevice>
#include <NetworkManagerQt/BondDevice>
#include <NetworkManagerQt/BridgeDevice>
#include <NetworkManagerQt/GenericDevice>
#include <NetworkManagerQt/InfinibandDevice>
#include <NetworkManagerQt/BluetoothDevice>
#include <NetworkManagerQt/OlpcMeshDevice>
#include <NetworkManagerQt/TeamDevice>
#include <NetworkManagerQt/WirelessSetting>
#include <NetworkManagerQt/WirelessSecuritySetting>
#include <NetworkManagerQt/Utils>
#include <NetworkManagerQt/ConnectionSettings>
#include <NetworkManagerQt/Security8021xSetting>
#include <QDBusPendingCallWatcher>
#include <QInputDialog>
#include <QMetaEnum>

QString enumToQstring(NetworkManager::AccessPoint::Capabilities cap, NetworkManager::AccessPoint::WpaFlags wpa_flags,NetworkManager::AccessPoint::WpaFlags rsn_flags);

class KyNetworkResourceManager : public QObject
{
    Q_OBJECT
public:
    static KyNetworkResourceManager* getInstance();
    static void Release();

public:
    explicit KyNetworkResourceManager(QObject *parent = nullptr);
    ~KyNetworkResourceManager();

    void removeActiveConnection(int pos);
    void clearActiveConnections();
    void insertActiveConnections();
    void addActiveConnection(NetworkManager::ActiveConnection::Ptr conn);

    void removeConnection(int pos);
    void clearConnections();
    void insertConnections();
    void addConnection(NetworkManager::Connection::Ptr conn);

    void removeDevice(int pos);
    void clearDevices();
    void insertDevices();
    void addDevice(NetworkManager::Device::Ptr conn);

    void removeWifiNetwork(int pos);
    void clearWifiNetworks();
    void addWifiNetwork(NetworkManager::WirelessNetwork::Ptr net);

    NetworkManager::ActiveConnection::Ptr findActiveConnection(QString const & path);
    template <typename Predicate>
    NetworkManager::Device::Ptr findDevice(Predicate const & pred);
    NetworkManager::Device::Ptr findDeviceUni(QString const & uni);
    NetworkManager::Device::Ptr findDeviceInterface(QString const & interfaceName);
    NetworkManager::WirelessNetwork::Ptr findWifiNetwork(QString const & ssid, QString const & devUni);

    void requestScan(NetworkManager::WirelessDevice * dev);

    void removeConnection(QString const & uuid);

    void connectionDump();

public:
    NetworkManager::Device::Ptr getNetworkDevice(const QString ifaceName);
    NetworkManager::ActiveConnection::Ptr getActiveConnect(const QString activeConnectUuid);
    NetworkManager::Connection::Ptr getConnect(const QString uuid);
    NetworkManager::WirelessNetwork::Ptr getWifiNetwork(const QString apName);

    NetworkManager::ActiveConnection::List getActiveConnectList();
    NetworkManager::Connection::List getConnectList();
    NetworkManager::Device::List getNetworkDeviceList();
    bool isActiveConnection(QString uuid);
    bool isActivatingConnection(QString uuid);

    void getConnectivity(NetworkManager::Connectivity &connectivity);

    bool NetworkManagerIsInited();

Q_SIGNALS:
    void connectionAdd(QString uuid);
    void connectionUpdate(QString uuid);
    void connectionRemove(QString path);

    void deviceAdd(QString deviceName, QString uni, NetworkManager::Device::Type deviceType);
    void deviceUpdate(QString deviceName, QString deviceUni);
    void deviceRemove(QString deviceName, QString uni);
    void deviceManagedChange(QString deviceName, bool managed);

    void deviceActiveChanage(QString deviceName, bool deviceActive);
    void deviceCarrierChanage(QString deviceName, bool pluged);
    void deviceBitRateChanage(QString deviceName, int bitRate);
    void deviceMacAddressChanage(QString deviceName, const QString &hwAddress);

    //to KyWirelessNetResource
    void wifiNetworkRemoved(QString, QString);
    void wifiNetworkAdded(QString, QString);
    void wifiNetworkPropertyChange(QString, QString, int, QString, QString);
    void wifiNetworkSecuChange(NetworkManager::AccessPoint *);
    void wifiNetworkDeviceDisappear();
    void wifiEnabledChanged(bool);
    void wiredEnabledChanged(bool);
    void connectivityCheckSpareUriChanged();

    void activeConnectionsReset();
    void activeConnectionAdd(QString uuid);
    void activeConnectionUpdate(QString uuid);
    void activeConnectionRemove(QString uuid);
    void activeConnectStateChangeReason(QString uuid,
                                    NetworkManager::ActiveConnection::State state,
                                    NetworkManager::ActiveConnection::Reason reason);
    void vpnActiveConnectStateChangeReason(QString uuid,
                                           NetworkManager::VpnConnection::State state,
                                           NetworkManager::VpnConnection::StateChangeReason reason);

    void connectivityChanged(NetworkManager::Connectivity connectivity);
    void stateChanged(NetworkManager::Device::State newstate, NetworkManager::Device::State oldstate, NetworkManager::Device::StateChangeReason reason);

    void needShowDesktop(QString);

public Q_SLOTS:
    void onInitNetwork();
    void setWirelessNetworkEnabled(bool enabled);

private Q_SLOTS:
    void insertWifiNetworks();
    void onServiceAppear(QString, QString, QString);
    void onPropertiesChanged(QVariantMap qvm);
    //connection
    void onConnectionUpdated();
    //void onConnectionRemoved();

    //active connection
    void onActiveConnectionUpdated();
    void onActiveConnectionChangedReason(NetworkManager::ActiveConnection::State state,
                                          NetworkManager::ActiveConnection::Reason reason);

    void onActiveConnectionChanged(NetworkManager::ActiveConnection::State state);
    void onVpnActiveConnectChanagedReason(NetworkManager::VpnConnection::State state,
                                          NetworkManager::VpnConnection::StateChangeReason reason);

    //device

    void onDeviceActiveChanage();

    void onDeviceUpdated();
    void onDeviceManagedChange();
    void onDeviceCarrierChanage(bool pluged);
    void onDeviceBitRateChanage(int bitRate);
    void onDeviceMacAddressChanage(const QString &hwAddress);
    void onDeviceStateChanged(NetworkManager::Device::State newstate,
                      NetworkManager::Device::State oldstate,
                      NetworkManager::Device::StateChangeReason reason);

    void onWifiNetworkAppeared(QString const & ssid);
    void onWifiNetworkDisappeared(QString const & ssid);

    //wifi network
    void onUpdateWirelessNet();
    void onWifiNetworkSecuChang();

    //notifier
    void onDeviceAdded(QString const & uni);
    void onDeviceRemoved(QString const & uni);
    void onActiveConnectionAdded(QString const & path);
    void onActiveConnectionRemoved(QString const & path);

    //settings notifier
    void onConnectionAdded(QString const & path);
    void onConnectionRemoved(QString const & path);

private:
    void onWifiNetworkAdd(NetworkManager::Device * dev, QString const & ssid);
    void onWifiNetworkUpdate(NetworkManager::WirelessNetwork * net);
    void onWifiNetworkRemove(NetworkManager::Device * dev, QString const & ssid);

    void onAccessPointUpdate(NetworkManager::WirelessNetwork * net);
    void onReferenceAccessPointChanged();

private:
    bool m_initFinished = false;

public:
    static KyNetworkResourceManager* m_pInstance;

    NetworkManager::ActiveConnection::List m_activeConns; //已连接资源类
    NetworkManager::Connection::List m_connections;  //配置文件资源类
    NetworkManager::Device::List m_devices;  //设备类
    NetworkManager::WirelessNetwork::List m_wifiNets; //无线热点类
};


#endif // LINGMONETWORKRESOURCEMANAGER_H
