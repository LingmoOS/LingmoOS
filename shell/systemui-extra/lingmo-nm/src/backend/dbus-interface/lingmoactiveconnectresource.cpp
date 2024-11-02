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
#include "lingmoutil.h"
#include "lingmoactiveconnectresource.h"
#include "kywirelessconnectoperation.h"

#include <NetworkManagerQt/IpConfig>
#include <NetworkManagerQt/Dhcp4Config>
#include <NetworkManagerQt/Dhcp6Config>

#define LOG_FLAG "[KyActiveConnectResourse]"

KyActiveConnectResourse::KyActiveConnectResourse(QObject *parent) : QObject(parent)
{
    m_networkResourceInstance = KyNetworkResourceManager::getInstance();
    m_networkdevice = new KyNetworkDeviceResourse();

    connect(m_networkResourceInstance, &KyNetworkResourceManager::activeConnectionRemove,
                                       this, &KyActiveConnectResourse::activeConnectRemove);
    connect(m_networkResourceInstance, &KyNetworkResourceManager::activeConnectStateChangeReason,
                                       this, &KyActiveConnectResourse::stateChangeReason);
    connect(m_networkResourceInstance, &KyNetworkResourceManager::vpnActiveConnectStateChangeReason,
                                       this, &KyActiveConnectResourse::vpnConnectChangeReason);
}

KyActiveConnectResourse::~KyActiveConnectResourse()
{
    m_networkResourceInstance = nullptr;
    if (nullptr != m_networkdevice) {
        delete m_networkdevice;
        m_networkdevice = nullptr;
    }
}

KyConnectItem *KyActiveConnectResourse::getActiveConnectionItem(NetworkManager::ActiveConnection::Ptr activeConnectPtr)
{
    qDebug()<<"[KyActiveConnectResourse]"<<"get active connect item";

    if (nullptr == activeConnectPtr) {
        qWarning()<<"[KyActiveConnectResourse]"<<"the active connect is empty";
        return nullptr;
    }

    if (NetworkManager::ActiveConnection::State::Activated != activeConnectPtr->state()) {
        qWarning()<<"[KyActiveConnectResourse]"<<"the active connect is not activated"
                 <<"connect name:"<<activeConnectPtr->connection()->name()
                 <<"connect state"<< activeConnectPtr->state();
        return nullptr;
    }

    KyConnectItem *activeConnectItem = new KyConnectItem();
    activeConnectItem->m_connectUuid = activeConnectPtr->uuid();

    NetworkManager::Connection::Ptr connectPtr = activeConnectPtr->connection();
    activeConnectItem->m_connectName = connectPtr->name();
    activeConnectItem->m_connectPath = connectPtr->path();

    activeConnectItem->m_connectState = NetworkManager::ActiveConnection::State::Activated;

    return activeConnectItem;
}

KyConnectItem *KyActiveConnectResourse::getActiveConnectionByUuid(QString connectUuid)
{
    NetworkManager::ActiveConnection::List activeConnectList;
    activeConnectList.clear();
    activeConnectList = m_networkResourceInstance->getActiveConnectList();

    if (activeConnectList.empty()) {
        qWarning()<<"[KyActiveConnectResourse]"<<"the active connect list is empty";
        return nullptr;
    }
    //可能存在已无效的ActiveConnection，所以使用uuid遍历处理需要满足device不为空且ActiveConnection状态为已连接
    NetworkManager::ActiveConnection::Ptr activeConnectPtr = nullptr;
    KyConnectItem *activeConnectItem = nullptr;
    bool isFind = false;
    for (int index = 0; index < activeConnectList.size(); index++) {
        activeConnectPtr = activeConnectList.at(index);
        if (activeConnectPtr.isNull()) {
            continue;
        }

        if (connectUuid != activeConnectPtr->uuid()) {
            continue;
        }

        activeConnectItem = getActiveConnectionItem(activeConnectPtr);

        if (nullptr == activeConnectItem || activeConnectPtr->devices().isEmpty()) {
            continue;
        } else {
           isFind = true;
           break;
        }
    }

    if (!isFind) {
        return nullptr;
    }

    QString ifaceUni = activeConnectPtr->devices().at(0);
    NetworkManager::Device:: Ptr devicePtr =
                m_networkResourceInstance->findDeviceUni(ifaceUni);
    activeConnectItem->m_ifaceName = devicePtr->interfaceName();
    activeConnectItem->m_itemType = activeConnectPtr->type();

    return activeConnectItem;
}

KyConnectItem *KyActiveConnectResourse::getActiveConnectionByUuid(QString connectUuid,
                                                                  QString deviceName)
{
    NetworkManager::ActiveConnection::Ptr activeConnectPtr =
            m_networkResourceInstance->getActiveConnect(connectUuid);

    if (nullptr == activeConnectPtr) {
        qWarning()<< "[KyActiveConnectResourse]" <<"it can not find connect "<< connectUuid;
        return nullptr;
    }

    QStringList interfaces = activeConnectPtr->devices();
    for (int index = 0; index < interfaces.size(); ++index) {
        QString ifaceUni = interfaces.at(index);
        NetworkManager::Device:: Ptr devicePtr =
                    m_networkResourceInstance->findDeviceUni(ifaceUni);
        if (devicePtr.isNull()) {
            continue;
        }

        if (devicePtr->interfaceName() == deviceName) {
            KyConnectItem *activeConnectItem =
                   getActiveConnectionItem(activeConnectPtr);
            if (nullptr == activeConnectItem) {
                return nullptr;
            }
            activeConnectItem->m_ifaceName = deviceName;
            activeConnectItem->m_itemType = activeConnectPtr->type();
            return activeConnectItem;
        }
    }

    return nullptr;
}

void KyActiveConnectResourse::getActiveConnectionList(QString deviceName,
                             NetworkManager::ConnectionSettings::ConnectionType connectionType,
                             QList<KyConnectItem *> &activeConnectItemList)
{
    qDebug()<<"[KyActiveConnectResourse]"<<"get activate connect for device"
                    << deviceName <<"connect type:"<<connectionType;

    NetworkManager::ActiveConnection::List activeConnectList;
    activeConnectList.clear();
    activeConnectList = m_networkResourceInstance->getActiveConnectList();

    if (activeConnectList.empty()) {
        qWarning()<<"[KyActiveConnectResourse]"<<"the active connect list is empty";
        return;
    }

    NetworkManager::ActiveConnection::Ptr activeConnectPtr = nullptr;
    for (int index = 0; index < activeConnectList.size(); index++) {
        activeConnectPtr = activeConnectList.at(index);
        if (activeConnectPtr.isNull()) {
            continue;
        }

        if (connectionType != activeConnectPtr->type()) {
            qDebug()<<"[KyActiveConnectResourse]" <<"the connect type " << activeConnectPtr->type()
                   <<"connect name" << activeConnectPtr->connection()->name();
            continue;
        }

        QStringList interfaces = activeConnectPtr->devices();
        for (int index = 0; index < interfaces.size(); ++index) {
            QString ifaceUni = interfaces.at(index);
            NetworkManager::Device:: Ptr devicePtr =
                        m_networkResourceInstance->findDeviceUni(ifaceUni);
            if (devicePtr->interfaceName() == deviceName) {
                KyConnectItem *activeConnectItem =
                        getActiveConnectionItem(activeConnectPtr);
                if (nullptr != activeConnectItem) {
                    activeConnectItem->m_ifaceName = deviceName;
                    activeConnectItem->m_itemType = connectionType;
                    activeConnectItemList << activeConnectItem;
                    //activeConnectItem->dumpInfo();
                }

                activeConnectPtr = nullptr;
                break;
            }
        }
    }

    return;
}

#if 0
void KyActiveConnectResourse::getWiredActivateConnect(QList<KyWiredConnectItem *> &wiredActiveConnectItemList)
{
    int index = 0;
    NetworkManager::ActiveConnection::List activeConnectList;
    qDebug()<<"[KyActiveConnectResourse]"<<"get wired activate connect";

    activeConnectList.clear();
    activeConnectList = m_networkResourceInstance->getActiveConnectList();

    if (activeConnectList.empty()) {
        qWarning()<<"[KyActiveConnectResourse]"<<"the active connect list is empty";
        return;
    }

    NetworkManager::ActiveConnection::Ptr activeConnectPtr = nullptr;
    for (index = 0; index < activeConnectList.size(); index++) {
        activeConnectPtr = activeConnectList.at(index);
        if (NetworkManager::ConnectionSettings::ConnectionType::Wired
                != activeConnectPtr->type()) {
            continue;
        }

        KyWiredConnectItem *activeConnectItem =
                getWiredActiveConnectItem(activeConnectPtr);
        if (nullptr != activeConnectItem) {
            wiredActiveConnectItemList << activeConnectItem;
            activeConnectItem->dumpInfo();
        }

        activeConnectPtr = nullptr;
    }

    return;
}

KyWiredConnectItem *KyActiveConnectResourse::getWiredActiveConnectItem(NetworkManager::ActiveConnection::Ptr activeConnectPtr)
{
    qDebug()<<"[KyActiveConnectResourse]"<<"get active connect item";

    if (nullptr == activeConnectPtr) {
        qWarning()<<"[KyActiveConnectResourse]"<<"the active connect is empty";
        return nullptr;
    }

    if (NetworkManager::ActiveConnection::State::Activated != activeConnectPtr->state()) {
        qWarning()<<"[KyActiveConnectResourse]"<<"the active connect is not activated"
                 <<activeConnectPtr->connection()->name() << activeConnectPtr->state();
        return nullptr;
    }

    KyWiredConnectItem *wiredItem = new KyWiredConnectItem();
    NetworkManager::Connection::Ptr connectPtr = activeConnectPtr->connection();
    NetworkManager::ConnectionSettings::Ptr settingPtr = connectPtr->settings();

    wiredItem->m_connectName = connectPtr->name();
    qDebug() <<"[KyActiveConnectResourse]"<< "connect uuid" << connectPtr->uuid()
             << "active connect uuid" << activeConnectPtr->uuid();
    wiredItem->m_connectUuid = activeConnectPtr->uuid();
    wiredItem->m_ifaceName = settingPtr->interfaceName();


    getActiveConnectIp(activeConnectPtr, wiredItem->m_ipv4, wiredItem->m_ipv6);

    m_networkdevice->getWiredHardwareInfo(settingPtr->interfaceName(), wiredItem);

    wiredItem->m_state = NetworkManager::ActiveConnection::State::Activated;
    //wiredItem->m_itemType;

    return wiredItem;
}

#endif

void KyActiveConnectResourse::getActiveConnectIpInfo(
                        const QString &connectUuid,
                        QString &ipv4Address,
                        QString &ipv6Address)
{
    NetworkManager::ActiveConnection::Ptr activeConnectPtr =
            m_networkResourceInstance->getActiveConnect(connectUuid);

    if (activeConnectPtr.isNull()) {
        qWarning()<< "[KyActiveConnectResourse]" <<"it can not find connect "<< connectUuid;
        return;
    }

    getActiveConnectIp(activeConnectPtr, ipv4Address, ipv6Address);

    return;
}

void KyActiveConnectResourse::getActiveConnectIp(
                        NetworkManager::ActiveConnection::Ptr activeConnectPtr,
                        QString &ipv4Address,
                        QString &ipv6Address)
{
    qDebug()<<"[KyActiveConnectResourse]"<<"get active connect ip info";

    NetworkManager::IpConfig ipv4Config =activeConnectPtr->ipV4Config();
    if (ipv4Config.isValid()) {
        if (!ipv4Config.addresses().isEmpty()) {
            NetworkManager::IpAddress address = ipv4Config.addresses().at(0);
            ipv4Address = address.ip().toString();
        } else {
            qWarning()<<"[KyActiveConnectResourse]"<<"the ipv4 address is empty.";
        }
    } else {
        qWarning()<<"[KyActiveConnectResourse]"<<"ipv4 config is not valid";
    }

    NetworkManager::IpConfig ipv6Config =activeConnectPtr->ipV6Config();
    if (ipv6Config.isValid()) {
        if (!ipv6Config.addresses().isEmpty()) {
             NetworkManager::IpAddress address = ipv6Config.addresses().at(0);
             ipv6Address = address.ip().toString();
        } else {
            qWarning()<<"[KyActiveConnectResourse]"<<"ipv6 address is empty";
        }
    } else {
        qWarning()<<"[KyActiveConnectResourse]"<<"ipv6 config is not valid";
    }

    return;
}

void KyActiveConnectResourse::getActiveConnectDnsInfo(
                        const QString &connectUuid,
                        QList<QHostAddress> &ipv4Dns,
                        QList<QHostAddress> &ipv6Dns)
{
    NetworkManager::ActiveConnection::Ptr activeConnectPtr =
            m_networkResourceInstance->getActiveConnect(connectUuid);
    if (activeConnectPtr.isNull()) {
        qWarning()<< "[KyActiveConnectResourse]" <<"it can not find connect "<< connectUuid;
        return;
    }

    getActiveConnectDns(activeConnectPtr, ipv4Dns, ipv6Dns);

    return;
}

void KyActiveConnectResourse::getActiveConnectDns(
                         NetworkManager::ActiveConnection::Ptr activeConnectPtr,
                         QList<QHostAddress> &ipv4Dns,
                         QList<QHostAddress> &ipv6Dns)
{
    qDebug()<<"[KyActiveConnectResourse]"<<"get active connect nameservice info";

    NetworkManager::IpConfig ipv4Config = activeConnectPtr->ipV4Config();
    if (ipv4Config.isValid()) {
        ipv4Dns = ipv4Config.nameservers();
    } else {
        qWarning()<<"[KyActiveConnectResourse]"<<"ipv4 config is not valid";
    }

    NetworkManager::IpConfig ipv6Config =activeConnectPtr->ipV6Config();
    if (ipv6Config.isValid()) {
        ipv6Dns = ipv6Config.nameservers();
    } else {
        qWarning()<<"[KyActiveConnectResourse]"<<"ipv6 config is not valid";
    }

    return;
}

KyVpnConnectItem *KyActiveConnectResourse::getVpnActiveConnectItem(NetworkManager::ActiveConnection::Ptr activeConnectPtr)
{
    qDebug()<<"[KyActiveConnectResourse]"<<"get vpn active connect item";

    if (nullptr == activeConnectPtr) {
        qWarning()<<"[KyActiveConnectResourse]"<<"get vpn active connect failed, the active connect is empty";
        return nullptr;
    }

    if (NetworkManager::ActiveConnection::State::Activated != activeConnectPtr->state()) {
        qWarning()<<"[KyActiveConnectResourse]"<<"the active connect is not activated"
                 <<activeConnectPtr->connection()->name() << activeConnectPtr->state();
        return nullptr;
    }

    KyVpnConnectItem *vpnItem = new KyVpnConnectItem();
    NetworkManager::Connection::Ptr connectPtr = activeConnectPtr->connection();
    vpnItem->m_vpnName = connectPtr->name();

    vpnItem->m_vpnUuid = activeConnectPtr->uuid();

    NetworkManager::ConnectionSettings::Ptr settingsPtr = connectPtr->settings();
    NetworkManager::VpnSetting::Ptr vpnSetting = settingsPtr->setting(NetworkManager::Setting::Vpn).dynamicCast<NetworkManager::VpnSetting>();
    NMStringMap vpnDataMap = vpnSetting->data();
    if (vpnDataMap.isEmpty()) {
        qWarning()<<"[KyActiveConnectResourse]"<<"get vpn connection Data failed, the data is empty";
    } else {
        vpnItem->m_vpnGateWay = vpnDataMap["gateway"];
        vpnItem->m_vpnUser = vpnDataMap["user"];
        if ( "yes" == vpnDataMap["require-mppe"]) {
            vpnItem->m_vpnMppe = true;
        } else {
            vpnItem->m_vpnMppe = false;
            qDebug()<<"[KyActiveConnectResourse]"<<"vpn mppe required:"<< vpnDataMap["require-mppe"];
        }
    }

    getActiveConnectIp(activeConnectPtr, vpnItem->m_vpnIpv4Address, vpnItem->m_vpnIpv6Address);

    NetworkManager::VpnConnection *p_vpnConnect =  qobject_cast<NetworkManager::VpnConnection *>(activeConnectPtr.data());
    vpnItem->m_vpnState = p_vpnConnect->state();

    return vpnItem;
}

void KyActiveConnectResourse::getVpnActivateConnect(QList<KyVpnConnectItem *> &vpnActiveConnectItemList)
{
    int index = 0;
    NetworkManager::ActiveConnection::List activeConnectList;

    activeConnectList.clear();
    activeConnectList = m_networkResourceInstance->getActiveConnectList();

    if (activeConnectList.empty()) {
        qWarning()<<"[KyActiveConnectResourse]"<<"get vpn active connect failed, the active connect list is empty";
        return;
    }

    NetworkManager::ActiveConnection::Ptr activeConnectPtr = nullptr;
    for (index = 0; index < activeConnectList.size(); index++) {
        activeConnectPtr = activeConnectList.at(index);
        if (activeConnectPtr.isNull()) {
            continue;
        }

        if (!activeConnectPtr->vpn()) {
            continue;
        }

        KyVpnConnectItem *activeConnectItem =
                getVpnActiveConnectItem(activeConnectPtr);
        if (nullptr != activeConnectItem) {
            vpnActiveConnectItemList << activeConnectItem;
        }

        activeConnectPtr = nullptr;
    }

    return;
}

KyBluetoothConnectItem *KyActiveConnectResourse::getBtActiveConnectItem(NetworkManager::ActiveConnection::Ptr activeConnectPtr)
{
    qDebug()<<"[KyActiveConnectResourse]"<<"get bluetooth active connect item";

    if (nullptr == activeConnectPtr) {
        qWarning()<<"[KyActiveConnectResourse]"<<"get bluetooth item failed, the active connect is empty";
        return nullptr;
    }

    if (NetworkManager::ActiveConnection::State::Activated != activeConnectPtr->state()) {
        qWarning()<<"[KyActiveConnectResourse]"<<"get bluetooth item failed, the active connect is not activated"
                 <<activeConnectPtr->connection()->name() << activeConnectPtr->state();
        return nullptr;
    }

    KyBluetoothConnectItem *bluetoothItem = new KyBluetoothConnectItem();
    NetworkManager::Connection::Ptr connectPtr = activeConnectPtr->connection();

    bluetoothItem->m_connectName = connectPtr->name();
    bluetoothItem->m_connectUuid = activeConnectPtr->uuid();
    bluetoothItem->m_state = NetworkManager::ActiveConnection::State::Activated;

    getActiveConnectIp(activeConnectPtr, bluetoothItem->m_ipv4Address, bluetoothItem->m_ipv6Address);

    NetworkManager::ConnectionSettings::Ptr settingPtr = connectPtr->settings();
    NetworkManager::BluetoothSetting::Ptr bluetoothSetting =
            settingPtr->setting(NetworkManager::Setting::Bluetooth).dynamicCast<NetworkManager::BluetoothSetting>();
    bluetoothItem->m_deviceAddress = bluetoothSetting->bluetoothAddress();
    QByteArray btAddrArray = bluetoothSetting->bluetoothAddress();
    for (int index = 0; index < btAddrArray.size(); ++index) {
        qDebug("bt address %d %s", index, btAddrArray[index]);
    }
    qDebug()<<"bluetooth device address"<<bluetoothItem->m_deviceAddress;
    //wiredItem->m_itemType;

    return bluetoothItem;
}

void KyActiveConnectResourse::getBtActivateConnect(QList<KyBluetoothConnectItem *> &btActiveConnectItemList)
{
    int index = 0;
    NetworkManager::ActiveConnection::List activeConnectList;

    activeConnectList.clear();
    activeConnectList = m_networkResourceInstance->getActiveConnectList();

    if (activeConnectList.empty()) {
        qWarning()<<"[KyActiveConnectResourse]"<<"get bluetooth active connect failed, the active connect list is empty";
        return;
    }

    NetworkManager::ActiveConnection::Ptr activeConnectPtr = nullptr;
    for (index = 0; index < activeConnectList.size(); index++) {
        activeConnectPtr = activeConnectList.at(index);
        if (activeConnectPtr.isNull()) {
            continue;
        }

        if (NetworkManager::ConnectionSettings::ConnectionType::Bluetooth
                != activeConnectPtr->type()) {
            continue;
        }

        KyBluetoothConnectItem *activeConnectItem = getBtActiveConnectItem(activeConnectPtr);
        if (nullptr != activeConnectItem) {
            btActiveConnectItemList << activeConnectItem;
        }

        activeConnectPtr = nullptr;
    }

    return;
}

KyApConnectItem *KyActiveConnectResourse::getApActiveConnectItem(NetworkManager::ActiveConnection::Ptr activeConnectPtr)
{
    qDebug()<<"[KyActiveConnectResourse]"<<"get wireless ap active connect item";

    if (nullptr == activeConnectPtr) {
        qWarning()<<"[KyActiveConnectResourse]"<<"get wireless ap item failed, the active connect is empty";
        return nullptr;
    }

    if (NetworkManager::ActiveConnection::State::Activated != activeConnectPtr->state()) {
        qWarning()<<"[KyActiveConnectResourse]"<<"get ap item failed, the active connect is not activated"
                 <<activeConnectPtr->connection()->name() << activeConnectPtr->state();
        return nullptr;
    }

    NetworkManager::Connection::Ptr connectPtr = activeConnectPtr->connection();
    NetworkManager::ConnectionSettings::Ptr settingPtr = connectPtr->settings();
    NetworkManager::WirelessSetting::Ptr wirelessSetting
        = settingPtr->setting(NetworkManager::Setting::Wireless).dynamicCast<NetworkManager::WirelessSetting>();
    if (NetworkManager::WirelessSetting::NetworkMode::Ap
                                    != wirelessSetting->mode()) {
        qDebug() << "[KyActiveConnectResourse]" <<"get ap item failed, the active connect mode is not ap.";
        return nullptr;
    }

    KyApConnectItem *apConnectItem = new KyApConnectItem();
    apConnectItem->m_connectName = connectPtr->name();
    apConnectItem->m_connectUuid = activeConnectPtr->uuid();
    apConnectItem->m_ifaceName = settingPtr->interfaceName();

    QByteArray rawSsid = wirelessSetting->ssid();
    apConnectItem->m_connectSsid = getSsidFromByteArray(rawSsid);


    KyWirelessConnectOperation wirelessOperation;
    apConnectItem->m_password = wirelessOperation.getPsk(apConnectItem->m_connectUuid);

    return apConnectItem;
}

void KyActiveConnectResourse::getApActivateConnect(QList<KyApConnectItem *> &apConnectItemList)
{
    int index = 0;
    NetworkManager::ActiveConnection::List activeConnectList;

    activeConnectList.clear();
    activeConnectList = m_networkResourceInstance->getActiveConnectList();

    if (activeConnectList.empty()) {
        qWarning()<<"[KyActiveConnectResourse]"
                 <<"get ap active connect failed, the active connect list is empty";
        return;
    }

    NetworkManager::ActiveConnection::Ptr activeConnectPtr = nullptr;
    for (index = 0; index < activeConnectList.size(); index++) {
        activeConnectPtr = activeConnectList.at(index);
        if (activeConnectPtr.isNull()) {
            continue;
        }

        if (NetworkManager::ConnectionSettings::ConnectionType::Wireless
                != activeConnectPtr->type()) {
            continue;
        }

        KyApConnectItem *apConnectItem = getApActiveConnectItem(activeConnectPtr);
        if (nullptr != apConnectItem) {
            apConnectItemList << apConnectItem;
        }

        activeConnectPtr = nullptr;
    }

    return;
}

QString KyActiveConnectResourse::getDeviceOfActivateConnect(QString conUuid)
{
    QString deviceName = "";

    NetworkManager::ActiveConnection::Ptr activeConnectPtr =
                                    m_networkResourceInstance->getActiveConnect(conUuid);

    if (nullptr == activeConnectPtr) {
        qWarning()<< "[KyActiveConnectResourse]" <<"it can not find connect "<< conUuid;
        return deviceName;
    }

    QStringList interfaces = activeConnectPtr->devices();
    if (interfaces.isEmpty()) {
        qWarning()<< LOG_FLAG << "get device of active connection failed.";
        return deviceName;
    }

    QString ifaceUni = interfaces.at(0);
    NetworkManager::Device:: Ptr devicePtr =
              m_networkResourceInstance->findDeviceUni(ifaceUni);
    deviceName = devicePtr->interfaceName();

    return deviceName;
}

bool KyActiveConnectResourse::connectionIsVirtual(QString uuid)
{
    NetworkManager::ActiveConnection::Ptr activeConnectPtr =
                    m_networkResourceInstance->getActiveConnect(uuid);

    if (activeConnectPtr.isNull()) {
        qWarning()<< LOG_FLAG << "check connection virtual is failed.";
        return false;
    }

    QStringList interfaces = activeConnectPtr->devices();
    if (interfaces.isEmpty()) {
        qWarning()<< LOG_FLAG << "active connection get device failed.";
        return false;
    }

    QString ifaceUni = interfaces.at(0);
    NetworkManager::Device:: Ptr devicePtr =
                m_networkResourceInstance->findDeviceUni(ifaceUni);
    if (devicePtr.isNull()) {
        return false;
    }

    return !m_networkdevice->checkDeviceType(devicePtr->interfaceName(), NetworkManager::Device::Type::Ethernet);
}

bool KyActiveConnectResourse::wiredConnectIsActived()
{
    int index = 0;
    NetworkManager::ActiveConnection::List activeConnectList;

    activeConnectList.clear();
    activeConnectList = m_networkResourceInstance->getActiveConnectList();

    if (activeConnectList.empty()) {
        qWarning()<<"[KyActiveConnectResourse]"
                 <<"get active connect failed, the active connect list is empty";
        return false;
    }

    NetworkManager::ActiveConnection::Ptr activeConnectPtr = nullptr;
    for (index = 0; index < activeConnectList.size(); index++) {
        activeConnectPtr = activeConnectList.at(index);
        if (activeConnectPtr.isNull()) {
            continue;
        }

        if (NetworkManager::ConnectionSettings::ConnectionType::Wired
                != activeConnectPtr->type()) {
            continue;
        }

        if (connectionIsVirtual(activeConnectPtr->uuid())) {
            continue;
        }

        if (activeConnectPtr->state() == NetworkManager::ActiveConnection::State::Activated) {
            return true;
        }
    }

    return false;
}

bool KyActiveConnectResourse::checkWirelessStatus(NetworkManager::ActiveConnection::State state)
{
    int index = 0;
    NetworkManager::ActiveConnection::List activeConnectList;

    activeConnectList.clear();
    activeConnectList = m_networkResourceInstance->getActiveConnectList();

    if (activeConnectList.empty()) {
        qWarning()<<"[KyActiveConnectResourse]"
                 <<"get active connect failed, the active connect list is empty";
        return false;
    }

    NetworkManager::ActiveConnection::Ptr activeConnectPtr = nullptr;
    for (index = 0; index < activeConnectList.size(); index++) {
        activeConnectPtr = activeConnectList.at(index);
        if (activeConnectPtr.isNull()) {
            continue;
        }

        if (NetworkManager::ConnectionSettings::ConnectionType::Wireless
                != activeConnectPtr->type()) {
            continue;
        }

        if (activeConnectPtr->state() == state) {
            return true;
        }
    }

    return false;

}

QString KyActiveConnectResourse::getAcitveConnectionPathByUuid(QString connectUuid)
{
    NetworkManager::ActiveConnection::Ptr activeAonnectPtr = nullptr;

    activeAonnectPtr = m_networkResourceInstance->getActiveConnect(connectUuid);
    if (nullptr == activeAonnectPtr) {
        return nullptr;
    }

    return activeAonnectPtr->path();
}

int KyActiveConnectResourse::getActivateWifiSignal(QString devName)
{
    int signalStrength = -1;
    KyNetworkDeviceResourse devResource;
    QStringList devList;
    devResource.getNetworkDeviceList(NetworkManager::Device::Type::Wifi, devList);

    for (int i = 0; i < devList.size(); ++i) {

        NetworkManager::Device::Ptr connectDevice =
                            m_networkResourceInstance->findDeviceInterface(devList.at(i));

        if (nullptr == connectDevice || !connectDevice->isValid()) {
            qWarning()<< LOG_FLAG <<"getDeviceActiveAPInfo failed, the device" << devList.at(i) << "is not existed";
            continue;
        }

        if (connectDevice->type() == NetworkManager::Device::Wifi) {
            if ((!devName.isEmpty() && connectDevice->interfaceName() == devName)
                    || devName.isEmpty()) {
                NetworkManager::WirelessDevice *wirelessDevicePtr =
                    qobject_cast<NetworkManager::WirelessDevice *>(connectDevice.data());
                NetworkManager::AccessPoint::Ptr apPtr = wirelessDevicePtr->activeAccessPoint();
                if (apPtr.isNull()) {
                    continue;
                }
                signalStrength = apPtr->signalStrength();
                break;
            }
        }
    }

    return signalStrength;
}
