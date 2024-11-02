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
#include "kywirelessnetitem.h"
#include "lingmoactiveconnectresource.h"
#include <NetworkManagerQt/Connection>
#include "lingmoutil.h"

const QString ENTERPRICE_TYPE = "802.1X";
//const QString WPA1_AND_WPA2 = "WPA";
const QString WPA1 = "WPA1";
const QString WPA2 = "WPA2";
const QString WPA3 = "WPA3";

#define FREQ_5GHZ 5000

KyWirelessNetItem::KyWirelessNetItem(NetworkManager::WirelessNetwork::Ptr net)
{
    m_networkResourceInstance = KyNetworkResourceManager::getInstance();

    m_bssid = "";
    m_connectUuid = "";
    m_isConfigured = false;
    m_connName = "";
    m_connDbusPath = "";
    m_secuType = "";
    m_kySecuType = NONE;
    m_device = "";
    m_channel = 0;
    m_isMix = false;

    init(net);
}


KyWirelessNetItem::~KyWirelessNetItem()
{
    m_networkResourceInstance = nullptr;
}


void KyWirelessNetItem::init(NetworkManager::WirelessNetwork::Ptr net)
{
   // m_NetSsid = net->ssid();

    NetworkManager::AccessPoint::Ptr accessPointPtr = net->referenceAccessPoint();
    if (accessPointPtr->ssid().isEmpty()) {
        qDebug() << accessPointPtr->uni() << "ssid isEmpty";
        NetworkManager::AccessPoint::List list = net->accessPoints();
        if (list.size() > 1) {
            for (int i = 0; i < list.size(); ++i) {
                if (!list.at(i)->ssid().isEmpty()) {
                    qDebug() << "use" << accessPointPtr->uni();
                    accessPointPtr = list.at(i);
                    break;
                }
            }
        }
    }
    if (accessPointPtr->ssid().isEmpty()) {
        return;
    }
    QByteArray rawSsid = accessPointPtr->rawSsid();
    m_NetSsid = getSsidFromByteArray(rawSsid);

    m_signalStrength = accessPointPtr->signalStrength();
    m_frequency = accessPointPtr->frequency();
    m_channel = NetworkManager::findChannel(m_frequency);
    NetworkManager::AccessPoint::Capabilities cap = accessPointPtr->capabilities();
    NetworkManager::AccessPoint::WpaFlags wpaFlag = accessPointPtr->wpaFlags();
    NetworkManager::AccessPoint::WpaFlags rsnFlag = accessPointPtr->rsnFlags();
    m_secuType = enumToQstring(cap, wpaFlag, rsnFlag);
//    if (m_secuType.indexOf(ENTERPRICE_TYPE) >= 0) {
//            m_kySecuType = WPA_AND_WPA2_ENTERPRISE;
//        } else if (m_secuType.indexOf(WPA3) >= 0) {
//            m_kySecuType = WPA3_PERSONAL;
//        } else if ( m_secuType.indexOf(WPA1_AND_WPA2) >= 0) {
//            m_kySecuType = WPA_AND_WPA2_PERSONAL;
//    }
    setKySecuType(m_secuType);
    m_bssid = accessPointPtr->hardwareAddress();
    m_device = net->device();
    m_uni = accessPointPtr->uni();

    NetworkManager::Device::Ptr devicePtr = nullptr;
    devicePtr = m_networkResourceInstance->findDeviceInterface(m_device);
    if (!devicePtr.isNull()) {
        QString devUni = devicePtr->uni();
        NetworkManager::WirelessNetwork::Ptr wirelessPtr = nullptr;
        wirelessPtr = m_networkResourceInstance->findWifiNetwork(m_NetSsid, devUni);
        if (!wirelessPtr.isNull()) {
            NetworkManager::AccessPoint::List apList = wirelessPtr->accessPoints();
            bool b2G = false;
            bool b5G = false;
            if (!apList.empty()) {
                for (int i = 0; i < apList.count(); ++i) {
                    if (apList.at(i)->frequency() < FREQ_5GHZ) {
                        b2G = true;
                    }
                    if (apList.at(i)->frequency() >= FREQ_5GHZ) {
                        b5G = true;
                    }
                    if (b2G && b5G) {
                        m_isMix = true;
                        break;
                    }
                }
            }
            devicePtr = m_networkResourceInstance->findDeviceInterface(m_device);
            if (!devicePtr.isNull()) {
                QString devUni = devicePtr->uni();
                NetworkManager::WirelessNetwork::Ptr wirelessPtr = nullptr;
                wirelessPtr = m_networkResourceInstance->findWifiNetwork(m_NetSsid, devUni);
                if (!wirelessPtr.isNull()) {
                    NetworkManager::AccessPoint::List apList = wirelessPtr->accessPoints();
                    bool b2G = false;
                    bool b5G = false;
                    if (!apList.empty()) {
                        for (int i = 0; i < apList.count(); ++i) {
                            if (apList.at(i)->frequency() < FREQ_5GHZ) {
                                b2G = true;
                            }
                            if (apList.at(i)->frequency() >= FREQ_5GHZ) {
                                b5G = true;
                            }
                            if (b2G && b5G) {
                                m_isMix = true;
                                break;
                            }

                        }
                    }
                }
            }
        }
    }
    updatewirelessItemConnectInfo(*this);
}

int KyWirelessNetItem::getCategory(QString uni)
{

    QDBusInterface interface( "org.freedesktop.NetworkManager", uni, "org.freedesktop.DBus.Properties", QDBusConnection::systemBus() );
    if (!interface.isValid()) {
        qDebug() << Q_FUNC_INFO << "dbus is invalid";
        return -1;
    }

    QDBusReply<QVariant> reply = interface.call("Get", "org.freedesktop.NetworkManager.AccessPoint", "Category");
    if (!reply.isValid()) {
        //qDebug()<<"can not get the attribute 'Category' in func getCategory()";
        return 0;
    } else {
        return reply.value().toInt();
    }

}

void KyWirelessNetItem::setKySecuType(QString strSecuType)
{
    if (strSecuType.indexOf(ENTERPRICE_TYPE) >= 0) {
        m_kySecuType = WPA_AND_WPA2_ENTERPRISE;
    } else if (strSecuType.indexOf(WPA3) >= 0) {
        if (strSecuType.indexOf(WPA1) >= 0 || strSecuType.indexOf(WPA2) >= 0) {
            m_kySecuType = WPA_AND_WPA3;
        } else {
            m_kySecuType = WPA3_PERSONAL;
        }
    } else if ( strSecuType.indexOf(WPA1) >= 0 || strSecuType.indexOf(WPA2) >= 0) {
        m_kySecuType = WPA_AND_WPA2_PERSONAL;
    } else {
        m_kySecuType = NONE;
    }
}

void updatewirelessItemConnectInfo(KyWirelessNetItem& item)
{
    KyNetworkResourceManager *networkResourceInstance = KyNetworkResourceManager::getInstance();

    bool findHotspot = false;
    bool findInfrastructure = false;

    KyWirelessNetItem hotspotItem;
    KyWirelessNetItem connectItem;

    for (auto const & conn : networkResourceInstance->m_connections) {
        NetworkManager::ConnectionSettings::Ptr settings = conn->settings();
        if (settings->connectionType() != NetworkManager::ConnectionSettings::Wireless) {
            continue;
        }

        NetworkManager::WirelessSetting::Ptr wifi_sett
            = settings->setting(NetworkManager::Setting::Wireless).dynamicCast<NetworkManager::WirelessSetting>();
        QString devName = networkResourceInstance->findDeviceUni(item.getDevice())->interfaceName();
        QByteArray rawSsid = wifi_sett->ssid();
        QString wifiSsid = getSsidFromByteArray(rawSsid);
        if (wifiSsid == item.m_NetSsid
                && (settings->interfaceName().compare(devName) == 0 || settings->interfaceName().isEmpty())) {
            /*
            * 如果有激活的链接，则取激活的链接，没有则取最后一个，因为一个热点可以创建多个链接, 有WIFI的则用WIFI，否则用adhoc
            */
            KyActiveConnectResourse actResource;
            KyConnectItem * kyItem = actResource.getActiveConnectionByUuid(settings->uuid(), devName);
            if (nullptr != kyItem) {
                item.m_connectUuid = settings->uuid();
                item.m_connName    = conn->name();
                item.m_connDbusPath = conn->path();
                item.m_isConfigured = true;
                return;
            }

            if (wifi_sett->mode() != NetworkManager::WirelessSetting::NetworkMode::Infrastructure) {
                hotspotItem.m_connectUuid = settings->uuid();
                hotspotItem.m_connName    = conn->name();
                hotspotItem.m_connDbusPath = conn->path();
                hotspotItem.m_isConfigured = true;
                findHotspot = true;
            } else {
                connectItem.m_connectUuid = settings->uuid();
                connectItem.m_connName    = conn->name();
                connectItem.m_connDbusPath = conn->path();
                connectItem.m_isConfigured = true;
                findInfrastructure = true;
            }
        }
    }

    if (findInfrastructure) {
        item.m_connectUuid = connectItem.m_connectUuid;
        item.m_connName = connectItem.m_connName;
        item.m_connDbusPath = connectItem.m_connDbusPath;
        item.m_isConfigured = connectItem.m_isConfigured;
    } else if (findHotspot) {
        item.m_connectUuid = hotspotItem.m_connectUuid;
        item.m_connName = hotspotItem.m_connName;
        item.m_connDbusPath = hotspotItem.m_connDbusPath;
        item.m_isConfigured = hotspotItem.m_isConfigured;
    } else {
        item.m_connectUuid.clear();
        item.m_connName.clear();
        item.m_connDbusPath.clear();
        item.m_isConfigured = false;
    }
}
