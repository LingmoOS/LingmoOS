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
#ifndef LINGMONETORKDEVICERESOURCE_H
#define LINGMONETORKDEVICERESOURCE_H

#include <QString>
#include "lingmonetworkresourcemanager.h"
#include "lingmoconnectitem.h"
#include "lingmowiredconnectoperation.h"

class KyNetworkDeviceResourse : public QObject
{
    Q_OBJECT
public:
    explicit KyNetworkDeviceResourse(QObject *parent = nullptr);
    ~KyNetworkDeviceResourse();

Q_SIGNALS:
    void deviceAdd(QString deviceName, NetworkManager::Device::Type deviceType);
    void deviceRemove(QString deviceName);
    void deviceUpdate(QString deviceName);

    void deviceNameUpdate(QString oldName, QString newName);
    void stateChange(QString deviceName, int state);

    void carrierChanage(QString deviceName, bool pluged);
    void deviceBitRateChanage(QString deviceName, int bitRate);
    void deviceMacAddressChanage(QString deviceName, const QString &hwAddress);
    void deviceActiveChanage(QString deviceName, bool deivceActive);
    void deviceManagedChange(QString deviceName, bool managed);
    void stateChanged(NetworkManager::Device::State newstate, NetworkManager::Device::State oldstate, NetworkManager::Device::StateChangeReason reason);

public Q_SLOTS:
    void onDeviceAdd(QString deviceName, QString uni, NetworkManager::Device::Type deviceType);
    void onDeviceRemove(QString deviceName, QString uni);
    void onDeviceUpdate(QString interface, QString dbusPath);

public:
    void getNetworkDeviceList(NetworkManager::Device::Type deviceType, QStringList &networkDeviceList);
    void getHardwareInfo(QString ifaceName, QString &hardAddress, int &bandWith);
    void getDeviceActiveAPInfo(const QString devName, QString &strMac, uint &iHz, uint &iChan, QString &secuType);
    int getWirelessDeviceCapability(const QString deviceName);
    NetworkManager::Device::State getDeviceState(QString deviceName);
    bool getActiveConnectionInfo(const QString devName, int &signalStrength, QString &uni, QString &secuType);

    bool wiredDeviceIsCarriered(QString deviceName);
    bool wirelessDeviceIsExist(const QString devName);
    bool checkDeviceType(QString deviceName, NetworkManager::Device::Type deviceType);

    void setDeviceRefreshRate(QString deviceName, int ms);

    void setDeviceManaged(QString devName, bool managed);
    bool getDeviceManaged(QString devName);

    qulonglong getDeviceRxRefreshRate(QString deviceName);
    qulonglong getDeviceTxRefreshRate(QString deviceName);

    void getDeviceConnectivity(const QString &deviceName, NetworkManager::Connectivity &connectivity);

private:
    void initDeviceMap();
    uint kyFindChannel(uint freq);

private:
    KyWiredConnectOperation wiredOperation;
    KyNetworkResourceManager *m_networkResourceInstance = nullptr;
    QStringList m_activeConnectUuidList;
    QMap<QString, QString> m_deviceMap;
};
#endif // LINGMONETORKDEVICERESOURCE_H
