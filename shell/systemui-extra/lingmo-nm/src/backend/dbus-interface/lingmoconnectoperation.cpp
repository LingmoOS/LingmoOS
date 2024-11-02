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

#include "lingmoconnectoperation.h"

#include <NetworkManagerQt/AdslDevice>
#include <NetworkManagerQt/WiredDevice>
#include <NetworkManagerQt/Ipv4Setting>
#include <NetworkManagerQt/Ipv6Setting>
#include <NetworkManagerQt/WiredSetting>

KyConnectOperation::KyConnectOperation(QObject *parent) : QObject(parent)
{
    m_networkResourceInstance = KyNetworkResourceManager::getInstance();
}

KyConnectOperation::~KyConnectOperation()
{
    m_networkResourceInstance = nullptr;
}

void KyConnectOperation::ipv4SettingSet(
        NetworkManager::Ipv4Setting::Ptr &ipv4Setting,
        const KyConnectSetting &connectSettingsInfo)
{
    ipv4Setting->setInitialized(true);
    ipv4Setting->setDns(connectSettingsInfo.m_ipv4Dns);

    if (CONFIG_IP_DHCP == connectSettingsInfo.m_ipv4ConfigIpType) {
        ipv4Setting->setMethod(NetworkManager::Ipv4Setting::Automatic);
        return;
    } else {
        ipv4Setting->setMethod(NetworkManager::Ipv4Setting::Manual);
    }
    ipv4Setting->setAddresses(connectSettingsInfo.m_ipv4Address);

    return;
}
void KyConnectOperation::ipv6SettingSet(
        NetworkManager::Ipv6Setting::Ptr &ipv6Setting,
        const KyConnectSetting &connectSettingsInfo)
{
    ipv6Setting->setInitialized(true);
    ipv6Setting->setDns(connectSettingsInfo.m_ipv6Dns);

    if (CONFIG_IP_DHCP == connectSettingsInfo.m_ipv6ConfigIpType) {
        ipv6Setting->setMethod(NetworkManager::Ipv6Setting::Automatic);
        ipv6Setting->setPrivacy(NetworkManager::Ipv6Setting::Disabled);
        return;
    }

    ipv6Setting->setMethod(NetworkManager::Ipv6Setting::Manual);
    ipv6Setting->setAddresses(connectSettingsInfo.m_ipv6Address);

    return ;
}

void KyConnectOperation::connectSettingSet(
        NetworkManager::ConnectionSettings::Ptr connectionSettings,
        const KyConnectSetting &connectSettingsInfo)
{
    connectionSettings->setId(connectSettingsInfo.m_connectName);
    connectionSettings->setUuid(NetworkManager::ConnectionSettings::createNewUuid());
    connectionSettings->setAutoconnect(true);
    connectionSettings->setAutoconnectPriority(0);
    if (!connectSettingsInfo.m_ifaceName.isEmpty()) {
        connectionSettings->setInterfaceName(connectSettingsInfo.m_ifaceName);
    }
    return;
}

void KyConnectOperation::setAutoConnect(NetworkManager::ConnectionSettings::Ptr &connectSetting,
                     bool bAutoConnect)
{
    if (connectSetting.isNull()) {
        return;
    }

    connectSetting->setAutoconnect(bAutoConnect);

    return;
}

void KyConnectOperation::createConnect(KyConnectSetting &connectSettingsInfo)
{
    qDebug()<<"create connect need to do";
    return;
}

void KyConnectOperation::updateConnect(NetworkManager::ConnectionSettings::Ptr connectionSettings, const KyConnectSetting &connectSettingsInfo)
{
    qDebug()<<"update connect"<<connectionSettings->uuid();

    NetworkManager::Ipv4Setting::Ptr ipv4Setting = connectionSettings->setting(NetworkManager::Setting::Ipv4).dynamicCast<NetworkManager::Ipv4Setting>();
    ipv4SettingSet(ipv4Setting, connectSettingsInfo);

    NetworkManager::Ipv6Setting::Ptr ipv6Setting = connectionSettings->setting(NetworkManager::Setting::Ipv6).dynamicCast<NetworkManager::Ipv6Setting>();
    ipv6SettingSet(ipv6Setting, connectSettingsInfo);

//    NetworkManager::WiredSetting::Ptr wiredSetting = connectionSettings->setting(NetworkManager::Setting::Wired).dynamicCast<NetworkManager::WiredSetting>();
//    wiredSetting->setInitialized(true);

    return ;
}

void KyConnectOperation::deleteConnect(const QString &connectUuid)
{
    qDebug()<<"delete connect uuid " << connectUuid;

    NetworkManager::Connection::Ptr connectPtr =
            NetworkManager::findConnectionByUuid(connectUuid);
    if (nullptr == connectPtr) {
        QString errorMessage = tr("it can not find connection") + connectUuid;
        qWarning()<<errorMessage;
        Q_EMIT deleteConnectionError(errorMessage);
        return;
    }

    connectPtr->remove();

    return ;
}

void KyConnectOperation::activateConnection(const QString connectUuid, const QString deviceName)
{
    QString connectPath = "";
    QString deviceIdentifier = "";
    QString connectName = "";
    QString specificObject = "";
    NetworkManager::Connection::Ptr connectPtr = nullptr;

    qDebug()<<"it will activate connect"<<connectUuid;
    connectPtr = NetworkManager::findConnectionByUuid(connectUuid);
    if (nullptr == connectPtr) {
        QString errorMessage = "the connect uuid " + connectUuid + "is not exsit";
        qWarning()<<errorMessage;
        Q_EMIT activateConnectionError(errorMessage);
        return;
    }
#if 0
    if (NetworkManager::ConnectionSettings::Wired != connectPtr->settings()->connectionType()) {
        QString errorMessage = tr("the connect type is")
                                + connectPtr->settings()->connectionType()
                                + tr(", but it is not wired");
        qWarning()<<errorMessage;
        Q_EMIT activateConnectionError(errorMessage);
        return;
    }
#endif
    connectPath = connectPtr->path();
    connectName = connectPtr->name();
//    deviceName = connectPtr->settings()->interfaceName();

//    for (auto const & dev : m_networkResourceInstance->m_devices) {
//        for (auto const & dev_conn : dev->availableConnections()) {
//            if (dev_conn == connectPtr) {
//                deviceIdentifier = dev->uni();
//                deviceName = dev->interfaceName();
//                break;
//            }
//        }
//    }

    auto dev = m_networkResourceInstance->findDeviceInterface(deviceName);
    if (!dev.isNull()) {
        deviceIdentifier = dev->uni();
    }


    if (deviceIdentifier.isEmpty()) {
        QString errorMessage = tr("device Identifier is empty, its name") + deviceName;
        qWarning() << errorMessage;
        Q_EMIT activateConnectionError(errorMessage);
        return ;
    }

    qDebug() <<"active wired connect: path "<< connectPath
             << "device identify " << deviceIdentifier
             << "connect name " << connectName
             << "device name" << deviceName
             << "specific parameter"<< specificObject;

    QDBusPendingCallWatcher * watcher;
    watcher = new QDBusPendingCallWatcher{NetworkManager::activateConnection(connectPath, deviceIdentifier, specificObject), this};
    connect(watcher, &QDBusPendingCallWatcher::finished, [this, connectName, deviceName] (QDBusPendingCallWatcher * watcher) {
        if (watcher->isError() || !watcher->isValid()) {
            QString errorMessage = tr("activate connection failed: ") + watcher->error().message();
            qWarning()<<errorMessage;
            Q_EMIT this->activateConnectionError(errorMessage);
         } else {
            qWarning()<<"active wired connect complete.";
         }

         watcher->deleteLater();
    });

    return ;
}

void KyConnectOperation::deactivateConnection(const QString activeConnectName, const QString &activeConnectUuid)
{
    NetworkManager::ActiveConnection::Ptr activateConnectPtr = nullptr;

    qDebug()<<"deactivetate connect name"<<activeConnectName<<"uuid"<<activeConnectUuid;

    activateConnectPtr = m_networkResourceInstance->getActiveConnect(activeConnectUuid);
    if (nullptr == activateConnectPtr) {
        QString errorMessage = tr("it can not find the activate connect")
                            + activeConnectName + tr("uuid") + activeConnectUuid;
        qWarning()<<errorMessage;
        Q_EMIT deactivateConnectionError(errorMessage);
        return ;
    }

    qDebug() <<"dead active connection path:"<< activateConnectPtr->path();
    QDBusPendingReply<> reply = NetworkManager::deactivateConnection(activateConnectPtr->path());
    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(reply, this);
    connect(watcher, &QDBusPendingCallWatcher::finished, [this, activateConnectPtr] (QDBusPendingCallWatcher * watcher) {
        if (watcher->isError() || !watcher->isValid()) {
            QString errorMessage = tr("deactivation of connection")
                        + activateConnectPtr->connection()->name() + tr("failed:")
                        + watcher->error().message();

            qWarning()<<errorMessage;
            Q_EMIT this->deactivateConnectionError(errorMessage);
        } else {
            qWarning() << "deactive connect operation finished" << activateConnectPtr->connection()->name();
        }
         watcher->deleteLater();
    });

    return;
}
