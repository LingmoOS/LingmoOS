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
#ifndef LINGMOCONNECTRESOURCE_H
#define LINGMOCONNECTRESOURCE_H

#include <QString>
#include "lingmonetworkresourcemanager.h"
#include "lingmoconnectitem.h"
#include "lingmovpnconnectitem.h"
#include "lingmobluetoothconnectitem.h"
#include "lingmonetworkdeviceresource.h"
#include "lingmoconnectsetting.h"
#include "lingmoapconnectitem.h"

class KyConnectResourse : public QObject
{
    Q_OBJECT
public:
    explicit KyConnectResourse(QObject *parent = nullptr);
    ~KyConnectResourse();

public:
    KyConnectItem *getConnectionItemByUuid(QString connectUuid);
    KyConnectItem *getConnectionItemByUuidWithoutActivateChecking(QString connectUuid);
    KyConnectItem *getConnectionItemByUuid(QString connectUuid, QString deviceName);
    void getVpnAndVirtualConnections(QList<KyConnectItem *> &connectItemList);
    void getConnectionList(QString deviceName,
                           NetworkManager::ConnectionSettings::ConnectionType connectionType,
                           QList<KyConnectItem *> &connectItemList);
 //   void getWiredConnections(QList<KyWiredConnectItem *> &wiredConnectItemList);
    void getVpnConnections(QList<KyVpnConnectItem *> &vpnConnectItemList);
    void getBluetoothConnections(QList<KyBluetoothConnectItem *> &bluetoothConnectItemList);
    void getApConnections(QList<KyApConnectItem *> &apConnectItemList);
    KyApConnectItem *getApConnectionByUuid(QString connectUuid);
    QString getApConnectionPathByUuid(QString connectUuid);
    QString getApAcitveConnectionPathByUuid(QString connectUuid);

    void getConnectionSetting(QString connectUuid, KyConnectSetting &connectSetting);
    bool getInterfaceByUuid(QString &deviceName, const QString connUuid);
    void getConnectivity(NetworkManager::Connectivity &connectivity);

    bool isVirtualConncection(QString uuid);
    bool isWiredConnection(QString uuid);
    bool isWirelessConnection(QString uuid);
    bool isActivatedConnection(QString uuid);
    bool isApConnection(QString uuid);

private:
    bool isActiveDevice(QString conUuid, QString devName);
    KyConnectItem *getConnectionItem(NetworkManager::Connection::Ptr connectPtr, QString devName);
    void getConnectIp(NetworkManager::ConnectionSettings::Ptr settingPtr,
                      QString &ipv4Address,
                      QString &ipv6Address);
//    KyWiredConnectItem *getWiredConnectItem(NetworkManager::Connection::Ptr connectPtr);

    void getVpnConnectData(NetworkManager::ConnectionSettings::Ptr settingPtr,
                           KyVpnConnectItem *vpnItem);
    KyVpnConnectItem *getVpnConnectItem(NetworkManager::Connection::Ptr connectPtr);
    KyBluetoothConnectItem *getBluetoothConnectItem(NetworkManager::Connection::Ptr connectPtr);
    KyApConnectItem *getApConnectItem(NetworkManager::Connection::Ptr connectPtr);

    void getIpv4ConnectSetting(NetworkManager::Ipv4Setting::Ptr &ipv4Setting,
                            KyConnectSetting &connectSetting);
    void getIpv6ConnectSetting(NetworkManager::Ipv6Setting::Ptr &ipv6Setting,
                            KyConnectSetting &connectSetting);

Q_SIGNALS:
    void connectionAdd(QString uuid);
    void connectionUpdate(QString uuid);
    void connectionRemove(QString path);
    void connectivityChanged(NetworkManager::Connectivity connectivity);
    void connectivityCheckSpareUriChanged();

    void needShowDesktop(QString);

private:
    KyNetworkResourceManager *m_networkResourceInstance = nullptr;
};
#endif // LINGMOCONNECTRESOURCE_H
