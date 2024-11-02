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
#ifndef NETWORKMODECONFIG_H
#define NETWORKMODECONFIG_H

#include <QObject>
#include <QDBusInterface>
#include <QDBusReply>
#include "lingmoactiveconnectresource.h"
#include "lingmoconnectresource.h"
#include "kywirelessnetresource.h"

enum NetworkModeType {
    DBUS_INVAILD = -2,
    NO_CONFIG = -1,
    KSC_FIREWALL_PUBLIC = 0,
    KSC_FIREWALL_PRIVATE
};

class NetworkModeConfig : public QObject
{
    Q_OBJECT
public:
    static NetworkModeConfig *getInstance();
    //安全中心-获取网络模式配置
    int getNetworkModeConfig(QString uuid);
    //安全中心-设置网络模式配置
    void setNetworkModeConfig(QString uuid, QString cardName, QString ssid, int mode);
    //安全中心-解除连接（用于防火墙处从正在使用的网络中删除）
    int breakNetworkConnect(QString uuid, QString cardName, QString ssid);

    static NetworkModeConfig *m_netModeInstance;

private:
    explicit NetworkModeConfig(QObject *parent = nullptr);
    QDBusInterface *m_dbusInterface = nullptr;
};


class NetworkMode : public QObject
{
    Q_OBJECT
public:
    NetworkMode(QObject *parent = 0);
    ~NetworkMode() = default;
    void initWiredNetworkMode();
    void initWirelessNetworkMode();

private:
    KyActiveConnectResourse *m_activatedConnectResource = nullptr;
    void setFirstConnectNetworkMode(QString uuid, QString deviceName, QString ssid);

private Q_SLOTS:
    void onConnectionStateChanged(QString uuid,
                                  NetworkManager::ActiveConnection::State state,
                                  NetworkManager::ActiveConnection::Reason reason);
};

#endif // NETWORKMODECONFIG_H
