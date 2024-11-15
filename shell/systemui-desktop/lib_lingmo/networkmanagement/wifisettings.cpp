/*
 *   Copyright 2018 Martin Kacej <m.kacej@atlas.sk>
 *   Copyright 2021 Wang Rui <wangrui@jingos.com>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2 or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU Library General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "wifisettings.h"

#include <NetworkManagerQt/ActiveConnection>
#include <NetworkManagerQt/AccessPoint>
#include <NetworkManagerQt/Connection>
#include <NetworkManagerQt/ConnectionSettings>
#include <NetworkManagerQt/GsmSetting>
#include <NetworkManagerQt/Ipv4Setting>
#include <NetworkManagerQt/Manager>
#include <NetworkManagerQt/Settings>
#include <NetworkManagerQt/Utils>
#include <NetworkManagerQt/WiredDevice>
#include <NetworkManagerQt/WiredSetting>
#include <NetworkManagerQt/WirelessDevice>
#include <NetworkManagerQt/WirelessSetting>
#include <NetworkManagerQt/Security8021xSetting>

#include <sys/types.h>
#include <pwd.h>

WifiSettings::WifiSettings(QObject* parent)
    : QObject(parent)
{
    ::passwd *pw = ::getpwuid(::getuid());
    m_userName = QString::fromLocal8Bit(pw->pw_name);
}

WifiSettings::~WifiSettings()
{
}

QVariantMap WifiSettings::getConnectionSettings(const QString &connection, const QString &type)
{
    if (type.isEmpty())
        return QVariantMap();

    NetworkManager::Connection::Ptr con = NetworkManager::findConnection(connection);
    if (!con)
        return QVariantMap();

    if (type == "secrets")
        return con->secrets(QLatin1String("802-11-wireless-security")).value().value(QLatin1String("802-11-wireless-security"));

    QVariantMap map = con->settings()->toMap().value(type);
    if (type == "ipv4") {
        NetworkManager::Ipv4Setting::Ptr ipSettings = NetworkManager::Ipv4Setting::Ptr(new NetworkManager::Ipv4Setting());
        ipSettings->fromMap(map);
        map.clear();
        if (ipSettings->method() == NetworkManager::Ipv4Setting::Automatic) {
            map.insert(QLatin1String("method"),QVariant(QLatin1String("auto")));
        }

        if (ipSettings->method() == NetworkManager::Ipv4Setting::Manual) {
            map.insert(QLatin1String("method"),QVariant(QLatin1String("manual")));
            map.insert(QLatin1String("address"),QVariant(ipSettings->addresses().first().ip().toString()));
            map.insert(QLatin1String("prefix"),QVariant(ipSettings->addresses().first().prefixLength()));
            map.insert(QLatin1String("gateway"),QVariant(ipSettings->addresses().first().gateway().toString()));
            map.insert(QLatin1String("dns"),QVariant(ipSettings->dns().first().toString()));
        }
    }
    return map;
}

QVariantMap WifiSettings::getActiveConnectionInfo(const QString &connection)
{
    if (connection.isEmpty())
        return QVariantMap();

    NetworkManager::ActiveConnection::Ptr activeCon;
    NetworkManager::Connection::Ptr con = NetworkManager::findConnection(connection);
    foreach (const NetworkManager::ActiveConnection::Ptr &active, NetworkManager::activeConnections()) {
        if (active->uuid() == con->uuid())
            activeCon = active;
    }

    if (!activeCon) {
        qWarning() << "Active" << connection << "not found";
        return QVariantMap();
    }

    QVariantMap map;
    if (activeCon->ipV4Config().addresses().count() > 0) {
        map.insert("address",QVariant(activeCon->ipV4Config().addresses().first().ip().toString()));
        map.insert("prefix",QVariant(activeCon->ipV4Config().addresses().first().netmask().toString()));
    }
    map.insert("gateway",QVariant(activeCon->ipV4Config().gateway()));
    if (activeCon->ipV4Config().nameservers().count() > 0)
        map.insert("dns",QVariant(activeCon->ipV4Config().nameservers().first().toString()));
    //qWarning() << map;
    return map;
}

void WifiSettings::addConnectionFromQML(const QVariantMap &QMLmap)
{
    if (QMLmap.isEmpty())
        return;

    NetworkManager::ConnectionSettings::Ptr connectionSettings = NetworkManager::ConnectionSettings::Ptr(new NetworkManager::ConnectionSettings(NetworkManager::ConnectionSettings::Wireless));
    connectionSettings->setId(QMLmap.value(QLatin1String("id")).toString());
    connectionSettings->setUuid(NetworkManager::ConnectionSettings::createNewUuid());

    NetworkManager::WirelessSetting::Ptr wirelessSettings = NetworkManager::WirelessSetting::Ptr(new NetworkManager::WirelessSetting());
    wirelessSettings->setSsid(QMLmap.value(QLatin1String("id")).toString().toUtf8());
    if (QMLmap["mode"].toString() == "infrastructure") {
        wirelessSettings->setMode(NetworkManager::WirelessSetting::Infrastructure);
        connectionSettings->setAutoconnect(true);
    }
    if (QMLmap["mode"].toString() == "ap") {
        wirelessSettings->setMode(NetworkManager::WirelessSetting::Ap);
        connectionSettings->setAutoconnect(false);
    }
    if (QMLmap.contains("hidden")) {
        wirelessSettings->setHidden(QMLmap.value("hidden").toBool());
    }

    NetworkManager::Ipv4Setting::Ptr ipSettings = NetworkManager::Ipv4Setting::Ptr(new NetworkManager::Ipv4Setting());
    if (QMLmap["method"] == QLatin1String("auto")) {
        ipSettings->setMethod(NetworkManager::Ipv4Setting::ConfigMethod::Automatic);
    }
    if (QMLmap["method"] == QLatin1String("shared")) {
        ipSettings->setMethod(NetworkManager::Ipv4Setting::ConfigMethod::Shared);
    }
    if (QMLmap["method"] == QLatin1String("manual")) {
        ipSettings->setMethod(NetworkManager::Ipv4Setting::ConfigMethod::Manual);
        NetworkManager::IpAddress ipaddr;
        ipaddr.setIp(QHostAddress(QMLmap["address"].toString()));
        ipaddr.setPrefixLength(QMLmap["prefix"].toInt());
        ipaddr.setGateway(QHostAddress(QMLmap["gateway"].toString()));
        ipSettings->setAddresses(QList<NetworkManager::IpAddress>({ipaddr}));
        ipSettings->setDns(QList<QHostAddress>({QHostAddress(QMLmap["dns"].toString())}));
    }

    NMVariantMapMap map = connectionSettings->toMap();
    map.insert("802-11-wireless",wirelessSettings->toMap());
    map.insert("ipv4",ipSettings->toMap());

    // TODO can't set password for AP
    // needs further inspection

    if (QMLmap.contains("802-11-wireless-security")) {
        QVariantMap securMap = QMLmap["802-11-wireless-security"].toMap();
        int type = securMap["type"].toInt();
        if (!type == NetworkManager::NoneSecurity) {
            NetworkManager::WirelessSecuritySetting::Ptr securitySettings = NetworkManager::WirelessSecuritySetting::Ptr(new NetworkManager::WirelessSecuritySetting());
            if (type == NetworkManager::Wpa2Psk ) {
                if (QMLmap["mode"].toString() == "ap") {
                    securitySettings->setKeyMgmt(NetworkManager::WirelessSecuritySetting::KeyMgmt::WpaNone);
                } else {
                    securitySettings->setKeyMgmt(NetworkManager::WirelessSecuritySetting::KeyMgmt::WpaPsk);
                }
                securitySettings->setAuthAlg(NetworkManager::WirelessSecuritySetting::AuthAlg::Open);
                securitySettings->setPskFlags(NetworkManager::Setting::SecretFlagType::AgentOwned);
                securitySettings->setPsk(securMap["password"].toString());
            }
            if (type == NetworkManager::StaticWep) {
                securitySettings->setKeyMgmt(NetworkManager::WirelessSecuritySetting::KeyMgmt::Wep);
                securitySettings->setAuthAlg(NetworkManager::WirelessSecuritySetting::AuthAlg::Open);
                securitySettings->setWepKeyType(NetworkManager::WirelessSecuritySetting::WepKeyType::Hex);
                securitySettings->setWepKeyFlags(NetworkManager::Setting::SecretFlagType::AgentOwned);
                securitySettings->setWepKey0(securMap["password"].toString());
            }
            map.insert("802-11-wireless-security",securitySettings->toMap());
        }
    }
    //qWarning() << map;
    NetworkManager::addConnection(map);
}

void WifiSettings::updateConnectionFromQML(const QString &path, const QVariantMap &map)
{
    NetworkManager::Connection::Ptr con = NetworkManager::findConnection(path);
    if (!con)
        return;

    //qWarning() << map;
    if (map.contains("id"))
        con->settings()->setId(map.value("id").toString());

    NMVariantMapMap toUpdateMap = con->settings()->toMap();

    NetworkManager::Ipv4Setting::Ptr ipSetting = con->settings()->setting(NetworkManager::Setting::Ipv4).staticCast<NetworkManager::Ipv4Setting>();
    if (ipSetting->method() == NetworkManager::Ipv4Setting::Automatic || ipSetting->method() == NetworkManager::Ipv4Setting::Manual) {
        if (map.value("method") == "auto") {
            ipSetting->setMethod(NetworkManager::Ipv4Setting::Automatic);
        }

        if (map.value("method") == "manual") {
            ipSetting->setMethod(NetworkManager::Ipv4Setting::ConfigMethod::Manual);
            NetworkManager::IpAddress ipaddr;
            ipaddr.setIp(QHostAddress(map["address"].toString()));
            ipaddr.setPrefixLength(map["prefix"].toInt());
            ipaddr.setGateway(QHostAddress(map["gateway"].toString()));
            ipSetting->setAddresses(QList<NetworkManager::IpAddress>({ipaddr}));
            ipSetting->setDns(QList<QHostAddress>({QHostAddress(map["dns"].toString())}));
        }
        toUpdateMap.insert("ipv4",ipSetting->toMap());
    }

    NetworkManager::WirelessSetting::Ptr wirelessSetting = con->settings()->setting(NetworkManager::Setting::Wireless).staticCast<NetworkManager::WirelessSetting>();
    if (map.contains("hidden")) {
        wirelessSetting->setHidden(map.value("hidden").toBool());
    }
    if (map.contains("id")) {
        wirelessSetting->setSsid(map.value("id").toByteArray());
    }
    toUpdateMap.insert("802-11-wireless",wirelessSetting->toMap());

    if (map.contains("802-11-wireless-security")) {
        QVariantMap secMap = map.value("802-11-wireless-security").toMap();
        //qWarning() << secMap;
        NetworkManager::WirelessSecuritySetting::Ptr securitySetting = con->settings()->setting(NetworkManager::Setting::WirelessSecurity).staticCast<NetworkManager::WirelessSecuritySetting>();
        if ((securitySetting->keyMgmt() == NetworkManager::WirelessSecuritySetting::Wep)
                && (secMap.value("type") == NetworkManager::StaticWep))
        {
            securitySetting->setWepKey0(secMap["password"].toString());
        }

        if ((securitySetting->keyMgmt() == NetworkManager::WirelessSecuritySetting::WpaPsk)
                && (secMap.value("type") == NetworkManager::Wpa2Psk))
        {
            securitySetting->setPsk(secMap["password"].toString());
        }

        // TODO can't set password for AP
        // needs further inspection
        if (wirelessSetting->mode() == NetworkManager::WirelessSetting::Ap) {
            if (securitySetting->toMap().empty()) { //no security
                if (secMap.value("type") == NetworkManager::Wpa2Psk) {
                    securitySetting->setKeyMgmt(NetworkManager::WirelessSecuritySetting::WpaNone);
                    securitySetting->setPsk(secMap.value("password").toString());
                }
            }
            if (securitySetting->keyMgmt() == NetworkManager::WirelessSecuritySetting::WpaNone) {
                if (secMap.empty()) {
                    securitySetting->setKeyMgmt(NetworkManager::WirelessSecuritySetting::Unknown);
                }
                if (secMap.value("type") == NetworkManager::Wpa2Psk) {
                    securitySetting->setPsk(secMap.value("password").toString());
                }
            }
        }

        toUpdateMap.insert("802-11-wireless-security",securitySetting->toMap());
    }
    qWarning() << toUpdateMap;
    con->update(toUpdateMap);
}

QString WifiSettings::getAccessPointDevice()
{
    NetworkManager::WirelessDevice::Ptr device;
    foreach (const NetworkManager::Device::Ptr &dev, NetworkManager::networkInterfaces()) {
        if (dev->type() == NetworkManager::Device::Wifi){
            device = dev.staticCast<NetworkManager::WirelessDevice>();
            if (device->wirelessCapabilities().testFlag(NetworkManager::WirelessDevice::ApCap))
                break; // we have wireless device with access point capability
        }
    }
    if (device) {
        return device->uni();
    } else {
        qWarning() << "No wireless device found";
    }
    return QString();
}

QString WifiSettings::getAccessPointConnection()
{
    foreach (const NetworkManager::Connection::Ptr &con,  NetworkManager::listConnections()) {
        NetworkManager::Setting::Ptr d = con->settings()->setting(NetworkManager::Setting::Wireless);
        if (!d.isNull()){
            if( d.staticCast<NetworkManager::WirelessSetting>()->mode() == NetworkManager::WirelessSetting::Ap){
                return con->path();
            }
        }
    }
    return QString();
}

bool WifiSettings::addOtherConnection(const QString ssid, const QString userName, const QString pwd, const QString type)
{
    NetworkManager::ConnectionSettings::Ptr connectionSettings;
    connectionSettings = NetworkManager::ConnectionSettings::Ptr(new NetworkManager::ConnectionSettings(NetworkManager::ConnectionSettings::Wireless));
    connectionSettings->setAutoconnect(true);
    connectionSettings->setUuid(NetworkManager::ConnectionSettings::createNewUuid());
    connectionSettings->setId(ssid);
    connectionSettings->addToPermissions(m_userName, QString());

    NMVariantMapMap csMapMap = connectionSettings->toMap();;
    NetworkManager::WirelessSetting wirelessSetting;

    wirelessSetting.setSsid(ssid.toUtf8());
    wirelessSetting.setInitialized(true);
    wirelessSetting.setMode(NetworkManager::WirelessSetting::Infrastructure);
    wirelessSetting.setHidden(true);

    if (type != "None") {
        wirelessSetting.setSecurity("802-11-wireless-security");
    }

    NetworkManager::WirelessSecuritySetting wirelessSecuritySetting;
    if(type == "None") {
        wirelessSecuritySetting.setKeyMgmt(NetworkManager::WirelessSecuritySetting::Unknown);
    } else if (type == "WEP") {
        wirelessSecuritySetting.setKeyMgmt(NetworkManager::WirelessSecuritySetting::Ieee8021x);
        NetworkManager::Security8021xSetting security8021x;
        security8021x.setEapMethods(QList<NetworkManager::Security8021xSetting::EapMethod>() << NetworkManager::Security8021xSetting::EapMethodPwd);
        security8021x.setIdentity(userName);
        security8021x.setPassword(pwd);
        security8021x.setPasswordFlags(NetworkManager::Setting::AgentOwned);
        QVariantMap security8021xMap = security8021x.toMap();
        csMapMap.insert(NetworkManager::Setting::typeAsString(NetworkManager::Setting::Security8021x), security8021xMap);
    } else if (type == "WPA/WPA2") {
        wirelessSecuritySetting.setKeyMgmt(NetworkManager::WirelessSecuritySetting::WpaPsk);
        wirelessSecuritySetting.setPsk(pwd);
        wirelessSecuritySetting.setPskFlags(NetworkManager::Setting::AgentOwned);
    } else if (type == "WPA3") {
        wirelessSecuritySetting.setKeyMgmt(NetworkManager::WirelessSecuritySetting::SAE);
        wirelessSecuritySetting.setPsk(pwd);
        wirelessSecuritySetting.setPskFlags(NetworkManager::Setting::AgentOwned);
    } else if (type == "LEAP") {
        wirelessSecuritySetting.setKeyMgmt(NetworkManager::WirelessSecuritySetting::Ieee8021x);
        wirelessSecuritySetting.setAuthAlg(NetworkManager::WirelessSecuritySetting::Leap);
        wirelessSecuritySetting.setLeapUsername(userName);
        wirelessSecuritySetting.setLeapPassword(pwd);
        wirelessSecuritySetting.setPskFlags(NetworkManager::Setting::AgentOwned);
    } else if(type == "WepHex") {
        wirelessSecuritySetting.setKeyMgmt(NetworkManager::WirelessSecuritySetting::Wep);
        wirelessSecuritySetting.setWepKeyType(NetworkManager::WirelessSecuritySetting::Hex);
        wirelessSecuritySetting.setWepTxKeyindex(1);
        wirelessSecuritySetting.setWepKey0(pwd);
        wirelessSecuritySetting.setAuthAlg(NetworkManager::WirelessSecuritySetting::Open);
    }

    NetworkManager::Ipv4Setting ipv4Setting;
    ipv4Setting.setMethod(NetworkManager::Ipv4Setting::Automatic);

    QVariantMap wifiSettingMap = wirelessSetting.toMap();
    QVariantMap wifiSecurityMap = wirelessSecuritySetting.toMap();
    QVariantMap ipv4Map = ipv4Setting.toMap();

    csMapMap.insert(NetworkManager::Setting::typeAsString(NetworkManager::Setting::Wireless), wifiSettingMap);
    if(type != "None") {
        csMapMap.insert(NetworkManager::Setting::typeAsString(NetworkManager::Setting::WirelessSecurity), wifiSecurityMap);
    }
    csMapMap.insert(NetworkManager::Setting::typeAsString(NetworkManager::Setting::Ipv4), ipv4Map);
    NetworkManager::addConnection(csMapMap);

    return true;
}
