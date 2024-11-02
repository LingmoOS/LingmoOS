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
#include "kywirelessnetresource.h"
#include "lingmoutil.h"

#define LOG_FLAG "[KyWirelessNetResource]"
const QString ENTERPRICE_TYPE = "802.1X";
const QString WPA1_AND_WPA2 = "WPA";
const QString WPA3 = "WPA3";

static bool subWifiListSort(const KyWirelessNetItem info1, const KyWirelessNetItem info2)
{
    if (info1.m_isConfigured == info2.m_isConfigured) {
        if (info1.m_signalStrength != info2.m_signalStrength) {
            return info1.m_signalStrength > info2.m_signalStrength;
        } else {
            if (QString::compare(info1.m_NetSsid, info2.m_NetSsid, Qt::CaseInsensitive) > 0) {
                return false;
            } else {
                return true;
            }
        }
    }
    return info1.m_isConfigured;
}

static void wifiListSort(QList<KyWirelessNetItem> &list)
{
    qSort(list.begin(), list.end(), subWifiListSort);
}

KyWirelessNetResource::KyWirelessNetResource(QObject *parent)
    : QObject(parent)
{
    qDebug()<< LOG_FLAG <<"KyWirelessNetResource";

    qRegisterMetaType<KyWirelessNetItem>("KyWirelessNetItem&");

    m_networkResourceInstance = KyNetworkResourceManager::getInstance();
    m_operation = new KyWirelessConnectOperation(this);
    m_networkDevice = new KyNetworkDeviceResourse(this);

    kyWirelessNetItemListInit();

    //TODO:connect device signal
    connect(m_networkResourceInstance, &KyNetworkResourceManager::wifiNetworkAdded,
                     this, &KyWirelessNetResource::onWifiNetworkAdded/*, Qt::ConnectionType::DirectConnection*/);
    connect(m_networkResourceInstance, &KyNetworkResourceManager::wifiNetworkRemoved,
                     this, &KyWirelessNetResource::onWifiNetworkRemoved/*, Qt::ConnectionType::DirectConnection*/);
    connect(m_networkResourceInstance, &KyNetworkResourceManager::wifiNetworkPropertyChange,
                     this, &KyWirelessNetResource::onWifiNetworkPropertyChange/*, Qt::ConnectionType::DirectConnection*/);
    connect(m_networkResourceInstance, &KyNetworkResourceManager::wifiNetworkSecuChange,
                     this, &KyWirelessNetResource::onWifiNetworkSecuChange/*, Qt::ConnectionType::DirectConnection*/);
    connect(m_networkResourceInstance, &KyNetworkResourceManager::wifiNetworkDeviceDisappear,
                     this, &KyWirelessNetResource::onWifiNetworkDeviceDisappear/*, Qt::ConnectionType::DirectConnection*/);

    connect(m_networkResourceInstance, &KyNetworkResourceManager::connectionAdd,
                     this, &KyWirelessNetResource::onConnectionAdd);
    connect(m_networkResourceInstance, &KyNetworkResourceManager::connectionRemove,
                     this, &KyWirelessNetResource::onConnectionRemove);
    connect(m_networkResourceInstance, &KyNetworkResourceManager::connectionUpdate,
                     this, &KyWirelessNetResource::onConnectionUpdate);

    connect(m_networkDevice, &KyNetworkDeviceResourse::deviceAdd, this, &KyWirelessNetResource::onDeviceAdd);
    connect(m_networkDevice, &KyNetworkDeviceResourse::deviceRemove, this, &KyWirelessNetResource::onDeviceRemove);
    connect(m_networkDevice, &KyNetworkDeviceResourse::deviceNameUpdate, this, &KyWirelessNetResource::onDeviceNameUpdate);
}

KyWirelessNetResource::~KyWirelessNetResource()
{
    m_networkResourceInstance = nullptr;
}

bool KyWirelessNetResource::getAllDeviceWifiNetwork(QMap<QString, QList<KyWirelessNetItem>> &map)
{
//    onWifiNetworkDeviceDisappear();
    if (m_WifiNetworkList.isEmpty()) {
        return false;
    } else {
        QMap<QString, QList<KyWirelessNetItem> >::iterator iter = m_WifiNetworkList.begin();
        while (iter != m_WifiNetworkList.end()) {
            wifiListSort(m_WifiNetworkList[iter.key()]);
            iter++;
        }
        map = m_WifiNetworkList;
        return true;
    }
}


bool KyWirelessNetResource::getDeviceWifiNetwork(QString devIfaceName, QList<KyWirelessNetItem> &wirelessNetResource)
{
   if (!m_WifiNetworkList.contains(devIfaceName)) {
        return false;
    } else {
        wifiListSort(m_WifiNetworkList[devIfaceName]);
        wirelessNetResource = m_WifiNetworkList[devIfaceName];
        return true;
    }
}

bool KyWirelessNetResource::getWifiNetwork(const QString &devIfaceName,
                                           const QString &ssid,
                                           KyWirelessNetItem &wirelessNetResource)
{
    if (!m_WifiNetworkList.contains(devIfaceName)) {
         qDebug()<< LOG_FLAG << "getWifiNetwork fail, not contain " << devIfaceName;
        return false;
    } else {
        for (int index = 0; index < m_WifiNetworkList[devIfaceName].size(); index++){
            if (m_WifiNetworkList[devIfaceName].at(index).m_NetSsid == ssid) {
                wirelessNetResource = m_WifiNetworkList[devIfaceName].at(index);
                qDebug()<< LOG_FLAG << "getWifiNetwork success";
                return true;
            }
        }
    }

    qDebug()<< LOG_FLAG << "getWifiNetwork fail, not contain " << ssid;

    return false;
}

void KyWirelessNetResource::getWirelessActiveConnection(NetworkManager::ActiveConnection::State state, QMap<QString, QStringList> &map)
{
    int index = 0;
    map.clear();
    NetworkManager::ActiveConnection::List activeConnectionList;

    activeConnectionList.clear();
    activeConnectionList = m_networkResourceInstance->getActiveConnectList();
    if (activeConnectionList.isEmpty()) {
        return;
    }

    NetworkManager::ActiveConnection::Ptr activeConnectionPtr = nullptr;
    for (; index < activeConnectionList.size(); index++) {
        activeConnectionPtr = activeConnectionList.at(index);
        if (activeConnectionPtr.isNull()) {
            continue;
        }

        if (NetworkManager::ConnectionSettings::ConnectionType::Wireless != activeConnectionPtr->type()) {
            continue;
        }

        if (state != activeConnectionPtr->state()) {
            continue;
        }

        qDebug()<< LOG_FLAG << "getWirelessActiveConnection " << activeConnectionPtr->uuid();
        QString ssid;
        QString ifaceName = getDeviceIFace(activeConnectionPtr, ssid);
        if(ifaceName.isEmpty() || ssid.isNull()) {
            continue;
        }

        if (map.contains(ifaceName)) {
            map[ifaceName].append(activeConnectionPtr->uuid());
        } else {
            QStringList list;
            list.append(activeConnectionPtr->uuid());
            map.insert(ifaceName,list);
        }
    }

    return;
}

QString KyWirelessNetResource::getActiveConnectSsidByDevice(QString deviceName)
{
    QString ssid = "";

    NetworkManager::ActiveConnection::List activeConnectionList;
    activeConnectionList.clear();
    activeConnectionList = m_networkResourceInstance->getActiveConnectList();
    if (activeConnectionList.isEmpty()) {
        return ssid;
    }

    NetworkManager::ActiveConnection::Ptr activeConnectionPtr = nullptr;
    for (int index = 0; index < activeConnectionList.size(); index++) {
        activeConnectionPtr = activeConnectionList.at(index);
        if (activeConnectionPtr.isNull()) {
            continue;
        }

        if (NetworkManager::ConnectionSettings::ConnectionType::Wireless != activeConnectionPtr->type()) {
            continue;
        }

        if (NetworkManager::ActiveConnection::State::Activated != activeConnectionPtr->state()) {
            continue;
        }

        QStringList interfaces = activeConnectionPtr->devices();
        if(interfaces.isEmpty()) {
            qWarning()<< LOG_FLAG <<"get active device failed.";
            continue;
        }

        QString ifaceUni = interfaces.at(0);
        NetworkManager::Device:: Ptr devicePtr =
                    m_networkResourceInstance->findDeviceUni(ifaceUni);
        if (deviceName != devicePtr->interfaceName()) {
            continue;
        }

        NetworkManager::Connection::Ptr connectPtr = activeConnectionPtr->connection();
        NetworkManager::ConnectionSettings::Ptr settingPtr = connectPtr->settings();
        NetworkManager::WirelessSetting::Ptr wirelessSettingPtr =
                settingPtr->setting(NetworkManager::Setting::Wireless).dynamicCast<NetworkManager::WirelessSetting>();

        QByteArray rawSsid = wirelessSettingPtr->ssid();
        ssid = getSsidFromByteArray(rawSsid);

        break;
    }

    return ssid;
}

bool KyWirelessNetResource::getActiveWirelessNetItem(QString deviceName, KyWirelessNetItem &wirelessNetItem)
{
    if (!m_WifiNetworkList.contains(deviceName)) {
         qDebug() << "getWifiNetwork fail,not contain " << deviceName;
        return false;
    }

    QString ssid = getActiveConnectSsidByDevice(deviceName);
    if (ssid.isEmpty()) {
        return false;
    }

    for (int index = 0; index < m_WifiNetworkList[deviceName].size(); index ++) {
        if (m_WifiNetworkList[deviceName].at(index).m_NetSsid  == ssid) {
            wirelessNetItem = m_WifiNetworkList[deviceName].at(index);
            updatewirelessItemConnectInfo(wirelessNetItem);
            qDebug()<< LOG_FLAG << "getWifiNetwork success";
            return true;
        }
    }

    return false;
}

QString KyWirelessNetResource::getDeviceIFace(NetworkManager::ActiveConnection::Ptr actConn,
                                              QString &wirelessNetResourcessid)
{
    if (actConn.isNull()) {
        return "";
    }

    NetworkManager::Connection::Ptr conn = actConn->connection();
    if (conn.isNull()) {
        return "";
    }

    NetworkManager::ConnectionSettings::Ptr sett = conn->settings();
    if (sett.isNull()) {
        return "";
    }

    NetworkManager::WirelessSetting::Ptr wireless_sett = sett->setting(NetworkManager::Setting::Wireless).dynamicCast<NetworkManager::WirelessSetting>();
    if (wireless_sett.isNull()) {
        return "";
    }

    QByteArray rawSsid = wireless_sett->ssid();
    wirelessNetResourcessid = getSsidFromByteArray(rawSsid);

    QStringList interfaces = actConn->devices();
    if (interfaces.isEmpty()) {
        return "";
    }

    QString ifaceUni = interfaces.at(0);
    NetworkManager::Device:: Ptr devicePtr =
                m_networkResourceInstance->findDeviceUni(ifaceUni);

    if (devicePtr.isNull()) {
        return QString();
    }

    return devicePtr->interfaceName();
}

void KyWirelessNetResource::getSsidByUuid(const QString uuid, QString &ssid)
{
    ssid.clear();
    NetworkManager::Connection::Ptr connectPtr = m_networkResourceInstance->getConnect(uuid);
    if (connectPtr.isNull()) {
        return;
    }

    NetworkManager::WirelessSetting::Ptr wireless_sett
        = connectPtr->settings()->setting(NetworkManager::Setting::Wireless).dynamicCast<NetworkManager::WirelessSetting>();
    if (wireless_sett.isNull()) {
        qDebug()<< LOG_FLAG << "don't have WirelessSetting connection";
        return;
    }

    QByteArray rawSsid = wireless_sett->ssid();
    ssid = getSsidFromByteArray(rawSsid);

    qDebug()<< LOG_FLAG << "getSsidByUuid success " << ssid;

    return;
}

void KyWirelessNetResource::getDeviceByUuid(const QString uuid, QString &deviceName)
{
    deviceName.clear();

    NetworkManager::ActiveConnection::Ptr activeConnectionPtr = m_networkResourceInstance->getActiveConnect(uuid);
    if (!activeConnectionPtr.isNull()) {
        QStringList interfaces = activeConnectionPtr->devices();
        if (interfaces.size() > 0) {
            QString ifaceUni = interfaces.at(0);
            NetworkManager::Device:: Ptr devicePtr =
                    m_networkResourceInstance->findDeviceUni(ifaceUni);
            deviceName = devicePtr->interfaceName();
            return;
        } else {
            qDebug() << LOG_FLAG << "get device of active connection failed.";
        }
    }

    NetworkManager::Connection::Ptr connectPtr = m_networkResourceInstance->getConnect(uuid);
    if (connectPtr.isNull()) {
        return;
    }

    deviceName = connectPtr->settings()->interfaceName();

    return;
}

void KyWirelessNetResource::kyWirelessNetItemListInit()
{
    qDebug()<< LOG_FLAG << "wireless net size:" << m_networkResourceInstance->m_wifiNets.size();
    for (auto const & net : m_networkResourceInstance->m_wifiNets) {
        QString devIface = getDeviceIFace(net);
        if (devIface.isEmpty()) {
            continue;
        }

        KyWirelessNetItem item(net);
        if (item.m_NetSsid.isEmpty()) {
            continue;
        }
        if (!m_WifiNetworkList.contains(devIface)){
            QList<KyWirelessNetItem> list;
            list.append(item);
            m_WifiNetworkList.insert(devIface,list);
        } else {
            m_WifiNetworkList[devIface].append(item);
        }
    }

    return;
}

QString KyWirelessNetResource::getDeviceIFace(NetworkManager::WirelessNetwork::Ptr net)
{
    if (net.isNull()) {
        return "";
    }

    QString devUni = net->device();
    NetworkManager::Device::Ptr dev = m_networkResourceInstance->findDeviceUni(devUni);
    if (dev.isNull()) {
        qDebug()<< LOG_FLAG << "KyWirelessNetResource: can't find " << net->ssid() << " find in device list";
        return "";
    }

    return dev->interfaceName();
}

void KyWirelessNetResource::onWifiNetworkAdded(QString devIfaceName, QString ssid)
{

    NetworkManager::Device::Ptr dev = m_networkResourceInstance->findDeviceInterface(devIfaceName);
    if (dev.isNull()) {
        return;
    }

    NetworkManager::WirelessDevice* w_dev = qobject_cast<NetworkManager::WirelessDevice*>(dev.data());
    NetworkManager::WirelessNetwork::Ptr wifi = w_dev->findNetwork(ssid);

    if (wifi.isNull()) {
        return;
    }

    KyWirelessNetItem item(wifi);

    if (m_WifiNetworkList.contains(devIfaceName)) {
        for (int index = 0; index < m_WifiNetworkList[devIfaceName].size(); ++index) {
            if (m_WifiNetworkList[devIfaceName].at(index).m_NetSsid == item.m_NetSsid) {
                m_WifiNetworkList[devIfaceName].removeAt(index);
                index--;
            }
        }
        m_WifiNetworkList[devIfaceName].append(item);
    } else {
        QList<KyWirelessNetItem> list;
        list.append(item);
        m_WifiNetworkList.insert(devIfaceName,list);
    }

    Q_EMIT wifiNetworkAdd(devIfaceName, item);
}

void KyWirelessNetResource::onWifiNetworkRemoved(QString devIfaceName, QString ssid)
{
    if (!m_WifiNetworkList.contains(devIfaceName)) {
        return;
    }

    for (int index = 0; index < m_WifiNetworkList.value(devIfaceName).size(); ++index) {
        if (m_WifiNetworkList[devIfaceName].at(index).m_NetSsid == ssid) {
            m_WifiNetworkList[devIfaceName].removeAt(index);
            //remove后为空则删除
            if (m_WifiNetworkList.value(devIfaceName).isEmpty()) {
                m_WifiNetworkList.remove(devIfaceName);
            }
            Q_EMIT wifiNetworkRemove(devIfaceName,ssid);
            break;
        }
    }
}

void KyWirelessNetResource::onWifiNetworkSecuChange(NetworkManager::AccessPoint *accessPointPtr)
{
    QString secuType = enumToQstring(accessPointPtr->capabilities(),
                                     accessPointPtr->wpaFlags(),
                                     accessPointPtr->rsnFlags());


    QMap<QString, QList<KyWirelessNetItem> >::iterator iter = m_WifiNetworkList.begin();
    while (iter != m_WifiNetworkList.end()) {
        QList<KyWirelessNetItem>::iterator itemIter = iter.value().begin();
         while (itemIter != iter.value().end()) {
             if (itemIter->m_NetSsid == accessPointPtr->ssid()) {
                 QString devName = iter.key();
                 itemIter->m_secuType = secuType;
                 itemIter->setKySecuType(secuType);
                 //qDebug() << "!!!!" << itemIter->m_NetSsid << itemIter->m_secuType << itemIter->m_kySecuType;
                 Q_EMIT secuTypeChange(devName, accessPointPtr->ssid(), secuType);
                 break;
             }
             itemIter++;
         }
        iter++;
    }

}

void KyWirelessNetResource::onWifiNetworkPropertyChange(QString interface, QString ssid, int signal, QString bssid, QString sec)
{

    if (m_WifiNetworkList.contains(interface)) {
        QList<KyWirelessNetItem>::iterator iter = m_WifiNetworkList[interface].begin();
        while (iter != m_WifiNetworkList[interface].end()) {
            qDebug() << iter->m_NetSsid;
            if (iter->m_NetSsid == ssid) {
                //                 qDebug()<< LOG_FLAG <<"recive properity changed signal, sender is" << iter->m_NetSsid;
                if (iter->m_signalStrength != signal) {
                    iter->m_signalStrength = signal;
                    Q_EMIT signalStrengthChange(interface, ssid, iter->m_signalStrength);
                }

                if (iter->m_bssid != bssid) {
                    qDebug() << "bssid";
                    iter->m_bssid = bssid;
                    Q_EMIT bssidChange(interface, ssid, iter->m_bssid);
                }

                if (iter->m_secuType != sec) {
                    iter->setKySecuType(sec);
                    Q_EMIT secuTypeChange(interface, ssid, sec);
                }

                break;
            }

            iter++;
        }
    }
}

void KyWirelessNetResource::onWifiNetworkDeviceDisappear()
{
    m_WifiNetworkList.clear();
    kyWirelessNetItemListInit();
}

bool KyWirelessNetResource::getEnterPriseInfoTls(QString &uuid, KyEapMethodTlsInfo &info)
{
    NetworkManager::Connection::Ptr conn = m_networkResourceInstance->getConnect(uuid);
    if (conn.isNull()) {
        qDebug()<< LOG_FLAG << "modifyEnterPriseInfoTls connection missing";
        return false;
    }

    NetworkManager::WirelessSecuritySetting::Ptr security_sett =
            conn->settings()->setting(NetworkManager::Setting::WirelessSecurity).dynamicCast<NetworkManager::WirelessSecuritySetting>();
    if (security_sett.isNull()) {
        qDebug()<< LOG_FLAG << "don't have WirelessSecurity connection";
        return false;
    }
    if (security_sett->keyMgmt() != NetworkManager::WirelessSecuritySetting::WpaEap) {
        return false;
    }
    NetworkManager::Security8021xSetting::Ptr setting =
            conn->settings()->setting(NetworkManager::Setting::Security8021x).dynamicCast<NetworkManager::Security8021xSetting>();
    if (setting.isNull()) {
        qDebug()<< LOG_FLAG << "don't have Security8021x connection";
        return false;
    }


    info.identity = setting->identity();
    info.domain = setting->domainSuffixMatch();
    info.caCertPath = setting->caCertificate();
    if (info.caCertPath.left(7) == "file://") {
        info.caCertPath = info.caCertPath.mid(7);
    }

    info.clientCertPath = setting->clientCertificate();
    if (info.clientCertPath.left(7) == "file://") {
        info.clientCertPath = info.clientCertPath.mid(7);
    }

    info.clientPrivateKey = QString(setting->privateKey());
    if (info.clientPrivateKey.left(7) == "file://") {
        info.clientPrivateKey = info.clientPrivateKey.mid(7);
    }

    info.m_privateKeyPWDFlag = setting->privateKeyPasswordFlags();
    if (!info.m_privateKeyPWDFlag) {
        info.clientPrivateKeyPWD = m_operation->getPrivateKeyPassword(conn->uuid());
    }

    return true;
}

bool KyWirelessNetResource::getEnterPriseInfoPeap(QString &uuid, KyEapMethodPeapInfo &info)
{
    NetworkManager::Connection::Ptr conn = m_networkResourceInstance->getConnect(uuid);
    if (conn.isNull()) {
        qDebug()<< LOG_FLAG << "getEnterPriseInfoPeap connection missing";
        return false;
    }
    NetworkManager::WirelessSecuritySetting::Ptr security_sett
        = conn->settings()->setting(NetworkManager::Setting::WirelessSecurity).dynamicCast<NetworkManager::WirelessSecuritySetting>();
    if (security_sett.isNull()) {
        qDebug()<< LOG_FLAG << "don't have WirelessSecurity connection";
        return false;
    }

    if (security_sett->keyMgmt() != NetworkManager::WirelessSecuritySetting::WpaEap) {
        qDebug()<< LOG_FLAG << "keyMgmt not WpaEap " << security_sett->keyMgmt();
        return false;
    }

    NetworkManager::Security8021xSetting::Ptr setting =
            conn->settings()->setting(NetworkManager::Setting::Security8021x).dynamicCast<NetworkManager::Security8021xSetting>();
    if (setting.isNull() || !setting->eapMethods().contains(NetworkManager::Security8021xSetting::EapMethod::EapMethodPeap)) {
        qDebug()<< LOG_FLAG << "don't have Security8021x connection";
        return false;
    }

    info.phase2AuthMethod = (KyNoEapMethodAuth)setting->phase2AuthMethod();
    info.userName = setting->identity();
    info.m_passwdFlag = setting->passwordFlags();
    if (!info.m_passwdFlag) {
        info.userPWD = m_operation->get8021xPassword(conn->uuid());
    }

    return true;
}

bool KyWirelessNetResource::getEnterPriseInfoTtls(QString &uuid, KyEapMethodTtlsInfo &info)
{
    NetworkManager::Connection::Ptr conn = m_networkResourceInstance->getConnect(uuid);
    if (conn.isNull()) {
        qDebug()<< LOG_FLAG << "modifyEnterPriseInfoTtls connection missing";
        return false;
    }

    NetworkManager::WirelessSecuritySetting::Ptr security_sett
        = conn->settings()->setting(NetworkManager::Setting::WirelessSecurity).dynamicCast<NetworkManager::WirelessSecuritySetting>();
    if (security_sett.isNull()) {
        qDebug()<< LOG_FLAG << "don't have WirelessSecurity connection";
        return false;
    }
    if (security_sett->keyMgmt() != NetworkManager::WirelessSecuritySetting::WpaEap) {
        qDebug()<< LOG_FLAG << "not wpaeap"<<security_sett->keyMgmt();
        return false;
    }

    NetworkManager::Security8021xSetting::Ptr setting = conn->settings()->setting(NetworkManager::Setting::Security8021x).dynamicCast<NetworkManager::Security8021xSetting>();
    if (setting.isNull() || !setting->eapMethods().contains(NetworkManager::Security8021xSetting::EapMethod::EapMethodTtls)) {
        qDebug()<< LOG_FLAG << "don't have Security8021x connection";
        return false;
    }

    info.authEapMethod = (KyEapMethodAuth)setting->phase2AuthEapMethod();
    info.authNoEapMethod = (KyNoEapMethodAuth)setting->phase2AuthMethod();

    info.authType = KyTtlsAuthMethod::AUTH_EAP;
    if (info.authEapMethod != KyAuthEapMethodUnknown) {
        info.authType = KyTtlsAuthMethod::AUTH_EAP;
    } else {
        info.authType = KyTtlsAuthMethod::AUTH_NO_EAP;
    }

    info.userName = setting->identity();
    info.m_passwdFlag = setting->passwordFlags();
    if (!info.m_passwdFlag) {
        info.userPWD = m_operation->get8021xPassword(conn->uuid());
    }

    return true;
}

bool KyWirelessNetResource::getEnterPriseInfoLeap(QString &uuid, KyEapMethodLeapInfo &info)
{
    NetworkManager::Connection::Ptr conn = m_networkResourceInstance->getConnect(uuid);
    if (conn.isNull()) {
        qDebug()<< LOG_FLAG << "getEnterPriseInfoLeap connection missing";
        return false;
    }
    NetworkManager::WirelessSecuritySetting::Ptr security_sett
        = conn->settings()->setting(NetworkManager::Setting::WirelessSecurity).dynamicCast<NetworkManager::WirelessSecuritySetting>();
    if (security_sett.isNull()) {
        qDebug()<< LOG_FLAG << "don't have WirelessSecurity connection";
        return false;
    }

    if (security_sett->keyMgmt() != NetworkManager::WirelessSecuritySetting::WpaEap) {
        qDebug()<< LOG_FLAG << "keyMgmt not WpaEap " << security_sett->keyMgmt();
        return false;
    }

    NetworkManager::Security8021xSetting::Ptr setting =
            conn->settings()->setting(NetworkManager::Setting::Security8021x).dynamicCast<NetworkManager::Security8021xSetting>();
    if (setting.isNull() || !setting->eapMethods().contains(NetworkManager::Security8021xSetting::EapMethod::EapMethodLeap)) {
        qDebug()<< LOG_FLAG << "don't have Security8021x connection";
        return false;
    }

    info.m_userName = setting->identity();
    info.m_passwdFlag = setting->passwordFlags();
    if (!info.m_passwdFlag) {
        info.m_userPwd = m_operation->get8021xPassword(conn->uuid());
    }

    return true;
}

bool KyWirelessNetResource::getEnterPriseInfoPwd(QString &uuid, KyEapMethodPwdInfo &info)
{
    NetworkManager::Connection::Ptr conn = m_networkResourceInstance->getConnect(uuid);
    if (conn.isNull()) {
        qDebug()<< LOG_FLAG << "getEnterPriseInfoPwd connection missing";
        return false;
    }
    NetworkManager::WirelessSecuritySetting::Ptr security_sett
        = conn->settings()->setting(NetworkManager::Setting::WirelessSecurity).dynamicCast<NetworkManager::WirelessSecuritySetting>();
    if (security_sett.isNull()) {
        qDebug()<< LOG_FLAG << "don't have WirelessSecurity connection";
        return false;
    }

    if (security_sett->keyMgmt() != NetworkManager::WirelessSecuritySetting::WpaEap) {
        qDebug()<< LOG_FLAG << "keyMgmt not WpaEap " << security_sett->keyMgmt();
        return false;
    }

    NetworkManager::Security8021xSetting::Ptr setting =
            conn->settings()->setting(NetworkManager::Setting::Security8021x).dynamicCast<NetworkManager::Security8021xSetting>();
    if (setting.isNull() || !setting->eapMethods().contains(NetworkManager::Security8021xSetting::EapMethod::EapMethodPwd)) {
        qDebug()<< LOG_FLAG << "don't have Security8021x connection";
        return false;
    }

    info.m_userName = setting->identity();
    info.m_passwdFlag = setting->passwordFlags();
    if (!info.m_passwdFlag) {
        info.m_userPwd = m_operation->get8021xPassword(conn->uuid());
    }

    return true;
}

bool KyWirelessNetResource::getEnterPriseInfoFast(QString &uuid, KyEapMethodFastInfo &info)
{
    NetworkManager::Connection::Ptr conn = m_networkResourceInstance->getConnect(uuid);
    if (conn.isNull()) {
        qDebug()<< LOG_FLAG << "getEnterPriseInfoFast connection missing";
        return false;
    }
    NetworkManager::WirelessSecuritySetting::Ptr security_sett
        = conn->settings()->setting(NetworkManager::Setting::WirelessSecurity).dynamicCast<NetworkManager::WirelessSecuritySetting>();
    if (security_sett.isNull()) {
        qDebug()<< LOG_FLAG << "don't have WirelessSecurity connection";
        return false;
    }

    if (security_sett->keyMgmt() != NetworkManager::WirelessSecuritySetting::WpaEap) {
        qDebug()<< LOG_FLAG << "keyMgmt not WpaEap " << security_sett->keyMgmt();
        return false;
    }

    NetworkManager::Security8021xSetting::Ptr setting =
            conn->settings()->setting(NetworkManager::Setting::Security8021x).dynamicCast<NetworkManager::Security8021xSetting>();
    if (setting.isNull() || !setting->eapMethods().contains(NetworkManager::Security8021xSetting::EapMethod::EapMethodFast)) {
        qDebug()<< LOG_FLAG << "don't have Security8021x connection";
        return false;
    }

    info.m_anonIdentity = setting->anonymousIdentity();
    info.m_pacProvisioning = (KyFastProvisioning)setting->phase1FastProvisioning();
    info.m_pacFilePath = setting->pacFile();
    if (info.m_pacFilePath.left(7) == "file://") {
        info.m_pacFilePath = info.m_pacFilePath.mid(7);
    }
    info.m_authMethod = (KyNoEapMethodAuth)setting->phase2AuthMethod();

    info.m_userName = setting->identity();
    info.m_passwdFlag = setting->passwordFlags();
    if (!info.m_passwdFlag) {
        info.m_userPwd = m_operation->get8021xPassword(conn->uuid());
    }

    return true;
}

void KyWirelessNetResource::onConnectionAdd(QString uuid)
{
    qDebug() << LOG_FLAG << "onConnectionAdd " << uuid;
    NetworkManager::Connection::Ptr conn = m_networkResourceInstance->getConnect(uuid);
    if (conn.isNull()) {
        qDebug()<< LOG_FLAG << "onConnectionAdd can not find connection" << uuid;
        return;
    }

    NetworkManager::ConnectionSettings::Ptr sett = conn->settings();
    if (sett->connectionType() != NetworkManager::ConnectionSettings::ConnectionType::Wireless) {
        qDebug()<< LOG_FLAG << uuid << " is not wireless connection";
        return;
    }
    NetworkManager::WirelessSetting::Ptr wireless_sett =
            sett->setting(NetworkManager::Setting::Wireless).dynamicCast<NetworkManager::WirelessSetting>();

    QByteArray rawSsid = wireless_sett->ssid();
    QString wifiSsid = getSsidFromByteArray(rawSsid);
    QMap<QString, QString> map;
    map.clear();
    QMap<QString, QList<KyWirelessNetItem> >::iterator iter = m_WifiNetworkList.begin();
    while (iter != m_WifiNetworkList.end()) {
        for(int i = 0; i < iter.value().size(); i++) {
            if (iter.value().at(i).m_NetSsid == wifiSsid
                    && (sett->interfaceName() == iter.key() || sett->interfaceName().isEmpty())) {
                QString devIfaceName;
                QString ssid;
                m_WifiNetworkList[iter.key()][i].m_isConfigured = true;
                m_WifiNetworkList[iter.key()][i].m_connName = conn->name();
                m_WifiNetworkList[iter.key()][i].m_connectUuid = conn->uuid();
                m_WifiNetworkList[iter.key()][i].m_connDbusPath = conn->path();
                m_WifiNetworkList[iter.key()][i].m_channel = wireless_sett->channel();

                devIfaceName = iter.key();
                ssid = iter.value().at(i).m_NetSsid;
                map.insert(devIfaceName, ssid);

                break;
            }
        }
        iter++;
    }

    if (!map.isEmpty()) {
        for(auto var = map.cbegin(); var != map.cend(); var++) {
            QString devIfaceName = var.key();
            QString ssid = var.value();
            Q_EMIT connectionAdd(devIfaceName, ssid);
        }
    }
}

void KyWirelessNetResource::onConnectionRemove(QString path)
{
    qDebug()<< LOG_FLAG << "onConnectionRemove remove " << path;
    QMap<QString, QString> map;
    map.clear();

    QMap<QString, QList<KyWirelessNetItem> >::iterator iter = m_WifiNetworkList.begin();
    while (iter != m_WifiNetworkList.end()) {
        qDebug()<< LOG_FLAG <<"wifi network list key:" << iter.key();
        for(int i = 0; i < iter.value().size(); i++) {
            qDebug() << LOG_FLAG << "connection path" << iter.value().at(i).m_connDbusPath;
            if (iter.value().at(i).m_connDbusPath == path) {
                QString devIfaceName;
                QString ssid;
                m_WifiNetworkList[iter.key()][i].m_isConfigured = false;
                m_WifiNetworkList[iter.key()][i].m_connName = "";
                m_WifiNetworkList[iter.key()][i].m_connectUuid = "";
                m_WifiNetworkList[iter.key()][i].m_connDbusPath = "";
                m_WifiNetworkList[iter.key()][i].m_channel = 0;

                devIfaceName = iter.key();
                ssid = iter.value().at(i).m_NetSsid;
                map.insert(devIfaceName, ssid);
                break;
            }
        }
        iter++;
    } 

    if (!map.isEmpty()) {
        for(auto var = map.cbegin(); var != map.cend(); var++) {
            QString devIfaceName = var.key();
            QString ssid = var.value();
            Q_EMIT connectionRemove(devIfaceName, ssid, path);
        }
    }

}


void KyWirelessNetResource::onConnectionUpdate(QString uuid)
{
    qDebug()<< LOG_FLAG << "onConnectionUpdate " << uuid;
    NetworkManager::Connection::Ptr conn = m_networkResourceInstance->getConnect(uuid);
    if (conn.isNull()) {
        qDebug()<< LOG_FLAG  << "onConnectionAdd can not find connection" << uuid;
        return;
    }

    NetworkManager::ConnectionSettings::Ptr sett= conn->settings();
    if (sett->connectionType() != NetworkManager::ConnectionSettings::ConnectionType::Wireless) {
        return;
    }

    QString ssid, dev;
    getSsidByUuid(uuid, ssid);
    getDeviceByUuid(uuid, dev);

    if (!dev.isEmpty()) {
        NetworkManager::Device::Ptr devicePtr = m_networkResourceInstance->findDeviceInterface(dev);
        if (devicePtr.isNull() || !m_WifiNetworkList.contains(dev)) {
            return;
        }
    }

    QMap<QString, QList<KyWirelessNetItem> >::iterator iter;
    for (iter = m_WifiNetworkList.begin(); iter != m_WifiNetworkList.end(); ++iter) {
        QList<KyWirelessNetItem>::iterator itemIter;
        for (itemIter = iter.value().begin(); itemIter != iter.value().end(); ++itemIter) {
            //判断是否有其他wifi配置 更新WIFI-bd 的connect相关变量 emit update
            if (uuid == itemIter->m_connectUuid) {
                if (itemIter->m_NetSsid != ssid ||
                        (iter.key() != dev && !dev.isEmpty())) {
                    updatewirelessItemConnectInfo(*itemIter);
                    Q_EMIT connectionUpdate(iter.key(), itemIter->m_NetSsid);

                    //判断netptr是否为空 空返回
                    //否则 更新ssid 的connect相关变量 emit update ssid
                    NetworkManager::Device::Ptr devicePtr = m_networkResourceInstance->findDeviceInterface(dev);
                    NetworkManager::WirelessNetwork::Ptr netPtr = m_networkResourceInstance->findWifiNetwork(ssid, devicePtr->uni());
                    if (netPtr.isNull()) {
                        qDebug() << LOG_FLAG << ssid << "netPtr is Null";
                        return;
                    }
                }
            }
            //更新WIFI 的connect相关变量 emit update to ui
            if (ssid == itemIter->m_NetSsid) {
                if (iter.key() == dev || dev.isEmpty()) {
                    updatewirelessItemConnectInfo(*itemIter);
                    Q_EMIT connectionUpdate(dev, itemIter->m_NetSsid);
                }
            }
            break;
        }
    }
    Q_EMIT wifiNetworkUpdate();
}


void KyWirelessNetResource::onDeviceAdd(QString deviceName, NetworkManager::Device::Type deviceType)
{
    if(deviceType == NetworkManager::Device::Type::Wifi) {
        if (!m_WifiNetworkList.contains(deviceName)) {
            m_WifiNetworkList.insert(deviceName,QList<KyWirelessNetItem>());
        }
    }
}

void KyWirelessNetResource::onDeviceRemove(QString deviceName)
{
    if (m_WifiNetworkList.contains(deviceName)) {
        m_WifiNetworkList.remove(deviceName);
    }
}

void KyWirelessNetResource::onDeviceNameUpdate(QString oldName, QString newName)
{
    if (!m_WifiNetworkList.contains(oldName)) {
        return;
    }

    QMap<QString, QList<KyWirelessNetItem>> newWifiNetworkList(m_WifiNetworkList);
    QList<KyWirelessNetItem> list = m_WifiNetworkList[oldName];
    newWifiNetworkList.remove(oldName);
    newWifiNetworkList.insert(newName,list);
    m_WifiNetworkList = newWifiNetworkList;

}
