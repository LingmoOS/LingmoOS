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
#ifndef LINGMOACTIVECONNECTRESOURCE_H
#define LINGMOACTIVECONNECTRESOURCE_H

#include <QString>
#include "lingmonetworkresourcemanager.h"
#include "lingmoconnectitem.h"
#include "lingmonetworkdeviceresource.h"
#include "lingmovpnconnectitem.h"
#include "lingmobluetoothconnectitem.h"
#include "lingmoapconnectitem.h"

class KyActiveConnectResourse : public QObject
{
    Q_OBJECT
public:
    explicit KyActiveConnectResourse(QObject *parent = nullptr);
    ~KyActiveConnectResourse();

public:
    KyConnectItem *getActiveConnectionByUuid(QString connectUuid);
    KyConnectItem *getActiveConnectionByUuid(QString connectUuid, QString deviceName);
    void getActiveConnectionList(QString deviceName,
                                 NetworkManager::ConnectionSettings::ConnectionType connectionType,
                                 QList<KyConnectItem *> &connectItemList);
    void getActiveConnectIpInfo(const QString &uuid,
                            QString &ipv4Address,
                            QString &ipv6Address);
    void getActiveConnectDnsInfo(const QString &uuid,
                             QList<QHostAddress> &ipv4Dns,
                             QList<QHostAddress> &ipv6Dns);

    //void getWiredActivateConnect(QList<KyWiredConnectItem *> &wiredActiveConnectItemList);
    void getVpnActivateConnect(QList<KyVpnConnectItem *> &vpnActiveConnectItemList);
    void getBtActivateConnect(QList<KyBluetoothConnectItem *> &btActiveConnectItemList);

    void getApActivateConnect(QList<KyApConnectItem *> &apConnectItemList);

    QString getDeviceOfActivateConnect(QString conUuid);

    bool connectionIsVirtual(QString uuid);
    bool wiredConnectIsActived();
    bool checkWirelessStatus(NetworkManager::ActiveConnection::State state);
    QString getAcitveConnectionPathByUuid(QString uuid);

    int getActivateWifiSignal(QString devName = "");

private:
    void getActiveConnectIp(NetworkManager::ActiveConnection::Ptr activeConnectPtr,
                            QString &ipv4Address,
                            QString &ipv6Address);
    void getActiveConnectDns(NetworkManager::ActiveConnection::Ptr activeConnectPtr,
                             QList<QHostAddress> &ipv4Dns,
                             QList<QHostAddress> &ipv6Dns);

    KyConnectItem *getActiveConnectionItem(NetworkManager::ActiveConnection::Ptr activeConnectPtr);

   // KyWiredConnectItem *getWiredActiveConnectItem(NetworkManager::ActiveConnection::Ptr activeConnectPtr);
    KyVpnConnectItem *getVpnActiveConnectItem(NetworkManager::ActiveConnection::Ptr activeConnectPtr);
    KyBluetoothConnectItem *getBtActiveConnectItem(NetworkManager::ActiveConnection::Ptr activeConnectPtr);
    KyApConnectItem *getApActiveConnectItem(NetworkManager::ActiveConnection::Ptr activeConnectPtr);

Q_SIGNALS:
    void activeConnectAdd(QString activeConnectUuid);
    void updateActiveConnect(QString activeConnectUuid);
    void activeConnectRemove(QString activeConnectUuid);
    void stateChangeReason(QString uuid,
                                    NetworkManager::ActiveConnection::State state,
                                    NetworkManager::ActiveConnection::Reason reason);
    void vpnConnectChangeReason(QString uuid,
                                NetworkManager::VpnConnection::State state,
                                NetworkManager::VpnConnection::StateChangeReason reason);

private:
    KyNetworkResourceManager *m_networkResourceInstance = nullptr;
    KyNetworkDeviceResourse *m_networkdevice = nullptr;
};
#endif // LINGMOACTIVECONNECTRESOURCE_H
