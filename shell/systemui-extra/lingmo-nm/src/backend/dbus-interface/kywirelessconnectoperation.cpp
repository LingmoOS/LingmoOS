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
#include "kywirelessconnectoperation.h"
#include "lingmoutil.h"

#include <networkmanagerqt/wirelesssecuritysetting.h>

#define PSK_SETTING_NAME "802-11-wireless-security"
#define PRIVATE_PSK_SETTING_NAME "802-1x"

NetworkManager::ConnectionSettings::Ptr assembleWpaXPskSettings(NetworkManager::AccessPoint::Ptr accessPoint, QString &psk, bool isAutoConnect)
{
    QByteArray rawSsid = accessPoint->rawSsid();
    QString wifiSsid = getSsidFromByteArray(rawSsid);

    NetworkManager::ConnectionSettings::Ptr settings{new NetworkManager::ConnectionSettings{NetworkManager::ConnectionSettings::Wireless}};
    settings->setId(wifiSsid);
    settings->setUuid(NetworkManager::ConnectionSettings::createNewUuid());
    settings->setAutoconnect(isAutoConnect);
    //Note: workaround for wrongly (randomly) initialized gateway-ping-timeout
    settings->setGatewayPingTimeout(0);

    NetworkManager::WirelessSetting::Ptr wifi_sett
        = settings->setting(NetworkManager::Setting::Wireless).dynamicCast<NetworkManager::WirelessSetting>();
    wifi_sett->setInitialized(true);
    wifi_sett->setSsid(rawSsid);
    wifi_sett->setSecurity("802-11-wireless-security");

    NetworkManager::WirelessSecuritySetting::Ptr security_sett
        = settings->setting(NetworkManager::Setting::WirelessSecurity).dynamicCast<NetworkManager::WirelessSecuritySetting>();
    security_sett->setInitialized(true);
    if (NetworkManager::AccessPoint::Adhoc == accessPoint->mode()) {
        wifi_sett->setMode(NetworkManager::WirelessSetting::Adhoc);
        security_sett->setKeyMgmt(NetworkManager::WirelessSecuritySetting::WpaNone);
    } else {
        security_sett->setKeyMgmt(NetworkManager::WirelessSecuritySetting::WpaPsk);
    }
    if (!psk.isEmpty()) {
        security_sett->setPsk(psk);
    }

    return settings;
}

NetworkManager::ConnectionSettings::Ptr assembleSaeSettings(NetworkManager::AccessPoint::Ptr accessPoint, QString &psk, bool isAutoConnect)
{
    QByteArray rawSsid = accessPoint->rawSsid();
    QString wifiSsid = getSsidFromByteArray(rawSsid);

    NetworkManager::ConnectionSettings::Ptr settings{new NetworkManager::ConnectionSettings{NetworkManager::ConnectionSettings::Wireless}};
    settings->setId(wifiSsid);
    settings->setUuid(NetworkManager::ConnectionSettings::createNewUuid());
    settings->setAutoconnect(isAutoConnect);
    //Note: workaround for wrongly (randomly) initialized gateway-ping-timeout
    settings->setGatewayPingTimeout(0);

    NetworkManager::WirelessSetting::Ptr wifi_sett
        = settings->setting(NetworkManager::Setting::Wireless).dynamicCast<NetworkManager::WirelessSetting>();
    wifi_sett->setInitialized(true);
    wifi_sett->setSsid(rawSsid);
    wifi_sett->setSecurity("802-11-wireless-security");

    NetworkManager::WirelessSecuritySetting::Ptr security_sett
        = settings->setting(NetworkManager::Setting::WirelessSecurity).dynamicCast<NetworkManager::WirelessSecuritySetting>();
    security_sett->setInitialized(true);
    if (NetworkManager::AccessPoint::Adhoc == accessPoint->mode()) {
        wifi_sett->setMode(NetworkManager::WirelessSetting::Adhoc);
        security_sett->setKeyMgmt(NetworkManager::WirelessSecuritySetting::WpaNone);
    } else {
        security_sett->setKeyMgmt(NetworkManager::WirelessSecuritySetting::SAE);
    }
    if (!psk.isEmpty()) {
        security_sett->setPsk(psk);
    }

    return settings;
}

NetworkManager::ConnectionSettings::Ptr assembleWirelessSettings(
                                            const NetworkManager::AccessPoint::Ptr accessPointPtr,
                                            const KyWirelessConnectSetting &connSettingInfo,
                                            bool isHidden)
{
    QByteArray rawSsid;
    if (nullptr == accessPointPtr || accessPointPtr.isNull()) {
        rawSsid = connSettingInfo.m_ssid.toUtf8();
    } else {
        rawSsid = accessPointPtr->rawSsid();
    }

    NetworkManager::ConnectionSettings::Ptr settings{new NetworkManager::ConnectionSettings{NetworkManager::ConnectionSettings::Wireless}};
    settings->setId(connSettingInfo.m_connectName);
    settings->setUuid(NetworkManager::ConnectionSettings::createNewUuid());
    settings->setAutoconnect(connSettingInfo.isAutoConnect);
    //Note: workaround for wrongly (randomly) initialized gateway-ping-timeout
    settings->setGatewayPingTimeout(0);
    settings->setInterfaceName(connSettingInfo.m_ifaceName);

    NetworkManager::WirelessSetting::Ptr wifi_sett
        = settings->setting(NetworkManager::Setting::Wireless).dynamicCast<NetworkManager::WirelessSetting>();
    wifi_sett->setInitialized(true);
    wifi_sett->setSsid(rawSsid);
    wifi_sett->setSecurity("802-11-wireless-security");
    wifi_sett->setHidden(isHidden);

    if (connSettingInfo.m_type != KyKeyMgmt::WpaNone && connSettingInfo.m_type != KyKeyMgmt::Unknown)
    {
        NetworkManager::WirelessSecuritySetting::Ptr security_sett
            = settings->setting(NetworkManager::Setting::WirelessSecurity).dynamicCast<NetworkManager::WirelessSecuritySetting>();
        security_sett->setInitialized(true);
        security_sett->setKeyMgmt((NetworkManager::WirelessSecuritySetting::KeyMgmt)connSettingInfo.m_type);
        if (KyKeyMgmt::SAE == connSettingInfo.m_type || KyKeyMgmt::WpaPsk == connSettingInfo.m_type)
        {
            security_sett->setPsk(connSettingInfo.m_psk);
        }
    }
    return settings;
}


KyWirelessConnectOperation::KyWirelessConnectOperation(QObject *parent) : KyConnectOperation(parent)
{
    connect(m_networkResourceInstance, &KyNetworkResourceManager::wifiEnabledChanged,
                                        this, &KyWirelessConnectOperation::wifiEnabledChanged);
    connect(this, &KyWirelessConnectOperation::enabledWirelessNetwork,
                  m_networkResourceInstance, &KyNetworkResourceManager::setWirelessNetworkEnabled, Qt::ConnectionType::QueuedConnection);
}

KyWirelessConnectOperation::~KyWirelessConnectOperation()
{
    m_networkResourceInstance = nullptr;
}

void KyWirelessConnectOperation::activeWirelessConnect(QString devIfaceName, QString connUuid)
{
    activateConnection(connUuid, devIfaceName);
    return;
}

void KyWirelessConnectOperation::deActivateWirelessConnection(const QString activeConnectName, const QString &activeConnectUuid)
{
    deactivateConnection(activeConnectName, activeConnectUuid);
    return;
}

//普通wifi
void KyWirelessConnectOperation::addConnect(const KyWirelessConnectSetting &connSettingInfo)
{
    NetworkManager::WirelessNetwork::Ptr wifiNet =
                                checkWifiNetExist(connSettingInfo.m_ssid, connSettingInfo.m_ifaceName);
    if (wifiNet.isNull()) {
        QString errorMessage = "the ssid " + connSettingInfo.m_ssid
                                                + " is not exsit in " + connSettingInfo.m_ifaceName;
        qWarning()<<errorMessage;
        Q_EMIT createConnectionError(errorMessage);
        return;
    }

    NetworkManager::AccessPoint::Ptr accessPointPtr = wifiNet->referenceAccessPoint();

    NetworkManager::ConnectionSettings::Ptr connSetting =
                                    assembleWirelessSettings(accessPointPtr, connSettingInfo, false);
    setIpv4AndIpv6Setting(connSetting, connSettingInfo);

    QDBusPendingCallWatcher * watcher;
    watcher = new QDBusPendingCallWatcher{NetworkManager::addConnection(connSetting->toMap()), this};
    connect(watcher, &QDBusPendingCallWatcher::finished, [this](QDBusPendingCallWatcher * watcher) {
        if (watcher->isError() || !watcher->isValid()) {
            QString errorMessage = tr("create wireless connection failed: ") + watcher->error().message();
            qWarning()<<errorMessage;
            Q_EMIT this->createConnectionError(errorMessage);
        } else {
            qDebug()<<"create wireless connect complete";
        }
        watcher->deleteLater();
    });

    return;
}
//tls
void KyWirelessConnectOperation::addTlsConnect(const KyWirelessConnectSetting &connSettingInfo, const KyEapMethodTlsInfo &tlsInfo)
{
    NetworkManager::WirelessNetwork::Ptr wifiNet =
                                checkWifiNetExist(connSettingInfo.m_ssid, connSettingInfo.m_ifaceName);
    if (wifiNet.isNull()) {
        QString errorMessage = "the ssid " + connSettingInfo.m_ssid
                                                + " is not exsit in " + connSettingInfo.m_ifaceName;
        qWarning()<<errorMessage;
        Q_EMIT createConnectionError(errorMessage);
        return;
    }

    NetworkManager::AccessPoint::Ptr accessPointPtr = wifiNet->referenceAccessPoint();
    NetworkManager::ConnectionSettings::Ptr connSetting =
                                    assembleWirelessSettings(accessPointPtr, connSettingInfo, false);
    setIpv4AndIpv6Setting(connSetting, connSettingInfo);
    assembleEapMethodTlsSettings(connSetting, tlsInfo);

    QDBusPendingCallWatcher * watcher;
    watcher = new QDBusPendingCallWatcher{NetworkManager::addConnection(connSetting->toMap()), this};
    connect(watcher, &QDBusPendingCallWatcher::finished, [this](QDBusPendingCallWatcher * watcher) {
        if (watcher->isError() || !watcher->isValid()) {
            QString errorMessage = tr("create wireless tls connection failed: ") + watcher->error().message();
            qWarning()<<errorMessage;
            Q_EMIT this->createConnectionError(errorMessage);
        } else {
            qDebug()<<"create wireless connect complete";
        }
        watcher->deleteLater();
    });

    return;
}
//peap
void KyWirelessConnectOperation::addPeapConnect(const KyWirelessConnectSetting &connSettingInfo, const KyEapMethodPeapInfo &peapInfo)
{
    NetworkManager::WirelessNetwork::Ptr wifiNet =
                                checkWifiNetExist(connSettingInfo.m_ssid, connSettingInfo.m_ifaceName);
    if (wifiNet.isNull()) {
        QString errorMessage = "the ssid " + connSettingInfo.m_ssid
                                                + " is not exsit in " + connSettingInfo.m_ifaceName;
        qWarning()<<errorMessage;
        Q_EMIT createConnectionError(errorMessage);
        return;
    }

    NetworkManager::AccessPoint::Ptr accessPointPtr = wifiNet->referenceAccessPoint();
    NetworkManager::ConnectionSettings::Ptr connSetting =
                                    assembleWirelessSettings(accessPointPtr, connSettingInfo, false);
    setIpv4AndIpv6Setting(connSetting, connSettingInfo);
    assembleEapMethodPeapSettings(connSetting, peapInfo);

    QDBusPendingCallWatcher * watcher;
    watcher = new QDBusPendingCallWatcher{NetworkManager::addConnection(connSetting->toMap()), this};
    connect(watcher, &QDBusPendingCallWatcher::finished, [this](QDBusPendingCallWatcher * watcher) {
        if (watcher->isError() || !watcher->isValid()) {
            QString errorMessage = tr("create wireless peap connection failed: ") + watcher->error().message();
            qWarning()<<errorMessage;
            Q_EMIT this->createConnectionError(errorMessage);
        } else {
            qDebug()<<"create wireless connect complete";
        }
        watcher->deleteLater();
    });

    return;
}

void KyWirelessConnectOperation::addTtlsConnect(const KyWirelessConnectSetting &connSettingInfo, const KyEapMethodTtlsInfo &ttlsInfo)
{
    NetworkManager::WirelessNetwork::Ptr wifiNet =
                                checkWifiNetExist(connSettingInfo.m_ssid, connSettingInfo.m_ifaceName);
    if (wifiNet.isNull()) {
        QString errorMessage = "the ssid " + connSettingInfo.m_ssid
                                                + " is not exsit in " + connSettingInfo.m_ifaceName;
        qWarning()<<errorMessage;
        Q_EMIT createConnectionError(errorMessage);
        return;
    }

    NetworkManager::AccessPoint::Ptr accessPointPtr = wifiNet->referenceAccessPoint();
    NetworkManager::ConnectionSettings::Ptr connSetting =
                                    assembleWirelessSettings(accessPointPtr, connSettingInfo, false);
    setIpv4AndIpv6Setting(connSetting, connSettingInfo);
    assembleEapMethodTtlsSettings(connSetting, ttlsInfo);

    QDBusPendingCallWatcher * watcher;
    watcher = new QDBusPendingCallWatcher{NetworkManager::addConnection(connSetting->toMap()), this};
    connect(watcher, &QDBusPendingCallWatcher::finished, [this](QDBusPendingCallWatcher * watcher) {
        if (watcher->isError() || !watcher->isValid()) {
            QString errorMessage = tr("create wireless ttls connection failed: ") + watcher->error().message();
            qWarning()<<errorMessage;
            Q_EMIT this->createConnectionError(errorMessage);
        } else {
            qDebug()<<"create wireless connect complete";
        }
        watcher->deleteLater();
    });

    return;
}

//leap
void KyWirelessConnectOperation::addLeapConnect(const KyWirelessConnectSetting &connSettingInfo, const KyEapMethodLeapInfo &leapInfo)
{
    NetworkManager::WirelessNetwork::Ptr wifiNet =
                                checkWifiNetExist(connSettingInfo.m_ssid, connSettingInfo.m_ifaceName);
    if (wifiNet.isNull()) {
        QString errorMessage = "the ssid " + connSettingInfo.m_ssid
                                                + " is not exsit in " + connSettingInfo.m_ifaceName;
        qWarning() << errorMessage;
        Q_EMIT createConnectionError(errorMessage);
        return;
    }

    NetworkManager::AccessPoint::Ptr accessPointPtr = wifiNet->referenceAccessPoint();
    NetworkManager::ConnectionSettings::Ptr connSetting =
                                    assembleWirelessSettings(accessPointPtr, connSettingInfo, false);
    setIpv4AndIpv6Setting(connSetting, connSettingInfo);
    assembleEapMethodLeapSettings(connSetting, leapInfo);

    QDBusPendingCallWatcher * watcher;
    watcher = new QDBusPendingCallWatcher{NetworkManager::addConnection(connSetting->toMap()), this};
    connect(watcher, &QDBusPendingCallWatcher::finished, [this](QDBusPendingCallWatcher * watcher) {
        if (watcher->isError() || !watcher->isValid()) {
            QString errorMessage = tr("create wireless leap connection failed: ") + watcher->error().message();
            qWarning()<<errorMessage;
            Q_EMIT this->createConnectionError(errorMessage);
        } else {
            qDebug()<<"create wireless connect complete";
        }
        watcher->deleteLater();
    });

    return;
}

//pwd
void KyWirelessConnectOperation::addPwdConnect(const KyWirelessConnectSetting &connSettingInfo, const KyEapMethodPwdInfo &pwdInfo)
{
    NetworkManager::WirelessNetwork::Ptr wifiNet =
                                checkWifiNetExist(connSettingInfo.m_ssid, connSettingInfo.m_ifaceName);
    if (wifiNet.isNull()) {
        QString errorMessage = "the ssid " + connSettingInfo.m_ssid
                                                + " is not exsit in " + connSettingInfo.m_ifaceName;
        qWarning() << errorMessage;
        Q_EMIT createConnectionError(errorMessage);
        return;
    }

    NetworkManager::AccessPoint::Ptr accessPointPtr = wifiNet->referenceAccessPoint();
    NetworkManager::ConnectionSettings::Ptr connSetting =
                                    assembleWirelessSettings(accessPointPtr, connSettingInfo, false);
    setIpv4AndIpv6Setting(connSetting, connSettingInfo);
    assembleEapMethodPwdSettings(connSetting, pwdInfo);

    QDBusPendingCallWatcher * watcher;
    watcher = new QDBusPendingCallWatcher{NetworkManager::addConnection(connSetting->toMap()), this};
    connect(watcher, &QDBusPendingCallWatcher::finished, [this](QDBusPendingCallWatcher * watcher) {
        if (watcher->isError() || !watcher->isValid()) {
            QString errorMessage = tr("create wireless pwd connection failed: ") + watcher->error().message();
            qWarning()<<errorMessage;
            Q_EMIT this->createConnectionError(errorMessage);
        } else {
            qDebug()<<"create wireless connect complete";
        }
        watcher->deleteLater();
    });

    return;
}

//fast
void KyWirelessConnectOperation::addFastConnect(const KyWirelessConnectSetting &connSettingInfo, const KyEapMethodFastInfo &fastInfo)
{
    NetworkManager::WirelessNetwork::Ptr wifiNet =
                                checkWifiNetExist(connSettingInfo.m_ssid, connSettingInfo.m_ifaceName);
    if (wifiNet.isNull()) {
        QString errorMessage = "the ssid " + connSettingInfo.m_ssid
                                                + " is not exsit in " + connSettingInfo.m_ifaceName;
        qWarning() << errorMessage;
        Q_EMIT createConnectionError(errorMessage);
        return;
    }

    NetworkManager::AccessPoint::Ptr accessPointPtr = wifiNet->referenceAccessPoint();
    NetworkManager::ConnectionSettings::Ptr connSetting =
                                    assembleWirelessSettings(accessPointPtr, connSettingInfo, false);
    setIpv4AndIpv6Setting(connSetting, connSettingInfo);
    assembleEapMethodFastSettings(connSetting, fastInfo);

    QDBusPendingCallWatcher * watcher;
    watcher = new QDBusPendingCallWatcher{NetworkManager::addConnection(connSetting->toMap()), this};
    connect(watcher, &QDBusPendingCallWatcher::finished, [this](QDBusPendingCallWatcher * watcher) {
        if (watcher->isError() || !watcher->isValid()) {
            QString errorMessage = tr("create wireless fast connection failed: ") + watcher->error().message();
            qWarning()<<errorMessage;
            Q_EMIT this->createConnectionError(errorMessage);
        } else {
            qDebug()<<"create wireless connect complete";
        }
        watcher->deleteLater();
    });
}

void KyWirelessConnectOperation::setWirelessAutoConnect(const QString &uuid, bool bAutoConnect)
{
    NetworkManager::Connection::Ptr connectPtr =
            NetworkManager::findConnectionByUuid(uuid);
    if (nullptr == connectPtr) {
        QString errorMessage = tr("it can not find connection") + uuid;
        qWarning()<<errorMessage;
        Q_EMIT updateConnectionError(errorMessage);
        return;
    }
    NetworkManager::ConnectionSettings::Ptr connectionSettings = connectPtr->settings();
    setAutoConnect(connectionSettings, bAutoConnect);
    connectPtr->update(connectionSettings->toMap());
    return;
}

void KyWirelessConnectOperation::deleteWirelessConnect(const QString &connectUuid)
{
    qDebug()<<"delete wireless connect uuid " << connectUuid;

    deleteConnect(connectUuid);

    return ;
}

QString KyWirelessConnectOperation::getPsk(const QString &connectUuid)
{
    qDebug() << "getPsk" << connectUuid;
    NetworkManager::Connection::Ptr connectPtr =
            NetworkManager::findConnectionByUuid(connectUuid);
    if (connectPtr.isNull()) {
        QString errorMessage = tr("it can not find connection") + connectUuid;
        qWarning()<<errorMessage;
        return "";
    }
    QDBusPendingReply<NMVariantMapMap> reply = connectPtr->secrets(PSK_SETTING_NAME);
    QMap<QString,QVariantMap> map(reply.value());
    if (map.contains("802-11-wireless-security")
            && map.value("802-11-wireless-security").contains("psk"))     {
        QString psk = map.value("802-11-wireless-security").value("psk").toString();
        return  psk;
    }
    return "";
}

QString KyWirelessConnectOperation::getPrivateKeyPassword(const QString &connectUuid)
{
    qDebug() << "getPsk" << connectUuid;
    NetworkManager::Connection::Ptr connectPtr =
            NetworkManager::findConnectionByUuid(connectUuid);
    if (connectPtr.isNull()) {
        QString errorMessage = tr("it can not find connection") + connectUuid;
        qWarning()<<errorMessage;
        return "";
    }
    QDBusInterface dbusInterface("org.freedesktop.NetworkManager",
                              connectPtr->path(),
                              "org.freedesktop.NetworkManager.Settings.Connection",
                              QDBusConnection::systemBus());
    dbusInterface.setTimeout(500);
    QDBusMessage result = dbusInterface.call("GetSecrets", PRIVATE_PSK_SETTING_NAME);
    const QDBusArgument &dbusArg1st = result.arguments().at( 0 ).value<QDBusArgument>();
    QMap<QString,QVariantMap> map;
    dbusArg1st >> map;
    if (map.contains("802-1x")
            && map.value("802-1x").contains("private-key-password")) {
        QString psk = map.value("802-1x").value("private-key-password").toString();
        return  psk;
    }
    return "";
}

QString KyWirelessConnectOperation::get8021xPassword(const QString &connectUuid)
{
    qDebug() << "getPsk" << connectUuid;
    NetworkManager::Connection::Ptr connectPtr =
            NetworkManager::findConnectionByUuid(connectUuid);
    if (connectPtr.isNull()) {
        QString errorMessage = tr("it can not find connection") + connectUuid;
        qWarning()<<errorMessage;
        return "";
    }

    QDBusInterface dbusInterface("org.freedesktop.NetworkManager",
                              connectPtr->path(),
                              "org.freedesktop.NetworkManager.Settings.Connection",
                              QDBusConnection::systemBus());
    dbusInterface.setTimeout(500);
    QDBusMessage result = dbusInterface.call("GetSecrets", PRIVATE_PSK_SETTING_NAME);
    const QDBusArgument &dbusArg1st = result.arguments().at( 0 ).value<QDBusArgument>();
    QMap<QString,QVariantMap> map;
    dbusArg1st >> map;

//    QDBusPendingReply<NMVariantMapMap> reply = connectPtr->secrets(PRIVATE_PSK_SETTING_NAME);
//    QMap<QString,QVariantMap> map(reply.value());
    if (map.contains("802-1x") && map.value("802-1x").contains("password"))
    {
        QString psk = map.value("802-1x").value("password").toString();
        return  psk;
    }
    return "";
}

void KyWirelessConnectOperation::updateIpv4AndIpv6SettingInfo(const QString &uuid, const KyConnectSetting &connectSettingsInfo)
{
    qDebug()<<"updateIpv4AndIpv6SettingInfo wireless connect uuid " << uuid;

    NetworkManager::Connection::Ptr connectPtr =
            NetworkManager::findConnectionByUuid(uuid);
    if (nullptr == connectPtr) {
        QString errorMessage = tr("it can not find connection") + uuid;
        qWarning()<<errorMessage;
        Q_EMIT updateConnectionError(errorMessage);
        return;
    }
    NetworkManager::ConnectionSettings::Ptr connectionSettings = connectPtr->settings();

    setIpv4AndIpv6Setting(connectionSettings,connectSettingsInfo);
    connectPtr->update(connectionSettings->toMap());
    return;
}

void KyWirelessConnectOperation::updateWirelessPersonalConnect(const QString &uuid, const KyWirelessConnectSetting &connSettingInfo, bool bPwdChange)
{
    NetworkManager::Connection::Ptr connectPtr =
            NetworkManager::findConnectionByUuid(uuid);
    if (nullptr == connectPtr) {
        QString errorMessage = tr("it can not find connection") + uuid;
        qWarning()<<errorMessage;
        Q_EMIT updateConnectionError(errorMessage);
        return;
    }
    NetworkManager::ConnectionSettings::Ptr connectionSettings = connectPtr->settings();

    if (connSettingInfo.m_type != Unknown &&connSettingInfo.m_type != WpaNone && connSettingInfo.m_type != WpaPsk && connSettingInfo.m_type != SAE)
    {
        qDebug() << "updateWirelessPersonalConnect " << connSettingInfo.m_type << " not support";
        return;
    }

    NetworkManager::Security8021xSetting::Ptr wifi_8021x_sett
            = connectionSettings->setting(NetworkManager::Setting::Security8021x).dynamicCast<NetworkManager::Security8021xSetting>();
    wifi_8021x_sett->setInitialized(false);

    updateWirelessSecu(connectionSettings, connSettingInfo, bPwdChange);
    connectPtr->update(connectionSettings->toMap());
    return;
}

void KyWirelessConnectOperation::updateWirelessEnterPriseTlsConnect(const QString &uuid, const KyEapMethodTlsInfo &tlsInfo)
{
    NetworkManager::Connection::Ptr connectPtr =
            NetworkManager::findConnectionByUuid(uuid);
    if (connectPtr.isNull()) {
        QString errorMessage = tr("it can not find connection") + uuid;
        qWarning()<<errorMessage;
        Q_EMIT updateConnectionError(errorMessage);
        return;
    }
    NetworkManager::ConnectionSettings::Ptr connectionSettings = connectPtr->settings();

    setWirelessSecuWpaXEap(connectionSettings);
    modifyEapMethodTlsSettings(connectionSettings, tlsInfo);
    connectPtr->update(connectionSettings->toMap());
    return;
}

void KyWirelessConnectOperation::updateWirelessEnterPrisePeapConnect(const QString &uuid, const KyEapMethodPeapInfo &peapInfo)
{
    NetworkManager::Connection::Ptr connectPtr =
            NetworkManager::findConnectionByUuid(uuid);
    if (nullptr == connectPtr) {
        QString errorMessage = tr("it can not find connection") + uuid;
        qWarning()<<errorMessage;
        Q_EMIT updateConnectionError(errorMessage);
        return;
    }
    NetworkManager::ConnectionSettings::Ptr connectionSettings = connectPtr->settings();

    setWirelessSecuWpaXEap(connectionSettings);
    modifyEapMethodPeapSettings(connectionSettings, peapInfo);
    connectPtr->update(connectionSettings->toMap());
    return;
}

void KyWirelessConnectOperation::updateWirelessEnterPriseTtlsConnect(const QString &uuid, const KyEapMethodTtlsInfo &ttlsInfo)
{
    NetworkManager::Connection::Ptr connectPtr =
            NetworkManager::findConnectionByUuid(uuid);
    if (nullptr == connectPtr) {
        QString errorMessage = tr("it can not find connection") + uuid;
        qWarning()<<errorMessage;
        Q_EMIT updateConnectionError(errorMessage);
        return;
    }
    NetworkManager::ConnectionSettings::Ptr connectionSettings = connectPtr->settings();

    setWirelessSecuWpaXEap(connectionSettings);
    modifyEapMethodTtlsSettings(connectionSettings, ttlsInfo);
    connectPtr->update(connectionSettings->toMap());
    return;
}

void KyWirelessConnectOperation::updateWirelessEnterPriseLeapConnect(const QString &uuid, const KyEapMethodLeapInfo &leapInfo)
{
    NetworkManager::Connection::Ptr connectPtr =
            NetworkManager::findConnectionByUuid(uuid);
    if (nullptr == connectPtr) {
        QString errorMessage = tr("it can not find connection") + uuid;
        qWarning()<<errorMessage;
        Q_EMIT updateConnectionError(errorMessage);
        return;
    }
    NetworkManager::ConnectionSettings::Ptr connectionSettings = connectPtr->settings();

    setWirelessSecuWpaXEap(connectionSettings);
    modifyEapMethodLeapSettings(connectionSettings, leapInfo);
    connectPtr->update(connectionSettings->toMap());
    return;
}

void KyWirelessConnectOperation::updateWirelessEnterPrisePwdConnect(const QString &uuid, const KyEapMethodPwdInfo &pwdInfo)
{
    NetworkManager::Connection::Ptr connectPtr =
            NetworkManager::findConnectionByUuid(uuid);
    if (nullptr == connectPtr) {
        QString errorMessage = tr("it can not find connection") + uuid;
        qWarning()<<errorMessage;
        Q_EMIT updateConnectionError(errorMessage);
        return;
    }
    NetworkManager::ConnectionSettings::Ptr connectionSettings = connectPtr->settings();

    setWirelessSecuWpaXEap(connectionSettings);
    modifyEapMethodPwdSettings(connectionSettings, pwdInfo);
    connectPtr->update(connectionSettings->toMap());
    return;
}

void KyWirelessConnectOperation::updateWirelessEnterPriseFastConnect(const QString &uuid, const KyEapMethodFastInfo &fastInfo)
{
    NetworkManager::Connection::Ptr connectPtr =
            NetworkManager::findConnectionByUuid(uuid);
    if (nullptr == connectPtr) {
        QString errorMessage = tr("it can not find connection") + uuid;
        qWarning()<<errorMessage;
        Q_EMIT updateConnectionError(errorMessage);
        return;
    }
    NetworkManager::ConnectionSettings::Ptr connectionSettings = connectPtr->settings();

    setWirelessSecuWpaXEap(connectionSettings);
    modifyEapMethodFastSettings(connectionSettings, fastInfo);
    connectPtr->update(connectionSettings->toMap());
    return;
}

void KyWirelessConnectOperation::addAndActiveWirelessConnect(QString & devIface,KyWirelessConnectSetting &connSettingInfo,bool isHidden)
{
    qDebug() << "addAndActiveWirelessConnect" << connSettingInfo.m_ssid << devIface <<connSettingInfo.m_psk;
    QString conn_uni;
    QString dev_uni;
    QString conn_name;
    QString spec_object;
    NMVariantMapMap map_settings;

    NetworkManager::WirelessNetwork::Ptr wifiNet = checkWifiNetExist(connSettingInfo.m_ssid, devIface);
    if (!isHidden && wifiNet.isNull()) {
        QString errorMessage = "the ssid " + connSettingInfo.m_ssid + " is not exsit in " + devIface;
        qWarning()<<errorMessage;
        Q_EMIT activateConnectionError(errorMessage);
        return;
    }

    qDebug() << "addAndActiveWirelessConnect  find " << connSettingInfo.m_ssid << " in " << devIface << " or hide";

    auto dev = m_networkResourceInstance->findDeviceInterface(devIface);
    if (dev.isNull()) {
        Q_EMIT addAndActivateConnectionError("can not find device");
        return;
    }

    qDebug() << "addAndActiveWirelessConnect device " << devIface << " exist";
    dev_uni = dev->uni();
    auto spec_dev = dev->as<NetworkManager::WirelessDevice>();
    if (!isHidden) {
        qDebug() << "start assemble";
        auto access_point = wifiNet->referenceAccessPoint();
        conn_uni = access_point->uni();
        conn_name = access_point->ssid();
        spec_object = conn_uni;

        NetworkManager::WirelessSecurityType sec_type = NetworkManager::findBestWirelessSecurity(spec_dev->wirelessCapabilities()
                                                                                                 , true, (spec_dev->mode() == NetworkManager::WirelessDevice::Adhoc)
                                                                                                 , access_point->capabilities(), access_point->wpaFlags(), access_point->rsnFlags());

        qDebug() << "findBestWirelessSecurity type "<< sec_type;

        NetworkManager::ConnectionSettings::Ptr settings = nullptr;
        switch (sec_type)
        {
        case NetworkManager::UnknownSecurity:
            qWarning() << QStringLiteral("unknown security to use for '%1'").arg(conn_name);
        case NetworkManager::NoneSecurity:
            //nothing to do
            break;
        case NetworkManager::WpaPsk:
        case NetworkManager::Wpa2Psk:
            settings = assembleWpaXPskSettings(access_point, connSettingInfo.m_psk, connSettingInfo.isAutoConnect);
            if (nullptr != settings) {
                map_settings = settings->toMap();
            } else {
                qWarning() << QStringLiteral("connection settings assembly for '%1' failed, abandoning activation...").arg(conn_name);
                return;
            }
            break;
            //TODO: other types...
        case NetworkManager::SAE:
            settings = assembleSaeSettings(access_point, connSettingInfo.m_psk, connSettingInfo.isAutoConnect);
            if (nullptr != settings) {
                map_settings = settings->toMap();
            } else {
                qWarning() << QStringLiteral("connection settings assembly for '%1' failed, abandoning activation...").arg(conn_name);
                return;
            }
            break;
        default:
            qDebug() << "addAndActiveWirelessConnect not support";
            break;
        }

        qDebug() << "finish assemble";
    } else {
        qDebug() << "start assembleWirelessSettings";
        map_settings = assembleWirelessSettings(nullptr, connSettingInfo, isHidden)->toMap();
        qDebug() << "finish assembleWirelessSettings";
    }

    QDBusPendingCallWatcher * watcher;
    watcher = new QDBusPendingCallWatcher{NetworkManager::addAndActivateConnection(map_settings, dev_uni, spec_object), this};
    connect(watcher, &QDBusPendingCallWatcher::finished, [&] (QDBusPendingCallWatcher * watcher) {
        if (watcher->isError() || !watcher->isValid()) {
            QString errorMessage = watcher->error().message();
            qDebug() << "activation of connection failed " << errorMessage;
            Q_EMIT addAndActivateConnectionError(errorMessage);
        }
        watcher->deleteLater();
    });
}

void KyWirelessConnectOperation::requestWirelessScan()
{
    for (auto const & dev : m_networkResourceInstance->m_devices) {
        auto spec_dev = dev->as<NetworkManager::WirelessDevice>();
        if (nullptr != spec_dev) {
            m_networkResourceInstance->requestScan(spec_dev);
        }
    }
}

void KyWirelessConnectOperation::addAndActiveWirelessEnterPriseTlsConnect(KyEapMethodTlsInfo &info, KyWirelessConnectSetting &connSettingInfo,
                                                                          QString & devIface, bool isHidden)
{
    QString conn_uni;
    QString dev_uni;
    QString spec_object;
    NMVariantMapMap map_settings;
    NetworkManager::AccessPoint::Ptr accessPointPtr = nullptr;

    if (!isHidden) {
        NetworkManager::WirelessNetwork::Ptr wifiNet = checkWifiNetExist(connSettingInfo.m_ssid, devIface);
        if (wifiNet.isNull()) {
            QString errorMessage = "the ssid " + connSettingInfo.m_ssid + " is not exsit in " + devIface;
            qWarning()<<errorMessage;
            Q_EMIT activateConnectionError(errorMessage);
            return;
        }

        accessPointPtr = wifiNet->referenceAccessPoint();
        conn_uni = accessPointPtr->uni();
        spec_object = conn_uni;
    }

    auto dev = m_networkResourceInstance->findDeviceInterface(devIface);
    if (dev.isNull()) {
        Q_EMIT addAndActivateConnectionError("can not find device");
        return;
    }
    dev_uni = dev->uni();

    NetworkManager::ConnectionSettings::Ptr settings = assembleWirelessSettings(accessPointPtr, connSettingInfo, isHidden);
    assembleEapMethodTlsSettings(settings, info);

    if(settings.isNull()) {
        qDebug() << "assembleEapMethodTlsSettings failed";
        return;
    }

    map_settings = settings->toMap();

    QDBusPendingCallWatcher * watcher;
    watcher = new QDBusPendingCallWatcher{NetworkManager::addAndActivateConnection(map_settings, dev_uni, spec_object), this};
    connect(watcher, &QDBusPendingCallWatcher::finished, [&] (QDBusPendingCallWatcher * watcher) {
        if (watcher->isError() || !watcher->isValid()) {
            QString errorMessage = watcher->error().message();
            qDebug() << "addAndActiveWirelessEnterPriseTlsConnect failed " << errorMessage;
            Q_EMIT addAndActivateConnectionError(errorMessage);
        }
        watcher->deleteLater();
    });

}

void KyWirelessConnectOperation::addAndActiveWirelessEnterPrisePeapConnect(KyEapMethodPeapInfo &info, KyWirelessConnectSetting &connSettingInfo,
                                                                           QString & devIface, bool isHidden)
{
    qDebug() <<"addAndActiveWirelessEnterPrisePeapConnect";
    QString conn_uni;
    QString dev_uni;
    QString spec_object;
    NMVariantMapMap map_settings;
    NetworkManager::AccessPoint::Ptr accessPointPtr = nullptr;

    if (!isHidden) {
        NetworkManager::WirelessNetwork::Ptr wifiNet = checkWifiNetExist(connSettingInfo.m_ssid, devIface);
        if (wifiNet.isNull()) {
            QString errorMessage = "the ssid " + connSettingInfo.m_ssid + " is not exsit in " + devIface;
            qWarning()<<errorMessage;
            Q_EMIT activateConnectionError(errorMessage);
            return;
        }

        accessPointPtr = wifiNet->referenceAccessPoint();
        conn_uni = accessPointPtr->uni();
        spec_object = conn_uni;
    }

    auto dev = m_networkResourceInstance->findDeviceInterface(devIface);
    if (dev.isNull()) {
        Q_EMIT addAndActivateConnectionError("can not find device");
        return;
    }
    dev_uni = dev->uni();

    NetworkManager::ConnectionSettings::Ptr settings =
                                assembleWirelessSettings(accessPointPtr, connSettingInfo, isHidden);
    assembleEapMethodPeapSettings(settings, info);

    if(settings.isNull()) {
        qDebug() << "assembleEapMethodPeapSettings failed";
        return;
    }
    map_settings = settings->toMap();

    QDBusPendingCallWatcher * watcher;
    watcher = new QDBusPendingCallWatcher{NetworkManager::addAndActivateConnection(map_settings, dev_uni, spec_object), this};
    connect(watcher, &QDBusPendingCallWatcher::finished, [&] (QDBusPendingCallWatcher * watcher) {
        if (watcher->isError() || !watcher->isValid()) {
            QString errorMessage = watcher->error().message();
            qDebug() << "addAndActiveWirelessEnterPrisePeapConnect failed " << errorMessage;
            Q_EMIT addAndActivateConnectionError(errorMessage);
        }
        watcher->deleteLater();
    });

}

void KyWirelessConnectOperation::addAndActiveWirelessEnterPriseTtlsConnect(KyEapMethodTtlsInfo &info, KyWirelessConnectSetting &connSettingInfo,
                                                                           QString & devIface, bool isHidden)
{
    QString conn_uni;
    QString dev_uni;
    QString spec_object;
    NMVariantMapMap map_settings;
    NetworkManager::AccessPoint::Ptr accessPointPtr = nullptr;

    if (!isHidden) {
        NetworkManager::WirelessNetwork::Ptr wifiNet = checkWifiNetExist(connSettingInfo.m_ssid, devIface);
        if (wifiNet.isNull()) {
            QString errorMessage = "the ssid " + connSettingInfo.m_ssid + " is not exsit in " + devIface;
            qWarning()<<errorMessage;
            Q_EMIT activateConnectionError(errorMessage);
            return;
        }

        accessPointPtr = wifiNet->referenceAccessPoint();
        conn_uni = accessPointPtr->uni();
        spec_object = conn_uni;
    }

    auto dev = m_networkResourceInstance->findDeviceInterface(devIface);
    if (dev.isNull()) {
        Q_EMIT addAndActivateConnectionError("can not find device");
        return;
    }
    dev_uni = dev->uni();

    NetworkManager::ConnectionSettings::Ptr settings =
                        assembleWirelessSettings(accessPointPtr, connSettingInfo, isHidden);
    assembleEapMethodTtlsSettings(settings, info);

    if(settings.isNull()) {
        qDebug() << "assembleEapMethodTtlsSettings failed";
        return;
    }

    map_settings = settings->toMap();

    QDBusPendingCallWatcher * watcher;
    watcher = new QDBusPendingCallWatcher{NetworkManager::addAndActivateConnection(map_settings, dev_uni, spec_object), this};
    connect(watcher, &QDBusPendingCallWatcher::finished, [&] (QDBusPendingCallWatcher * watcher) {
        if (watcher->isError() || !watcher->isValid()) {
            QString errorMessage = watcher->error().message();
            qDebug() << "addAndActiveWirelessEnterPriseTtlsConnect failed " << errorMessage;
            Q_EMIT addAndActivateConnectionError(errorMessage);
        }
        watcher->deleteLater();
    });
}

void KyWirelessConnectOperation::addAndActiveWirelessEnterPriseLeapConnect(KyEapMethodLeapInfo &info, KyWirelessConnectSetting &connSettingInfo, QString &devIface, bool isHidden)
{
    QString conn_uni;
    QString dev_uni;
    QString spec_object;
    NMVariantMapMap map_settings;
    NetworkManager::AccessPoint::Ptr accessPointPtr = nullptr;

    if (!isHidden) {
        NetworkManager::WirelessNetwork::Ptr wifiNet = checkWifiNetExist(connSettingInfo.m_ssid, devIface);
        if (wifiNet.isNull()) {
            QString errorMessage = "the ssid " + connSettingInfo.m_ssid + " is not exsit in " + devIface;
            qWarning()<<errorMessage;
            Q_EMIT activateConnectionError(errorMessage);
            return;
        }

        accessPointPtr = wifiNet->referenceAccessPoint();
        conn_uni = accessPointPtr->uni();
        spec_object = conn_uni;
    }

    auto dev = m_networkResourceInstance->findDeviceInterface(devIface);
    if (dev.isNull()) {
        Q_EMIT addAndActivateConnectionError("can not find device");
        return;
    }
    dev_uni = dev->uni();

    NetworkManager::ConnectionSettings::Ptr settings =
                        assembleWirelessSettings(accessPointPtr, connSettingInfo, isHidden);
    assembleEapMethodLeapSettings(settings, info);

    if(settings.isNull()) {
        qDebug() << "assembleEapMethodLeapSettings failed";
        return;
    }

    map_settings = settings->toMap();

    QDBusPendingCallWatcher * watcher;
    watcher = new QDBusPendingCallWatcher{NetworkManager::addAndActivateConnection(map_settings, dev_uni, spec_object), this};
    connect(watcher, &QDBusPendingCallWatcher::finished, [&] (QDBusPendingCallWatcher * watcher) {
        if (watcher->isError() || !watcher->isValid()) {
            QString errorMessage = watcher->error().message();
            qDebug() << "addAndActiveWirelessEnterPriseLeapConnect failed " << errorMessage;
            Q_EMIT addAndActivateConnectionError(errorMessage);
        }
        watcher->deleteLater();
    });
}

void KyWirelessConnectOperation::addAndActiveWirelessEnterPrisePwdConnect(KyEapMethodPwdInfo &info, KyWirelessConnectSetting &connSettingInfo, QString &devIface, bool isHidden)
{
    QString conn_uni;
    QString dev_uni;
    QString spec_object;
    NMVariantMapMap map_settings;
    NetworkManager::AccessPoint::Ptr accessPointPtr = nullptr;

    if (!isHidden) {
        NetworkManager::WirelessNetwork::Ptr wifiNet = checkWifiNetExist(connSettingInfo.m_ssid, devIface);
        if (wifiNet.isNull()) {
            QString errorMessage = "the ssid " + connSettingInfo.m_ssid + " is not exsit in " + devIface;
            qWarning()<<errorMessage;
            Q_EMIT activateConnectionError(errorMessage);
            return;
        }

        accessPointPtr = wifiNet->referenceAccessPoint();
        conn_uni = accessPointPtr->uni();
        spec_object = conn_uni;
    }

    auto dev = m_networkResourceInstance->findDeviceInterface(devIface);
    if (dev.isNull()) {
        Q_EMIT addAndActivateConnectionError("can not find device");
        return;
    }
    dev_uni = dev->uni();

    NetworkManager::ConnectionSettings::Ptr settings =
                        assembleWirelessSettings(accessPointPtr, connSettingInfo, isHidden);
    assembleEapMethodPwdSettings(settings, info);

    if(settings.isNull()) {
        qDebug() << "assembleEapMethodPwdSettings failed";
        return;
    }

    map_settings = settings->toMap();

    QDBusPendingCallWatcher * watcher;
    watcher = new QDBusPendingCallWatcher{NetworkManager::addAndActivateConnection(map_settings, dev_uni, spec_object), this};
    connect(watcher, &QDBusPendingCallWatcher::finished, [&] (QDBusPendingCallWatcher * watcher) {
        if (watcher->isError() || !watcher->isValid()) {
            QString errorMessage = watcher->error().message();
            qDebug() << "addAndActiveWirelessEnterPrisePwdConnect failed " << errorMessage;
            Q_EMIT addAndActivateConnectionError(errorMessage);
        }
        watcher->deleteLater();
    });
}

void KyWirelessConnectOperation::addAndActiveWirelessEnterPriseFastConnect(KyEapMethodFastInfo &info, KyWirelessConnectSetting &connSettingInfo, QString &devIface, bool isHidden)
{
    QString conn_uni;
    QString dev_uni;
    QString spec_object;
    NMVariantMapMap map_settings;
    NetworkManager::AccessPoint::Ptr accessPointPtr = nullptr;

    if (!isHidden) {
        NetworkManager::WirelessNetwork::Ptr wifiNet = checkWifiNetExist(connSettingInfo.m_ssid, devIface);
        if (wifiNet.isNull()) {
            QString errorMessage = "the ssid " + connSettingInfo.m_ssid + " is not exsit in " + devIface;
            qWarning()<<errorMessage;
            Q_EMIT activateConnectionError(errorMessage);
            return;
        }

        accessPointPtr = wifiNet->referenceAccessPoint();
        conn_uni = accessPointPtr->uni();
        spec_object = conn_uni;
    }

    auto dev = m_networkResourceInstance->findDeviceInterface(devIface);
    if (dev.isNull()) {
        Q_EMIT addAndActivateConnectionError("can not find device");
        return;
    }
    dev_uni = dev->uni();

    NetworkManager::ConnectionSettings::Ptr settings =
                        assembleWirelessSettings(accessPointPtr, connSettingInfo, isHidden);
    assembleEapMethodFastSettings(settings, info);

    if(settings.isNull()) {
        qDebug() << "assembleEapMethodFastSettings failed";
        return;
    }

    map_settings = settings->toMap();

    QDBusPendingCallWatcher * watcher;
    watcher = new QDBusPendingCallWatcher{NetworkManager::addAndActivateConnection(map_settings, dev_uni, spec_object), this};
    connect(watcher, &QDBusPendingCallWatcher::finished, [&] (QDBusPendingCallWatcher * watcher) {
        if (watcher->isError() || !watcher->isValid()) {
            QString errorMessage = watcher->error().message();
            qDebug() << "addAndActiveWirelessEnterPriseFastConnect failed " << errorMessage;
            Q_EMIT addAndActivateConnectionError(errorMessage);
        }
        watcher->deleteLater();
    });
}

//无线网络开关设置
void KyWirelessConnectOperation::setWirelessEnabled(bool enabled)
{
    Q_EMIT enabledWirelessNetwork(enabled);

    return;
}

bool KyWirelessConnectOperation::getWirelessEnabled()
{
    return NetworkManager::isWirelessEnabled();
}

bool KyWirelessConnectOperation::getConnSecretFlags(QString &connUuid, NetworkManager::Setting::SecretFlags &flag)
{
    NetworkManager::Connection::Ptr conn;
    conn = m_networkResourceInstance->getConnect(connUuid);
    if (conn.isNull()) {
        qDebug() <<"get failed";
        return false;
    }

    NetworkManager::WirelessSecuritySetting::Ptr security_sett
            = conn->settings()->setting(NetworkManager::Setting::WirelessSecurity).dynamicCast<NetworkManager::WirelessSecuritySetting>();
    flag = security_sett->pskFlags();
    return true;
}

NetworkManager::ConnectionSettings::Ptr
    KyWirelessConnectOperation::createWirelessApSetting(const QString apSsid,
                                                        const QString apPassword,
                                                        const QString apDevice,
                                                        const QString wirelessBand)
{
    NetworkManager::ConnectionSettings::Ptr connectionSettings =
                NetworkManager::ConnectionSettings::Ptr(new NetworkManager::ConnectionSettings(NetworkManager::ConnectionSettings::Wireless));
    connectionSettings->setId(apSsid);
    connectionSettings->setUuid(NetworkManager::ConnectionSettings::createNewUuid());
    connectionSettings->setAutoconnect(false);
    connectionSettings->setAutoconnectPriority(0);
    connectionSettings->setInterfaceName(apDevice);
    //Note: workaround for wrongly (randomly) initialized gateway-ping-timeout
    connectionSettings->setGatewayPingTimeout(0);

    NetworkManager::Ipv4Setting::Ptr ipv4Setting = connectionSettings->setting(NetworkManager::Setting::Ipv4).dynamicCast<NetworkManager::Ipv4Setting>();
    ipv4Setting->setInitialized(true);
    ipv4Setting->setMethod(NetworkManager::Ipv4Setting::Shared);

    NetworkManager::Ipv6Setting::Ptr ipv6Setting = connectionSettings->setting(NetworkManager::Setting::Ipv6).dynamicCast<NetworkManager::Ipv6Setting>();
    ipv6Setting->setInitialized(true);
    ipv6Setting->setMethod(NetworkManager::Ipv6Setting::Ignored);

    NetworkManager::WirelessSetting::Ptr wirelessSetting
        = connectionSettings->setting(NetworkManager::Setting::Wireless).dynamicCast<NetworkManager::WirelessSetting>();
    wirelessSetting->setInitialized(true);
    wirelessSetting->setSsid(apSsid.toUtf8());
    wirelessSetting->setMode(NetworkManager::WirelessSetting::NetworkMode::Ap);
    wirelessSetting->setSecurity("802-11-wireless-security");
    if (wirelessBand == WIFI_BAND_2_4GHZ) {
        wirelessSetting->setBand(NetworkManager::WirelessSetting::FrequencyBand::Bg);
    } else if (wirelessBand == WIFI_BAND_5GHZ) {
        wirelessSetting->setBand(NetworkManager::WirelessSetting::FrequencyBand::A);
    } else {
        qWarning()<<"[KyWirelessConnectOperation] the band type undefined"<<wirelessBand;
        wirelessSetting->setBand(NetworkManager::WirelessSetting::FrequencyBand::Automatic);
    }


    NetworkManager::WirelessSecuritySetting::Ptr wirelessSecuritySetting
        = connectionSettings->setting(NetworkManager::Setting::WirelessSecurity).dynamicCast<NetworkManager::WirelessSecuritySetting>();
    if (apPassword.isEmpty()) {
        wirelessSecuritySetting->setInitialized(false);
    } else {
        wirelessSecuritySetting->setInitialized(true);
        wirelessSecuritySetting->setKeyMgmt(NetworkManager::WirelessSecuritySetting::WpaPsk);
        wirelessSecuritySetting->setPsk(apPassword);
    }

    return connectionSettings;
}

QStringList KyWirelessConnectOperation::getBlackListHostName(QString apConnectPath)
{
    QStringList blackList;
    blackList.clear();

    QDBusInterface dbusInterface("org.freedesktop.NetworkManager",
                              apConnectPath,
                              "org.freedesktop.NetworkManager.Settings.Connection",
                              QDBusConnection::systemBus());
    if (!dbusInterface.isValid()) {
        qWarning()<<Q_FUNC_INFO<<__LINE__<<"dbusInterface error! apConnectPath:"<<apConnectPath;
        return blackList;
    }

    QDBusMessage result = dbusInterface.call("GetSettings");
    const QDBusArgument &dbusArg1st = result.arguments().at( 0 ).value<QDBusArgument>();
    QMap<QString, QMap<QString, QVariant>> map;
    dbusArg1st >> map;
    if (map.isEmpty()) {
        qWarning() << Q_FUNC_INFO << __LINE__ <<"map is empty!";
        return blackList;
    }

    QMap<QString,QVariant> wirelessMap = map.value(KEY_802_11_WIRELESS);
    if (wirelessMap.isEmpty()) {
        qWarning() << Q_FUNC_INFO << __LINE__ <<"wirelessMap is empty!";
        return blackList;
    }
    if (wirelessMap.contains(KEY_BLACKLIST_HOSTNAME)) {
        blackList = wirelessMap.value(KEY_BLACKLIST_HOSTNAME).toStringList();
    }
    return blackList;
}

void KyWirelessConnectOperation::updateWirelessApSetting(
        NetworkManager::Connection::Ptr apConnectPtr,
        const QString apName, const QString apPassword,
        const QString apDevice, const QString wirelessBand)

{
    NetworkManager::ConnectionSettings::Ptr apConnectSettingPtr = apConnectPtr->settings();
    apConnectSettingPtr->setId(apName);
    apConnectSettingPtr->setInterfaceName(apDevice);
    apConnectSettingPtr->setAutoconnect(false);

    NetworkManager::WirelessSetting::Ptr wirelessSetting
        = apConnectSettingPtr->setting(NetworkManager::Setting::Wireless).dynamicCast<NetworkManager::WirelessSetting>();
    wirelessSetting->setInitialized(true);
    wirelessSetting->setSsid(apName.toUtf8());
    if (wirelessBand == WIFI_BAND_2_4GHZ) {
        wirelessSetting->setBand(NetworkManager::WirelessSetting::FrequencyBand::Bg);
    } else if (wirelessBand == WIFI_BAND_5GHZ) {
        wirelessSetting->setBand(NetworkManager::WirelessSetting::FrequencyBand::A);
    } else {
        qWarning()<<"[KyWirelessConnectOperation] the band type undefined"<<wirelessBand;
        wirelessSetting->setBand(NetworkManager::WirelessSetting::FrequencyBand::Automatic);
    }

    NetworkManager::WirelessSecuritySetting::Ptr wirelessSecuritySetting
        = apConnectSettingPtr->setting(NetworkManager::Setting::WirelessSecurity).dynamicCast<NetworkManager::WirelessSecuritySetting>();
    if (apPassword.isEmpty()) {
        wirelessSecuritySetting->setInitialized(false);
    } else {
        wirelessSecuritySetting->setInitialized(true);
        wirelessSecuritySetting->setKeyMgmt(NetworkManager::WirelessSecuritySetting::WpaPsk);
        wirelessSecuritySetting->setPsk(apPassword);
    }
    apConnectPtr->update(apConnectSettingPtr->toMap());

    QStringList blackList = getBlackListHostName(apConnectPtr->path());
    NMVariantMapMap newMap = apConnectSettingPtr->toMap();
    if (newMap.contains(KEY_802_11_WIRELESS)) {
        newMap[KEY_802_11_WIRELESS].insert(KEY_BLACKLIST_HOSTNAME, blackList);
        if (wirelessBand == WIFI_BAND_2_4GHZ) {
            newMap[KEY_802_11_WIRELESS].remove("channel");
        }
    }
    apConnectPtr->update(newMap);
    usleep(100*1000);
}

void KyWirelessConnectOperation::activeWirelessAp(const QString apUuid, const QString apName,
                                                  const QString apPassword, const QString apDevice,
                                                  const QString wirelessBand)

{
    qDebug() << "[activeWirelessAp]" << apUuid << apName << apPassword << apDevice << wirelessBand;
    //1、检查连接是否存在
    NetworkManager::Connection::Ptr connectPtr = m_networkResourceInstance->getConnect(apUuid);
    if (nullptr == connectPtr) {
        NetworkManager::Device::Ptr devicePtr = m_networkResourceInstance->findDeviceInterface(apDevice);
        if (devicePtr.isNull()) {
            QString errorMsg ="Create hotspot faild. " + apDevice + " is not existed";
            qWarning()<< errorMsg;
            Q_EMIT addAndActivateConnectionError(errorMsg);
            return;
        }

        QString deviceIdentifier = devicePtr->uni();

        NetworkManager::ConnectionSettings::Ptr apConnectSettingPtr =
                                createWirelessApSetting(apName, apPassword, apDevice, wirelessBand);
        QString specificObject = "";
        QDBusPendingCallWatcher * watcher;
        watcher = new QDBusPendingCallWatcher{NetworkManager::addAndActivateConnection(apConnectSettingPtr->toMap(), deviceIdentifier, specificObject), this};
        connect(watcher, &QDBusPendingCallWatcher::finished, [&] (QDBusPendingCallWatcher * watcher) {
            if (watcher->isError() || !watcher->isValid()) {
                QString errorMsg = "Create hotspot faild. " + watcher->error().message();
                qWarning() << errorMsg;
                Q_EMIT addAndActivateConnectionError(errorMsg);
            }
             watcher->deleteLater();
        });
    } else {
        updateWirelessApSetting(connectPtr, apName, apPassword, apDevice, wirelessBand);
        QTimer::singleShot(500, this, [=](){
            activateApConnectionByUuid(apUuid, apDevice);
        });
    }

    return;
}

void KyWirelessConnectOperation::deactiveWirelessAp(const QString apName, const QString apUuid)
{
    deactivateConnection(apName, apUuid);
    return;
}

//private
NetworkManager::WirelessNetwork::Ptr KyWirelessConnectOperation::checkWifiNetExist(QString ssid, QString devName)
{
    for (auto const & net : m_networkResourceInstance->m_wifiNets) {
        auto dev = m_networkResourceInstance->findDeviceUni(net->device());
        if (dev == nullptr) {
            continue;
        }
        if (dev->type() != NetworkManager::Device::Wifi || dev->interfaceName() != devName) {
            continue;
        }

        NetworkManager::AccessPoint::Ptr accessPointPtr = net->referenceAccessPoint();
        QByteArray rawSsid = accessPointPtr->rawSsid();
        QString wifiSsid = getSsidFromByteArray(rawSsid);

        if (ssid == wifiSsid) {
            return net;
        }
    }
    return nullptr;
}

KyKeyMgmt KyWirelessConnectOperation::getConnectKeyMgmt(const QString &uuid)
{
    NetworkManager::Connection::Ptr connectPtr =
            NetworkManager::findConnectionByUuid(uuid);
    if (connectPtr.isNull()) {
        return KyKeyMgmt::Unknown;
    }

    NetworkManager::WirelessSecuritySetting::Ptr security_sett
            = connectPtr->settings()->setting(NetworkManager::Setting::WirelessSecurity).dynamicCast<NetworkManager::WirelessSecuritySetting>();

    if(security_sett.isNull()) {
        return KyKeyMgmt::Unknown;
    }
    return (KyKeyMgmt)security_sett->keyMgmt();
}

void KyWirelessConnectOperation::updateWirelessSecu(NetworkManager::ConnectionSettings::Ptr connSettingPtr, const KyWirelessConnectSetting &connSettingInfo, bool bPwdChange)
{
    qDebug() << "updateWirelessSecu " << connSettingInfo.m_type;
    NetworkManager::WirelessSecuritySetting::Ptr security_sett
            = connSettingPtr->setting(NetworkManager::Setting::WirelessSecurity).dynamicCast<NetworkManager::WirelessSecuritySetting>();

    KyKeyMgmt type = connSettingInfo.m_type;
    qDebug() << "set keyMgmt " << type;
    if (type == WpaNone) {
        security_sett->setInitialized(false);
        return;
    } else {
        security_sett->setInitialized(true);
    }
    security_sett->setKeyMgmt((NetworkManager::WirelessSecuritySetting::KeyMgmt)type);
    if (bPwdChange) {
        security_sett->setPsk(connSettingInfo.m_psk);
        NetworkManager::Setting::SecretFlags flag = NetworkManager::Setting::None;
        security_sett->setPskFlags(flag);
    }
    return;
}

void KyWirelessConnectOperation::setWirelessSecuWpaXEap(NetworkManager::ConnectionSettings::Ptr connSettingPtr)
{
    NetworkManager::WirelessSecuritySetting::Ptr security_sett
            = connSettingPtr->setting(NetworkManager::Setting::WirelessSecurity).dynamicCast<NetworkManager::WirelessSecuritySetting>();

    security_sett->setInitialized(true);
    security_sett->setKeyMgmt(NetworkManager::WirelessSecuritySetting::KeyMgmt::WpaEap);
    return;
}

void KyWirelessConnectOperation::setIpv4AndIpv6Setting(NetworkManager::ConnectionSettings::Ptr connSetting, const KyConnectSetting &connSettingInfo)
{
    NetworkManager::Ipv4Setting::Ptr ipv4Setting = connSetting->setting(NetworkManager::Setting::Ipv4).dynamicCast<NetworkManager::Ipv4Setting>();
    ipv4SettingSet(ipv4Setting, connSettingInfo);

    NetworkManager::Ipv6Setting::Ptr ipv6Setting = connSetting->setting(NetworkManager::Setting::Ipv6).dynamicCast<NetworkManager::Ipv6Setting>();
    ipv6SettingSet(ipv6Setting, connSettingInfo);
}

void KyWirelessConnectOperation::activateApConnectionByUuid(const QString apUuid, const QString apDevice)
{
    QString connectPath = "";
    QString deviceIdentifier = "";
    QString connectName = "";
    QString specificObject = "/";

    qDebug()<<"it will activate hotspot connect"<<apUuid;

    NetworkManager::Connection::Ptr  connectPtr = m_networkResourceInstance->getConnect(apUuid);
    if (connectPtr.isNull())
    {
        QString errorMessage = tr("Create hotspot faild.UUID is empty, its name") + apUuid;
        qWarning() << errorMessage;
        Q_EMIT activateConnectionError(errorMessage);
        return;
    }
    connectPath = connectPtr->path();
    connectName = connectPtr->name();

    auto dev = m_networkResourceInstance->findDeviceInterface(apDevice);
    if (!dev.isNull()) {
        deviceIdentifier = dev->uni();
    }

    if (deviceIdentifier.isEmpty()) {
        QString errorMessage = tr("Create hotspot faild.Device Identifier is empty, its name") + apDevice;
        qWarning() << errorMessage;
        Q_EMIT activateConnectionError(errorMessage);
        return ;
    }

    QDBusPendingCallWatcher * watcher;
    watcher = new QDBusPendingCallWatcher{NetworkManager::activateConnection(connectPath, deviceIdentifier, specificObject), this};
    connect(watcher, &QDBusPendingCallWatcher::finished, [this, connectName, apDevice] (QDBusPendingCallWatcher * watcher) {
        if (watcher->isError() || !watcher->isValid()) {
            QString errorMessage = tr("Create hotspot faild. ") + watcher->error().message();
            qWarning()<<errorMessage;
            Q_EMIT this->activateConnectionError(errorMessage);
         } else {
            qWarning()<<"active wired connect complete.";
         }

         watcher->deleteLater();
    });

    return ;
}

bool KyWirelessConnectOperation::getEnterpiseEapMethod(const QString &uuid, KyEapMethodType &type)
{
    NetworkManager::Connection::Ptr connectPtr =
            NetworkManager::findConnectionByUuid(uuid);
    if (connectPtr.isNull()) {
        qWarning() << "getEnterpiseEapMethod faild.Can't find uuid = " << uuid;
        return false;
    }

    KyKeyMgmt keyMgmt = getConnectKeyMgmt(uuid);
    if (keyMgmt != WpaEap) {
        qWarning() << "getEnterpiseEapMethod but not WpaEap.it's " << keyMgmt;
        return false;
    }

    NetworkManager::ConnectionSettings::Ptr connectionSettings = connectPtr->settings();

    NetworkManager::Security8021xSetting::Ptr wifi_8021x_sett
            = connectionSettings->setting(NetworkManager::Setting::Security8021x).dynamicCast<NetworkManager::Security8021xSetting>();

    QList<NetworkManager::Security8021xSetting::EapMethod> list = wifi_8021x_sett->eapMethods();

    if (list.contains(NetworkManager::Security8021xSetting::EapMethod::EapMethodTls)) {
        type = TLS;
    } else if (list.contains(NetworkManager::Security8021xSetting::EapMethod::EapMethodPeap)) {
        type = PEAP;
    } else if (list.contains(NetworkManager::Security8021xSetting::EapMethod::EapMethodTtls)) {
        type = TTLS;
    } else if (list.contains(NetworkManager::Security8021xSetting::EapMethod::EapMethodLeap)) {
        type = LEAP;
    } else if (list.contains(NetworkManager::Security8021xSetting::EapMethod::EapMethodPwd)) {
        type = PWD;
    } else if (list.contains(NetworkManager::Security8021xSetting::EapMethod::EapMethodFast)) {
        type = FAST;
    }

    return true;
}
