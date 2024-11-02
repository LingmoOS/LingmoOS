/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * Copyright (C) 2022 Tianjin LINGMO Information Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "lingmoconnectresource.h"
#include "kywirelessconnectoperation.h"
#include "lingmoutil.h"

#include <NetworkManagerQt/Ipv4Setting>
#include <NetworkManagerQt/Ipv6Setting>
#include <NetworkManagerQt/VpnConnection>
#include <NetworkManagerQt/VpnPlugin>
#include <NetworkManagerQt/VpnSetting>

const QString str2GBand = "2.4GHz";
const QString str5GBand = "5GHz";

static bool subLanListSort(const KyConnectItem* info1, const KyConnectItem* info2)
{
    QString  name1 = info1->m_connectName;
    QString  name2 = info2->m_connectName;
    bool result = true;
    if (QString::compare(name1, name2, Qt::CaseInsensitive) > 0) {
        result =  false;
    }
    return result;
}

static bool subVpnListSort(const KyConnectItem* info1, const KyConnectItem* info2)
{
    if (info1->m_connectState != info2->m_connectState) {
        if (info1->m_connectState == 2) {
            return true;
        }

        if (info2->m_connectState == 2) {
            return false;
        }
    }
    QString  name1 = info1->m_connectName;
    QString  name2 = info2->m_connectName;
    bool result = true;
    if (QString::compare(name1, name2, Qt::CaseInsensitive) > 0) {
        result =  false;
    }
    return result;
}

static void lanListSort(QList<KyConnectItem *> &list)
{
    qSort(list.begin(), list.end(), subLanListSort);
}

static void vpnListSort(QList<KyConnectItem *> &list)
{
    qSort(list.begin(), list.end(), subVpnListSort);
}

KyConnectResourse::KyConnectResourse(QObject *parent) : QObject(parent)
{
    m_networkResourceInstance = KyNetworkResourceManager::getInstance();

    connect(m_networkResourceInstance, &KyNetworkResourceManager::connectionAdd, this, &KyConnectResourse::connectionAdd);
    connect(m_networkResourceInstance, &KyNetworkResourceManager::connectionRemove, this, &KyConnectResourse::connectionRemove);
    connect(m_networkResourceInstance, &KyNetworkResourceManager::connectionUpdate, this, &KyConnectResourse::connectionUpdate);
    connect(m_networkResourceInstance, &KyNetworkResourceManager::connectivityChanged, this, &KyConnectResourse::connectivityChanged);

    connect(m_networkResourceInstance, &KyNetworkResourceManager::connectivityCheckSpareUriChanged, this, &KyConnectResourse::connectivityCheckSpareUriChanged);
    connect(m_networkResourceInstance, &KyNetworkResourceManager::needShowDesktop, this, &KyConnectResourse::needShowDesktop);
}

KyConnectResourse::~KyConnectResourse()
{
    m_networkResourceInstance = nullptr;
}

KyConnectItem *KyConnectResourse::getConnectionItem(NetworkManager::Connection::Ptr connectPtr, QString devName)
{
    if (nullptr == connectPtr) {
        qWarning()<<"[KyConnectResourse]"<<"the connect is empty";
        return nullptr;
    }

    KyConnectItem *connectionItem = new KyConnectItem();
    connectionItem->m_connectName = connectPtr->name();
    connectionItem->m_connectUuid = connectPtr->uuid();
    connectionItem->m_connectPath = connectPtr->path();

    NetworkManager::ConnectionSettings::Ptr settingPtr = connectPtr->settings();
    connectionItem->m_ifaceName = settingPtr->interfaceName();
    connectionItem->m_itemType = settingPtr->connectionType();

    if (m_networkResourceInstance->isActivatingConnection(connectPtr->uuid())) {
        if (connectionItem->m_ifaceName == "" && devName != "") {
            if (isActiveDevice(connectionItem->m_connectUuid, devName)) {
                connectionItem->m_connectState = NetworkManager::ActiveConnection::State::Activating;
            } else {
                connectionItem->m_connectState = NetworkManager::ActiveConnection::State::Deactivated;
            }
        } else {
            connectionItem->m_connectState = NetworkManager::ActiveConnection::State::Activating;
        }
    } else {
        connectionItem->m_connectState = NetworkManager::ActiveConnection::State::Deactivated;
    }
    return connectionItem;
}

bool KyConnectResourse::isActiveDevice(QString conUuid, QString devName)
{
    QString deviceName = "";

    NetworkManager::ActiveConnection::Ptr activeConnectPtr =
                                    m_networkResourceInstance->getActiveConnect(conUuid);

    if (nullptr == activeConnectPtr) {
        qWarning()<< "[KyConnectResourse]" <<"it can not find activating connect "<< conUuid;
        return false;
    }

    QStringList interfaces = activeConnectPtr->devices();
    if (interfaces.isEmpty()) {
        qWarning()<< "[KyConnectResourse]" << "get device of activing connection failed.";
        return false;
    }

    QString ifaceUni = "";
    for (int index=0; index < interfaces.size(); index++) {
        ifaceUni = interfaces.at(index);
        NetworkManager::Device:: Ptr devicePtr =
              m_networkResourceInstance->findDeviceUni(ifaceUni);
        deviceName = devicePtr->interfaceName();
        if (deviceName == devName) {
            return true;
        }
    }
    return false;
}

KyConnectItem * KyConnectResourse::getConnectionItemByUuid(QString connectUuid)
{
    NetworkManager::Connection::Ptr connectPtr =
            m_networkResourceInstance->getConnect(connectUuid);

    if (nullptr == connectPtr) {
        qWarning()<< "[KyConnectResourse]" <<"get connect failed, connect uuid"<<connectUuid;
        return nullptr;
    }

    if (m_networkResourceInstance->isActiveConnection(connectPtr->uuid())) {
        qDebug()<<"[KyConnectResourse]"<<connectPtr->name()<<"is active connection";
        return nullptr;
    }

    KyConnectItem *connectItem = getConnectionItem(connectPtr, "");
    if (nullptr != connectItem) {
        //connectItem->dumpInfo();
        return connectItem;
    }

    return nullptr;
}

KyConnectItem * KyConnectResourse::getConnectionItemByUuidWithoutActivateChecking(QString connectUuid)
{
    NetworkManager::Connection::Ptr connectPtr =
            m_networkResourceInstance->getConnect(connectUuid);

    if (nullptr == connectPtr) {
        qWarning()<< "[KyConnectResourse]" <<"get connect failed, connect uuid"<<connectUuid;
        return nullptr;
    }

    KyConnectItem *connectItem = getConnectionItem(connectPtr, "");
    if (nullptr != connectItem) {
        //connectItem->dumpInfo();
        return connectItem;
    }

    return nullptr;
}

KyConnectItem * KyConnectResourse::getConnectionItemByUuid(QString connectUuid, QString deviceName)
{
    NetworkManager::Connection::Ptr connectPtr =
            m_networkResourceInstance->getConnect(connectUuid);

    if (nullptr == connectPtr) {
        qWarning()<< "[KyConnectResourse]" <<"get connect failed, connect uuid"<<connectUuid;
        return nullptr;
    }

    QString connectInterface = connectPtr->settings()->interfaceName();
    if (!connectInterface.isEmpty()
           && deviceName != connectInterface) {
        qDebug()<<"[KyConnectResourse]" << "connect name:"<< connectPtr->name()
               << "connect device name" << connectInterface;
        return nullptr;
    }

    if (m_networkResourceInstance->isActiveConnection(connectPtr->uuid())) {
        qDebug()<<"[KyConnectResourse]"<<connectPtr->name()<<"is active connection";
        return nullptr;
    }

    KyConnectItem *connectItem = getConnectionItem(connectPtr, deviceName);
    if (nullptr != connectItem) {
        //connectItem->dumpInfo();
        return connectItem;
    }

    return nullptr;
}

void KyConnectResourse::getVpnAndVirtualConnections(QList<KyConnectItem *> &connectItemList)
{
    int index = 0;
    NetworkManager::Connection::List connectList;

    qDebug()<<"[KyConnectResourse]"<<"get vpn && virtual connections";

    connectList.clear();
    connectList = m_networkResourceInstance->getConnectList();

    if (connectList.empty()) {
        qWarning()<<"[KyConnectResourse]"<<"get vpn connections failed, the connect list is empty";
        return;
    }

    NetworkManager::Connection::Ptr connectPtr = nullptr;
    for (index = 0; index < connectList.size(); index++) {
        connectPtr = connectList.at(index);
        if (connectPtr.isNull()) {
            continue;
        }
        if (NetworkManager::ConnectionSettings::ConnectionType::Vpn != connectPtr->settings()->connectionType()
            && NetworkManager::ConnectionSettings::ConnectionType::Bond != connectPtr->settings()->connectionType()
            && NetworkManager::ConnectionSettings::ConnectionType::Bridge != connectPtr->settings()->connectionType()
            && NetworkManager::ConnectionSettings::ConnectionType::Vlan != connectPtr->settings()->connectionType()
            && NetworkManager::ConnectionSettings::ConnectionType::Team != connectPtr->settings()->connectionType()
            && NetworkManager::ConnectionSettings::ConnectionType::IpTunnel != connectPtr->settings()->connectionType()
            && NetworkManager::ConnectionSettings::ConnectionType::Wired != connectPtr->settings()->connectionType()) {
            continue;
        }
        NetworkManager::Device::Ptr devicePtr = nullptr;
        devicePtr = m_networkResourceInstance->findDeviceInterface(connectPtr->settings()->interfaceName());
        if (NetworkManager::ConnectionSettings::ConnectionType::Wired == connectPtr->settings()->connectionType()) {
            if (devicePtr == nullptr || !devicePtr->udi().startsWith("/sys/devices/virtual/net")) {
                continue;
            }
        }
        QString devName = "";
        if (!devicePtr.isNull()) {
            devName = devicePtr->interfaceName();
        }

        KyConnectItem *connectItem = getConnectionItem(connectPtr, devName);
        if (nullptr != connectItem) {
            connectItemList << connectItem;
        }

        connectPtr = nullptr;
    }

    if (connectItemList.size() > 1) {
        vpnListSort(connectItemList);
    }
}

void KyConnectResourse::getConnectionList(QString deviceName,
                       NetworkManager::ConnectionSettings::ConnectionType connectionType,
                       QList<KyConnectItem *> &connectItemList)
{
    NetworkManager::Connection::List connectList;

    qDebug()<<"[KyConnectResourse]"<<"get connections item, device"
           <<deviceName << "connect type" << connectionType;

    connectList.clear();
    connectList = m_networkResourceInstance->getConnectList();

    if (connectList.empty()) {
        qWarning()<<"[KyConnectResourse]"<<"get connection failed, the connect list is empty";
        return;
    }

    NetworkManager::Connection::Ptr connectPtr = nullptr;
    for (int index = 0; index < connectList.size(); index++) {
        connectPtr = connectList.at(index);
        if (connectPtr.isNull()) {
            continue;
        }

        if (connectionType != connectPtr->settings()->connectionType()) {
            qDebug()<<"[KyConnectResourse]"<<"connect name:" << connectPtr->name()
                   <<"connect type:"<<connectPtr->settings()->connectionType();
            continue;
        }
        
        QString connectInterface = connectPtr->settings()->interfaceName();
        if (!connectInterface.isEmpty()
               && deviceName != connectInterface) {
            qDebug() << "[KyConnectResourse]" << "connect name:"<< connectPtr->name()
                   << "connect device name" << connectInterface;
            continue;
        }

        if (m_networkResourceInstance->isActiveConnection(connectPtr->uuid())) {
            qDebug()<<"[KyConnectResourse]"<<connectPtr->name()<<"is active connection";
            continue;
        }

        KyConnectItem *connectItem = getConnectionItem(connectPtr, deviceName);
        if (nullptr != connectItem) {
           // connectItem->m_itemType = connectionType;
            connectItemList << connectItem;
            //connectItem->dumpInfo();
        }

        connectPtr = nullptr;
    }

    if (connectItemList.size() > 1) {
        lanListSort(connectItemList);
    }
    return;    
}

#if 0
void KyConnectResourse::getWiredConnections(QList<KyWiredConnectItem *> &wiredConnectItemList)
{
    int index = 0;
    NetworkManager::Connection::List connectList;

    qDebug()<<"[KyConnectResourse]"<<"get wired connections";

    connectList.clear();
    connectList = m_networkResourceInstance->getConnectList();

    if (connectList.empty()) {
        qWarning()<<"[KyConnectResourse]"<<"get wired connection failed, the connect list is empty";
        return;
    }

    NetworkManager::Connection::Ptr connectPtr = nullptr;
    for (index = 0; index < connectList.size(); index++) {
        connectPtr = connectList.at(index);
        if (NetworkManager::ConnectionSettings::ConnectionType::Wired
                != connectPtr->settings()->connectionType()) {
            qDebug()<<"[KyConnectResourse]"<<"connect name:" << connectPtr->name()
                   <<"connect type:"<<connectPtr->settings()->connectionType();
            continue;
        }

        if (m_networkResourceInstance->isActiveConnection(connectPtr->uuid())) {
            qDebug()<<"[KyConnectResourse]"<<connectPtr->name()<<"is active connection";
            continue;
        }

        KyWiredConnectItem *connectItem = getWiredConnectItem(connectPtr);
        if (nullptr != connectItem) {
            wiredConnectItemList << connectItem;
            connectItem->dumpInfo();
        }

        connectPtr = nullptr;
    }

    return;
}

KyWiredConnectItem *KyConnectResourse::getWiredConnectItem(NetworkManager::Connection::Ptr connectPtr)
{
    qDebug()<<"[KyConnectResourse]"<<"get connect item";

    if (nullptr == connectPtr) {
        qWarning()<<"[KyConnectResourse]"<<"the connect is empty";
        return nullptr;
    }

    KyWiredConnectItem *wiredItem = new KyWiredConnectItem();
    NetworkManager::ConnectionSettings::Ptr settingPtr = connectPtr->settings();

    wiredItem->m_connectName = connectPtr->name();
    wiredItem->m_connectUuid = connectPtr->uuid();
    wiredItem->m_ifaceName = settingPtr->interfaceName();

    getConnectIp(settingPtr, wiredItem->m_ipv4, wiredItem->m_ipv6);

    m_networkdevice->getWiredHardwareInfo(settingPtr->interfaceName(), wiredItem);

    wiredItem->m_state = NetworkManager::ActiveConnection::State::Deactivated;
    //wiredItem->m_itemType;

    return wiredItem;
}

#endif

void KyConnectResourse::getConnectIp(
                        NetworkManager::ConnectionSettings::Ptr settingPtr,
                        QString &ipv4Address,
                        QString &ipv6Address)
{
    NetworkManager::Ipv4Setting::Ptr ipv4Setting =
            settingPtr->setting(NetworkManager::Setting::Ipv4).dynamicCast<NetworkManager::Ipv4Setting>();
    if (NetworkManager::Ipv4Setting::Manual == ipv4Setting->method()) {
       QList<NetworkManager::IpAddress> ipv4AddressList = ipv4Setting->addresses();
       NetworkManager::IpAddress settingIpv4Address = ipv4AddressList.at(0);
       if (settingIpv4Address.isValid()) {
           ipv4Address = settingIpv4Address.ip().toString();
       } else {
           qWarning()<<"[KyConnectResourse]"<<"get connect ipv4 failed, ipv4Address is not valid";
       }
    } else {
        qWarning()<<"[KyConnectResourse]"<<"get connect ipv4 failed, ipv4 config with dhcp";
    }

    NetworkManager::Ipv6Setting::Ptr ipv6Setting = settingPtr->setting(NetworkManager::Setting::Ipv6).dynamicCast<NetworkManager::Ipv6Setting>();
    if (nullptr !=ipv6Setting
            && NetworkManager::Ipv4Setting::Manual == ipv6Setting->method()) {
       QList<NetworkManager::IpAddress> ipv6AddressList = ipv6Setting->addresses();
       NetworkManager::IpAddress settingIpv6Address = ipv6AddressList.at(0);
       if (settingIpv6Address.isValid()) {
           ipv6Address = settingIpv6Address.ip().toString();
       } else {
           qWarning()<<"[KyConnectResourse]"<<"get connect ipv6 failed, ipv6Address is not valid";
       }
    } else {
        qWarning()<<"[KyConnectResourse]"<<"get connect ipv6 failed, ipv6 config with dhcp";
    }

    return;
}


void KyConnectResourse::getIpv4ConnectSetting(
                        NetworkManager::Ipv4Setting::Ptr &ipv4Setting,
                        KyConnectSetting &connectSetting)
{
    connectSetting.m_ipv4Dns = ipv4Setting->dns();
    if (NetworkManager::Ipv4Setting::Automatic == ipv4Setting->method()) {
        connectSetting.m_ipv4ConfigIpType = CONFIG_IP_DHCP;
        return;
    }

    connectSetting.m_ipv4ConfigIpType = CONFIG_IP_MANUAL;
    connectSetting.m_ipv4Address = ipv4Setting->addresses();
}

void KyConnectResourse::getIpv6ConnectSetting(
                        NetworkManager::Ipv6Setting::Ptr &ipv6Setting,
                        KyConnectSetting &connectSetting)
{
    connectSetting.m_ipv6Dns = ipv6Setting->dns();
    if (NetworkManager::Ipv6Setting::Automatic == ipv6Setting->method()) {
        connectSetting.m_ipv6ConfigIpType = CONFIG_IP_DHCP;
        return;
    }

    connectSetting.m_ipv6ConfigIpType = CONFIG_IP_MANUAL;
    connectSetting.m_ipv6Address = ipv6Setting->addresses();
}

void KyConnectResourse::getConnectivity(NetworkManager::Connectivity &connectivity)
{
    m_networkResourceInstance->getConnectivity(connectivity);
}

void KyConnectResourse::getConnectionSetting(QString connectUuid, KyConnectSetting &connectSetting)
{
    qDebug() <<"[KyConnectResourse]" << connectUuid <<"get connect setting info, connect uuid";

    NetworkManager::Connection::Ptr connectPtr =
                            m_networkResourceInstance->getConnect(connectUuid);

    if (nullptr == connectPtr
               || !connectPtr->isValid()) {
        qWarning() <<"[KyConnectResourse]" << "it can not find valid connection" << connectUuid;
        return;
    }

    connectSetting.m_connectName = connectPtr->name();

    NetworkManager::ConnectionSettings::Ptr connectionSettings = connectPtr->settings();
    connectSetting.m_ifaceName = connectionSettings->interfaceName();
    connectSetting.m_isAutoConnect = connectionSettings->autoconnect();

    NetworkManager::Ipv4Setting::Ptr ipv4Setting = connectionSettings->setting(NetworkManager::Setting::Ipv4).dynamicCast<NetworkManager::Ipv4Setting>();
    getIpv4ConnectSetting(ipv4Setting, connectSetting);

    NetworkManager::Ipv6Setting::Ptr ipv6Setting = connectionSettings->setting(NetworkManager::Setting::Ipv6).dynamicCast<NetworkManager::Ipv6Setting>();
    getIpv6ConnectSetting(ipv6Setting, connectSetting);

    return;
}

bool KyConnectResourse::getInterfaceByUuid(QString &deviceName, const QString connUuid)
{
    deviceName.clear();
    NetworkManager::Connection::Ptr connectPtr =
            m_networkResourceInstance->getConnect(connUuid);

    if (nullptr == connectPtr) {
        qWarning()<<"getInterfaceByUuid failed, connect uuid"<<connUuid;
        return false;
    }
    NetworkManager::ConnectionSettings::Ptr connectSettingPtr = connectPtr->settings();

    if (connectSettingPtr.isNull()) {
        qWarning()<<"getInterfaceByUuid failed, connect uuid"<<connUuid;
        return false;
    }

    deviceName = connectPtr->settings()->interfaceName();
    qDebug() << "getInterfaceByUuid success " << deviceName;
    return true;
}

void KyConnectResourse::getVpnConnectData(NetworkManager::ConnectionSettings::Ptr settingsPtr,
                       KyVpnConnectItem *vpnItem)
{
    NetworkManager::VpnSetting::Ptr vpnSetting = settingsPtr->setting(NetworkManager::Setting::Vpn).dynamicCast<NetworkManager::VpnSetting>();
    NMStringMap vpnDataMap = vpnSetting->data();
    if (vpnDataMap.isEmpty()) {
        qWarning()<<"get vpn connection Data failed, the data is empty";
        return;
    }

    vpnItem->m_vpnGateWay = vpnDataMap["gateway"];
    vpnItem->m_vpnUser = vpnDataMap["user"];
    if ( "yes" == vpnDataMap["require-mppe"]) {
        vpnItem->m_vpnMppe = true;
    } else {
        vpnItem->m_vpnMppe = false;
        qDebug()<<"vpn mppe required:"<< vpnDataMap["require-mppe"];
    }

    return;
}

KyVpnConnectItem *KyConnectResourse::getVpnConnectItem(NetworkManager::Connection::Ptr connectPtr)
{
    if (nullptr == connectPtr) {
        qWarning()<<"[KyConnectResourse]"<<"get vpn connection item failed, the connect is empty";
        return nullptr;
    }

    KyVpnConnectItem *vpnItem = new KyVpnConnectItem();
    NetworkManager::ConnectionSettings::Ptr settingPtr = connectPtr->settings();

    vpnItem->m_vpnName = connectPtr->name();
    vpnItem->m_vpnUuid = connectPtr->uuid();
    vpnItem->m_vpnState = NetworkManager::VpnConnection::State::Disconnected;

    getConnectIp(settingPtr, vpnItem->m_vpnIpv4Address, vpnItem->m_vpnIpv6Address);
    getVpnConnectData(settingPtr, vpnItem);

    return vpnItem;
}

void KyConnectResourse::getVpnConnections(QList<KyVpnConnectItem *> &vpnConnectItemList)
{
    int index = 0;
    NetworkManager::Connection::List connectList;

    qDebug()<<"[KyConnectResourse]"<<"get vpn connections";

    connectList.clear();
    connectList = m_networkResourceInstance->getConnectList();

    if (connectList.empty()) {
        qWarning()<<"[KyConnectResourse]"<<"get vpn connections failed, the connect list is empty";
        return;
    }

    NetworkManager::Connection::Ptr connectPtr = nullptr;
    for (index = 0; index < connectList.size(); index++) {
        connectPtr = connectList.at(index);
        if (connectPtr.isNull()) {
            continue;
        }

        if (NetworkManager::ConnectionSettings::ConnectionType::Vpn
                != connectPtr->settings()->connectionType()) {
            qDebug()<<"[KyConnectResourse]"<<"connect name:" << connectPtr->name()
                   <<"connect type:"<<connectPtr->settings()->connectionType();
            continue;
        }

        if (m_networkResourceInstance->isActiveConnection(connectPtr->uuid())) {
            qDebug()<<"[KyConnectResourse]"<<connectPtr->name()<<"is active connection";
            continue;
        }

        KyVpnConnectItem *connectItem = getVpnConnectItem(connectPtr);
        if (nullptr != connectItem) {
            vpnConnectItemList << connectItem;
            //connectItem->dumpInfo();
        }

        connectPtr = nullptr;
    }

    return;
}


KyBluetoothConnectItem *KyConnectResourse::getBluetoothConnectItem(NetworkManager::Connection::Ptr connectPtr)
{
    if (nullptr == connectPtr) {
        qWarning()<<"[KyConnectResourse]"<<"get bluetooth connection item failed, the connect is empty";
        return nullptr;
    }

    KyBluetoothConnectItem *bluetoothItem = new KyBluetoothConnectItem();
    bluetoothItem->m_connectName = connectPtr->name();
    bluetoothItem->m_connectUuid = connectPtr->uuid();
    bluetoothItem->m_state = NetworkManager::ActiveConnection::State::Deactivated;

    NetworkManager::ConnectionSettings::Ptr settingPtr = connectPtr->settings();
    getConnectIp(settingPtr, bluetoothItem->m_ipv4Address, bluetoothItem->m_ipv6Address);

    NetworkManager::BluetoothSetting::Ptr bluetoothSetting =
            settingPtr->setting(NetworkManager::Setting::Bluetooth).dynamicCast<NetworkManager::BluetoothSetting>();
    bluetoothItem->m_deviceAddress = bluetoothSetting->bluetoothAddress();
    QByteArray btAddrArray = bluetoothSetting->bluetoothAddress();
    for (int index = 0; index < btAddrArray.size(); ++index) {
        qDebug("bt address %d %s", index, btAddrArray[index]);
    }
   // qDebug()<<"bt address 0:"<< btAddrArray[0];
   // qDebug()<<"bt address 1:"<< btAddrArray[1];
   // qDebug()<<"array size:"<<btAddrArray.size();
    qDebug()<<"bluetooth device address:"<<bluetoothItem->m_deviceAddress.toInt(nullptr, 16);

    return bluetoothItem;
}

void KyConnectResourse::getBluetoothConnections(QList<KyBluetoothConnectItem *> &bluetoothConnectItemList)
{
    int index = 0;
    NetworkManager::Connection::List connectList;

    qDebug()<<"[KyConnectResourse]"<<"get bluetooth connections";

    connectList.clear();
    connectList = m_networkResourceInstance->getConnectList();

    if (connectList.empty()) {
        qWarning()<<"[KyConnectResourse]"<<"get bluetooth connections failed, the connect list is empty";
        return;
    }

    NetworkManager::Connection::Ptr connectPtr = nullptr;
    for (index = 0; index < connectList.size(); index++) {
        connectPtr = connectList.at(index);
        if (connectPtr.isNull()) {
            continue;
        }

        if (NetworkManager::ConnectionSettings::ConnectionType::Bluetooth
                != connectPtr->settings()->connectionType()) {
            qDebug()<<"[KyConnectResourse]"<<"connect name:" << connectPtr->name()
                   <<"connect type:"<<connectPtr->settings()->connectionType();
            continue;
        }

        if (m_networkResourceInstance->isActiveConnection(connectPtr->uuid())) {
            qDebug()<<"[KyConnectResourse]"<<connectPtr->name()<<"is active connection";
            continue;
        }

        KyBluetoothConnectItem *connectItem = getBluetoothConnectItem(connectPtr);
        if (nullptr != connectItem) {
            bluetoothConnectItemList << connectItem;
            //connectItem->dumpInfo();
        }

        connectPtr = nullptr;
    }

    return;
}


KyApConnectItem *KyConnectResourse::getApConnectItem(NetworkManager::Connection::Ptr connectPtr)
{
    if (nullptr == connectPtr) {
        qWarning()<<"[KyConnectResourse]"<<"get bluetooth connection item failed, the connect is empty";
        return nullptr;
    }

    NetworkManager::ConnectionSettings::Ptr settingPtr = connectPtr->settings();
    NetworkManager::WirelessSetting::Ptr wirelessSetting
        = settingPtr->setting(NetworkManager::Setting::Wireless).dynamicCast<NetworkManager::WirelessSetting>();

    if (NetworkManager::WirelessSetting::NetworkMode::Ap
                                    != wirelessSetting->mode()) {
        qDebug() << "[KyConnectResourse]" <<"get ap item failed, the active connect mode is not ap.";
        return nullptr;
    }

#ifdef CHECKDEVICE
    KyNetworkDeviceResourse deviceResource;
    if (!deviceResource.wirelessDeviceIsExist(settingPtr->interfaceName())) {
        qDebug() << "[KyConnectResourse]" <<"get ap item failed, the ap device is not exist yet";
        return nullptr;
    }
#endif

    QByteArray rawSsid = wirelessSetting->ssid();

    KyApConnectItem *apConnectItem = new KyApConnectItem();
    apConnectItem->m_connectName = connectPtr->name();
    apConnectItem->m_connectSsid = getSsidFromByteArray(rawSsid);
    apConnectItem->m_connectUuid = connectPtr->uuid();

    if (wirelessSetting->band() == NetworkManager::WirelessSetting::FrequencyBand::A) {
        apConnectItem->m_band = str5GBand;
    } else if (wirelessSetting->band() == NetworkManager::WirelessSetting::FrequencyBand::Bg) {
        apConnectItem->m_band = str2GBand;
    }
    apConnectItem->m_ifaceName = settingPtr->interfaceName();
    apConnectItem->m_isActivated = m_networkResourceInstance->isActiveConnection(connectPtr->uuid());

    //NetworkManager::WirelessSecuritySetting::Ptr wirelessSecuritySetting
    //    = settingPtr->setting(NetworkManager::Setting::WirelessSecurity).dynamicCast<NetworkManager::WirelessSecuritySetting>();
    KyWirelessConnectOperation wirelessOperation;
    apConnectItem->m_password = wirelessOperation.getPsk(apConnectItem->m_connectUuid);// wirelessSecuritySetting->psk();

    return apConnectItem;
}

KyApConnectItem *KyConnectResourse::getApConnectionByUuid(QString connectUuid)
{
    NetworkManager::Connection::Ptr connectPtr = nullptr;

    connectPtr = m_networkResourceInstance->getConnect(connectUuid);
    if (nullptr == connectPtr) {
        return nullptr;
    }

    if (NetworkManager::ConnectionSettings::ConnectionType::Wireless
            != connectPtr->settings()->connectionType()) {
        return nullptr;
    }

    KyApConnectItem *connectItem = getApConnectItem(connectPtr);

    return connectItem;
}

QString KyConnectResourse::getApConnectionPathByUuid(QString connectUuid)
{
    NetworkManager::Connection::Ptr connectPtr = nullptr;

    connectPtr = m_networkResourceInstance->getConnect(connectUuid);
    if (nullptr == connectPtr) {
        return nullptr;
    }

    return connectPtr->path();
}

void KyConnectResourse::getApConnections(QList<KyApConnectItem *> &apConnectItemList)
{
    QList<KyApConnectItem *> apActiveConnectItemList;
    QList<KyApConnectItem *> apDeactiveConnectItemList;
    int index = 0;
    NetworkManager::Connection::List connectList;

    qDebug()<<"[KyConnectResourse]"<<"get ap connections";

    connectList.clear();
    connectList = m_networkResourceInstance->getConnectList();

    if (connectList.empty()) {
        qWarning()<<"[KyConnectResourse]"<<"get ap connections failed, the connect list is empty";
        return;
    }

    NetworkManager::Connection::Ptr connectPtr = nullptr;
    for (index = 0; index < connectList.size(); index++) {
        connectPtr = connectList.at(index);
        if (NetworkManager::ConnectionSettings::ConnectionType::Wireless
                != connectPtr->settings()->connectionType()) {
            qDebug()<<"[KyConnectResourse]"<<"connect name:" << connectPtr->name()
                   <<"connect type:"<<connectPtr->settings()->connectionType();
            continue;
        }

        KyApConnectItem *connectItem = getApConnectItem(connectPtr);
        if (nullptr != connectItem) {
            if (connectItem->m_isActivated) {
                apActiveConnectItemList << connectItem;
            } else {
                apDeactiveConnectItemList << connectItem;
            }
        }

        connectPtr = nullptr;
    }

    apConnectItemList << apActiveConnectItemList << apDeactiveConnectItemList;

    return;
}

bool KyConnectResourse::isVirtualConncection(QString uuid)
{
    NetworkManager::Connection::Ptr connectPtr = nullptr;

    connectPtr = m_networkResourceInstance->getConnect(uuid);
    if (nullptr == connectPtr) {
        return false;
    }

    if (NetworkManager::ConnectionSettings::ConnectionType::Vpn == connectPtr->settings()->connectionType()
      ||NetworkManager::ConnectionSettings::ConnectionType::Bond == connectPtr->settings()->connectionType()
      ||NetworkManager::ConnectionSettings::ConnectionType::Bridge == connectPtr->settings()->connectionType()
      ||NetworkManager::ConnectionSettings::ConnectionType::Vlan == connectPtr->settings()->connectionType()
      ||NetworkManager::ConnectionSettings::ConnectionType::Team == connectPtr->settings()->connectionType()
      ||NetworkManager::ConnectionSettings::ConnectionType::IpTunnel == connectPtr->settings()->connectionType()) {
        return true;
    }

    NetworkManager::Device::Ptr devicePtr = nullptr;
    devicePtr = m_networkResourceInstance->findDeviceInterface(connectPtr->settings()->interfaceName());
    if (devicePtr.isNull()) {
        return false;
    }
    if (devicePtr->udi().startsWith("/sys/devices/virtual/net")) {
        return true;
    }

    return false;
}

bool KyConnectResourse::isWiredConnection(QString uuid)
{
    NetworkManager::Connection::Ptr connectPtr =
                      m_networkResourceInstance->getConnect(uuid);
    if (connectPtr.isNull()) {
        qWarning()<<"[KyConnectResourse]"<<"can not find wired connection"<<uuid;
        return false;
    }

    /*
    * 由于通过NetworkManager库获取的连接类型，准确的说是判断有线连接类型有时候是不准确的，因为该库
    * 对连接类型支持的不全，对于不支持的类型默认是有线连接类型，比如说wifi-p2p的投屏，所以需要通过
    * networkmanager的dbus接口获取。
    */

    QString connectPath = connectPtr->path();
    QString connectionType = getConnectTypeByDbus(connectPath);
    if (ETHERNET_TYPE == connectionType) {
        return true;
    }

    return false;
}

bool KyConnectResourse::isWirelessConnection(QString uuid)
{
    NetworkManager::Connection::Ptr connectPtr =
                        m_networkResourceInstance->getConnect(uuid);
    if (connectPtr.isNull()) {
        qWarning()<<"[KyConnectResourse]"<<"can not find wireless connection"<<uuid;
        return false;
    }


    if (connectPtr->isValid()) {
        NetworkManager::ConnectionSettings::Ptr connectSettingPtr = connectPtr->settings();

        if (connectSettingPtr.isNull()) {
            qWarning()<<"[KyConnectResourse]"<<"get connect setting failed, connect uuid"<<uuid;
            return false;
        }

        if (NetworkManager::ConnectionSettings::ConnectionType::Wireless ==
                connectPtr->settings()->connectionType()) {
            return true;
        }
    }

    return false;
}

bool KyConnectResourse::isActivatedConnection(QString uuid)
{
    return m_networkResourceInstance->isActiveConnection(uuid);
}

bool KyConnectResourse::isApConnection(QString uuid)
{
    NetworkManager::Connection::Ptr connectPtr = nullptr;

    connectPtr = m_networkResourceInstance->getConnect(uuid);
    if (nullptr == connectPtr) {
        return false;
    }

    if (NetworkManager::ConnectionSettings::ConnectionType::Wireless
            != connectPtr->settings()->connectionType()) {
        return false;
    }

    NetworkManager::ConnectionSettings::Ptr settingPtr = connectPtr->settings();
    NetworkManager::WirelessSetting::Ptr wirelessSetting
        = settingPtr->setting(NetworkManager::Setting::Wireless).dynamicCast<NetworkManager::WirelessSetting>();
    if (NetworkManager::WirelessSetting::NetworkMode::Ap
                                    != wirelessSetting->mode()) {
        return false;
    }

    return true;
}
