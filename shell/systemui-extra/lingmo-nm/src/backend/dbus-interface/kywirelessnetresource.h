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
#ifndef KYWIRELESSNETRESOURCE_H
#define KYWIRELESSNETRESOURCE_H

#include <QObject>
#include "kywirelessnetitem.h"
#include "lingmonetworkresourcemanager.h"
#include "kyenterpricesettinginfo.h"
#include "lingmoconnectresource.h"
#include "kywirelessconnectoperation.h"
#include "lingmonetworkdeviceresource.h"


//class KyWirelessNetItem;

class KyWirelessNetResource : public QObject
{
    Q_OBJECT
public:
    explicit KyWirelessNetResource(QObject *parent = nullptr);
    ~KyWirelessNetResource();

    //ui层调用接口
    bool getWifiNetwork(const QString &devIfaceName, const QString &ssid, KyWirelessNetItem &wirelessNetResource);
    bool getAllDeviceWifiNetwork(QMap<QString, QList<KyWirelessNetItem>> &map);
    bool getDeviceWifiNetwork(QString devIfaceName, QList<KyWirelessNetItem> &wirelessNetResource);

    bool getEnterPriseInfoTls(QString &uuid, KyEapMethodTlsInfo &info);
    bool getEnterPriseInfoPeap(QString &uuid, KyEapMethodPeapInfo &info);
    bool getEnterPriseInfoTtls(QString &uuid, KyEapMethodTtlsInfo &info);
    bool getEnterPriseInfoLeap(QString &uuid, KyEapMethodLeapInfo &info);
    bool getEnterPriseInfoPwd(QString &uuid, KyEapMethodPwdInfo &info);
    bool getEnterPriseInfoFast(QString &uuid, KyEapMethodFastInfo &info);

    void getWirelessActiveConnection(NetworkManager::ActiveConnection::State state, QMap<QString, QStringList> &map);
    bool getActiveWirelessNetItem(QString deviceName, KyWirelessNetItem &wirelessNetItem);

    QString getActiveConnectSsidByDevice(QString deviceName);
    void getSsidByUuid(const QString uuid, QString &ssid);
    void getDeviceByUuid(const QString uuid, QString &deviceName);


private:
    void kyWirelessNetItemListInit();
    QString getDeviceIFace(NetworkManager::WirelessNetwork::Ptr net);
    QString getDeviceIFace(NetworkManager::ActiveConnection::Ptr actConn, QString &wirelessNetResourcessid);

public Q_SLOTS:
    void onWifiNetworkAdded(QString, QString);
    void onWifiNetworkRemoved(QString, QString);
    void onWifiNetworkPropertyChange(QString interface, QString ssid, int signal, QString bssid, QString sec);
    void onWifiNetworkSecuChange(NetworkManager::AccessPoint *accessPointPtr);
    void onWifiNetworkDeviceDisappear();

    void onConnectionAdd(QString uuid);
    void onConnectionUpdate(QString uuid);
    void onConnectionRemove(QString);

    void onDeviceAdd(QString deviceName, NetworkManager::Device::Type deviceType);
    void onDeviceRemove(QString deviceName);
    void onDeviceNameUpdate(QString oldName, QString newName);

Q_SIGNALS:
    void signalStrengthChange(QString, QString, int);
    void bssidChange(QString, QString, QString);
    void secuTypeChange(QString, QString, QString);
    void connectionRemove(QString, QString, QString);
    void connectionAdd(QString, QString);
    void connectionUpdate(QString, QString);
    void wifiNetworkUpdate();
    void wifiNetworkAdd(QString, KyWirelessNetItem&);
    void wifiNetworkRemove(QString, QString);

private:
    KyNetworkResourceManager *m_networkResourceInstance = nullptr;
    KyWirelessConnectOperation  *m_operation = nullptr;
    KyNetworkDeviceResourse  *m_networkDevice = nullptr;
    QMap<QString, QList<KyWirelessNetItem> >      m_WifiNetworkList;

};

#endif // KYWIRELESSNETRESOURCE_H
