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

#include "lingmowiredconnectoperation.h"
#include "lingmoutil.h"

#include <NetworkManagerQt/AdslDevice>
#include <NetworkManagerQt/WiredDevice>
#include <NetworkManagerQt/Ipv4Setting>
#include <NetworkManagerQt/Ipv6Setting>
#include <NetworkManagerQt/WiredSetting>

KyWiredConnectOperation::KyWiredConnectOperation(QObject *parent) : KyConnectOperation(parent)
{
    connect(m_networkResourceInstance, &KyNetworkResourceManager::wiredEnabledChanged,
                                        this, &KyWiredConnectOperation::wiredEnabledChanged);
}

KyWiredConnectOperation::~KyWiredConnectOperation()
{
}

void KyWiredConnectOperation::setWiredEnabled(bool enabled)
{
    setWiredEnabledByGDbus(enabled);
}

bool KyWiredConnectOperation::getWiredEnabled()
{
    return getWiredEnabledByGDbus();
}

void KyWiredConnectOperation::createWiredConnect(KyConnectSetting &connectSettingsInfo)
{
    qDebug()<<"[KyWiredConnectOperation]" << "create connect ";
    connectSettingsInfo.dumpInfo();

    NetworkManager::ConnectionSettings::Ptr wiredConnectionSettings = NetworkManager::ConnectionSettings::Ptr(new NetworkManager::ConnectionSettings(NetworkManager::ConnectionSettings::Wired));
    connectSettingSet(wiredConnectionSettings, connectSettingsInfo);

    NetworkManager::Ipv4Setting::Ptr ipv4Setting = wiredConnectionSettings->setting(NetworkManager::Setting::Ipv4).dynamicCast<NetworkManager::Ipv4Setting>();
    ipv4SettingSet(ipv4Setting, connectSettingsInfo);

    NetworkManager::Ipv6Setting::Ptr ipv6Setting = wiredConnectionSettings->setting(NetworkManager::Setting::Ipv6).dynamicCast<NetworkManager::Ipv6Setting>();
    ipv6SettingSet(ipv6Setting, connectSettingsInfo);

    NetworkManager::WiredSetting::Ptr wiredSetting = wiredConnectionSettings->setting(NetworkManager::Setting::Wired).dynamicCast<NetworkManager::WiredSetting>();
    wiredSetting->setInitialized(true);

    QDBusPendingCallWatcher * watcher;
    watcher = new QDBusPendingCallWatcher{NetworkManager::addConnection(wiredConnectionSettings->toMap()), this};
    connect(watcher, &QDBusPendingCallWatcher::finished, [this](QDBusPendingCallWatcher * watcher) {
        if (watcher->isError() || !watcher->isValid()) {
            QString errorMessage = tr("create wired connection failed: ") + watcher->error().message();
            qWarning()<<errorMessage;
            Q_EMIT this->createConnectionError(errorMessage);
         } else {
            qDebug()<<"create wired connect complete";
         }
         watcher->deleteLater();
    });

    return;
}

void KyWiredConnectOperation::updateWiredConnect(const QString &connectUuid, const KyConnectSetting &connectSettingsInfo)
{
    qDebug()<<"update connect"<<connectUuid;
    NetworkManager::Connection::Ptr connectPtr =
            NetworkManager::findConnectionByUuid(connectUuid);
    if (nullptr == connectPtr) {
        QString errorMessage = tr("it can not find connection") + connectUuid;
        qWarning()<<errorMessage;
        Q_EMIT updateConnectionError(errorMessage);
        return;
    }

    NetworkManager::ConnectionSettings::Ptr connectSettingPtr = connectPtr->settings();
    updateConnect(connectSettingPtr, connectSettingsInfo);

    NMVariantMapMap mapmap1 = connectSettingPtr->toMap();
    if (mapmap1.contains("ipv4")) {
        QVariantMap map1 = mapmap1.value(QLatin1String("ipv4"));
        bool isAuto = false;
        if (map1.contains("method") && map1["method"] == "auto") {
            qDebug() << "[KyWiredConnectOperation] set ipv4 method auto, clear address-data && addresses && gateway";
            isAuto = true;
        }
        if (isAuto) {
            if (map1.contains("address-data")) {
                map1.remove("address-data");
            }
            if (map1.contains("addresses")) {
                map1.remove("addresses");
            }
            if (map1.contains("gateway")) {
                map1.remove("gateway");
            }
        }
        mapmap1["ipv4"] = map1;
    }

    if (mapmap1.contains("ipv6")) {
        QVariantMap map2 = mapmap1.value(QLatin1String("ipv6"));
        bool isAuto = false;
        if (map2.contains("method") && map2["method"] == "auto") {
            qDebug() << "[KyWiredConnectOperation] set ipv6 method auto, clear address-data && addresses && gateway";
            isAuto = true;
        }
        if (isAuto) {
            if (map2.contains("address-data")) {
                map2.remove("address-data");
            }
            if (map2.contains("addresses")) {
                map2.remove("addresses");
            }
            if (map2.contains("gateway")) {
                map2.remove("gateway");
            }
        }
        mapmap1["ipv6"] = map2;
    }

    connectPtr->update(mapmap1);

    return ;
}

void KyWiredConnectOperation::deleteWiredConnect(const QString &connectUuid)
{
    qDebug()<<"delete wired connect uuid " << connectUuid;

    deleteConnect(connectUuid);

    return ;
}

void KyWiredConnectOperation::activateWiredConnection(const QString connectUuid, const QString devName)
{
    activateConnection(connectUuid, devName);
    return ;
}

void KyWiredConnectOperation::deactivateWiredConnection(const QString activeConnectName, const QString &activeConnectUuid)
{
    qDebug()<<"deactivetate connect name"<<activeConnectName<<"uuid"<<activeConnectUuid;

    deactivateConnection(activeConnectName, activeConnectUuid);

    return;
}

void KyWiredConnectOperation::saveActiveConnection(QString &deviceName, QString &connectUuid)
{
    QSettings *p_settings = new QSettings(WIRED_NETWORK_STATE_CONF_FILE, QSettings::IniFormat);

    QString settingValue = p_settings->value(deviceName).toString();
    if (settingValue.isEmpty()) {
        p_settings->setValue(deviceName, connectUuid);
        p_settings->sync();
    }

    delete p_settings;
    p_settings = nullptr;

    return;
}

void KyWiredConnectOperation::getActiveConnection(QString &deviceName, QString &connectUuid)
{
    QSettings *p_settings = new QSettings(WIRED_NETWORK_STATE_CONF_FILE, QSettings::IniFormat);

    connectUuid = p_settings->value(deviceName).toString();
    p_settings->remove(deviceName);

    delete p_settings;
    p_settings = nullptr;

    return;
}

int KyWiredConnectOperation::closeWiredNetworkWithDevice(QString deviceName)
{
    NetworkManager::Device::Ptr wiredDevicePtr =
                            m_networkResourceInstance->findDeviceInterface(deviceName);

    if (wiredDevicePtr.isNull()) {
        qWarning()<<"[KyWiredConnectOperation]"<<"the network device" << deviceName <<"is not exist.";
        return -ENXIO;
    }

    if (NetworkManager::Device::Type::Ethernet != wiredDevicePtr->type()) {
        qWarning()<<"[KyWiredConnectOperation]"<<"the device type"
                  << wiredDevicePtr->type() <<"is not Ethernet.";
        return -EINVAL;
    }

    NetworkManager::ActiveConnection::Ptr activeConnectPtr = wiredDevicePtr->activeConnection();
    if (nullptr != activeConnectPtr) {
        QString activeConnectUuid = activeConnectPtr->uuid();
        if (!activeConnectUuid.isEmpty()) {
            qDebug()<<"[KyWiredConnectOperation]" <<"close wired network save connection uuid"
                   << activeConnectUuid <<"device name " << deviceName;
            saveActiveConnection(deviceName, activeConnectUuid);
        }
    }

    //wiredDevicePtr->setAutoconnect(false);
    wiredDevicePtr->disconnectInterface();

    return 0;
}

int KyWiredConnectOperation::openWiredNetworkWithDevice(QString deviceName)
{
    NetworkManager::Device::Ptr wiredDevicePtr =
                            m_networkResourceInstance->findDeviceInterface(deviceName);

    if (wiredDevicePtr.isNull() || !wiredDevicePtr->isValid()) {
        qWarning()<<"[KyWiredConnectOperation]"<<"the network device" << deviceName <<"is not exist.";
        return -ENXIO;
    }

    if (NetworkManager::Device::Type::Ethernet != wiredDevicePtr->type()) {
        qWarning()<<"[KyWiredConnectOperation]"<<"the device type"
                  << wiredDevicePtr->type() <<"is not Ethernet.";
        return -EINVAL;
    }

    NetworkManager::WiredDevice *p_wiredDevice =
        qobject_cast<NetworkManager::WiredDevice *>(wiredDevicePtr.data());

    if (p_wiredDevice->carrier()) {
        QString connectUuid;
        getActiveConnection(deviceName, connectUuid);
        if (!connectUuid.isEmpty()) {
            qDebug()<<"[KyWiredConnectOperation]" << "open wired network active connection"
                   << connectUuid <<"device name" << deviceName;
            activateConnection(connectUuid, deviceName);
        }
    }

    wiredDevicePtr->setAutoconnect(true);

    return 0;
}
