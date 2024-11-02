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
#ifndef LINGMOCONNECTOPERATION_H
#define LINGMOCONNECTOPERATION_H

#include "lingmonetworkresourcemanager.h"
#include "lingmoconnectsetting.h"

class KyConnectOperation : public QObject
{
    Q_OBJECT
public:
    explicit KyConnectOperation(QObject *parent = nullptr);
    ~KyConnectOperation();

public:
    void  createConnect(KyConnectSetting &connectSettingsInfo);
    void  updateConnect(NetworkManager::ConnectionSettings::Ptr connectionSettings, const KyConnectSetting &connectSettingsInfo);
    void  deleteConnect(const QString &connectUuid);
    void  activateConnection(const QString connectUuid, const QString deviceName);
    void  deactivateConnection(const QString activeConnectName, const QString &activeConnectUuid);

public:
    void connectSettingSet(
                        NetworkManager::ConnectionSettings::Ptr connectionSettings,
                        const KyConnectSetting &connectSettingsInfo);
    void ipv4SettingSet(NetworkManager::Ipv4Setting::Ptr &ipv4Setting,
                         const KyConnectSetting &connectSettingsInfo);
    void ipv6SettingSet(NetworkManager::Ipv6Setting::Ptr &ipv6Setting,
                         const KyConnectSetting &connectSettingsInfo);
    void setAutoConnect(NetworkManager::ConnectionSettings::Ptr &connectSetting,
                         bool bAutoConnect);
    inline void errorProcess(QString errorMessage);

Q_SIGNALS:
    void createConnectionError(QString errorMessage);
    void updateConnectionError(QString errorMessage);
    void deleteConnectionError(QString errorMessage);
    void activateConnectionError(QString errorMessage);
    void deactivateConnectionError(QString errorMessage);

protected:
    KyNetworkResourceManager *m_networkResourceInstance = nullptr;
};

#endif // LINGMOCONNECTOPERATION_H
