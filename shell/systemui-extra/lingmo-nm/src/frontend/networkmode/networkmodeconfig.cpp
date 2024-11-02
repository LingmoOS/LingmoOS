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
#include "networkmodeconfig.h"
#include "firewalldialog.h"
#include <QDebug>

#define LOG_FLAG  "[NetworkMode]"

NetworkModeConfig *NetworkModeConfig::m_netModeInstance = nullptr;

NetworkModeConfig *NetworkModeConfig::getInstance()
{
    if (m_netModeInstance == NULL) {
        m_netModeInstance = new NetworkModeConfig();
    }
    return m_netModeInstance;
}


NetworkModeConfig::NetworkModeConfig(QObject *parent) : QObject(parent)
{
    m_dbusInterface = new QDBusInterface("com.ksc.defender",
                       "/firewall",
                       "com.ksc.defender.firewall",
                       QDBusConnection::systemBus());
    if(!m_dbusInterface->isValid()) {
        qWarning() << qPrintable(QDBusConnection::sessionBus().lastError().message());
    }
}

int NetworkModeConfig::getNetworkModeConfig(QString uuid)
{
    if (uuid.isEmpty()) {
           qWarning()<< /*LOG_FLAG <<*/ "uuid is empty, so can not get network mode config";
           return NO_CONFIG;
       }

    if(m_dbusInterface == nullptr || !m_dbusInterface->isValid()) {
        qWarning () << "com.ksc.defender dbus is invalid";
        return DBUS_INVAILD;
    }

    QDBusReply<int> reply = m_dbusInterface->call("get_networkModeConfig", uuid);
    if (reply.isValid()) {
        return reply.value();
    } else {
        qWarning() << "call get_networkModeConfig failed" << reply.error().message();
    }
    return NO_CONFIG;
}

void NetworkModeConfig::setNetworkModeConfig(QString uuid, QString cardName, QString ssid, int mode)
{
    if(m_dbusInterface == nullptr || !m_dbusInterface->isValid()) {
        qWarning () << "com.ksc.defender dbus is invalid";
        return;
    }

    QDBusReply<int> reply = m_dbusInterface->call("set_networkModeConfig", uuid, cardName, ssid, mode);
    if (reply.isValid()) {
        qDebug() << "set_networkModeConfig" << ssid << uuid << cardName << mode << ",result" << reply.value();
    } else {
        qWarning() << "call set_networkModeConfig" << reply.error().message();
    }
}

int NetworkModeConfig::breakNetworkConnect(QString uuid, QString cardName, QString ssid)
{
    if(m_dbusInterface == nullptr || !m_dbusInterface->isValid()) {
        qWarning () << "com.ksc.defender dbus is invalid";
        return -1;
    }

    QDBusReply<int> reply = m_dbusInterface->call("break_networkConnect", uuid, cardName, ssid);
    if (reply.isValid()) {
        qDebug() << "break_networkConnect" << ssid << uuid << cardName << ",result" << reply.value();
        return reply.value();
    } else {
        qWarning() << "call break_networkConnect failed" << reply.error().message();
        return -1;
    }
}

//安全中心-网络防火墙模式配置
NetworkMode::NetworkMode(QObject *parent)
    :QObject(parent)
{
    qRegisterMetaType<NetworkManager::Device::State>("NetworkManager::Device::State");
    qRegisterMetaType<NetworkManager::Device::StateChangeReason>("NetworkManager::Device::StateChangeReason");
    m_activatedConnectResource = new KyActiveConnectResourse(this);
    connect(m_activatedConnectResource, &KyActiveConnectResourse::stateChangeReason,
            this, &NetworkMode::onConnectionStateChanged);
}

void NetworkMode::initWiredNetworkMode()
{
    qDebug()<< LOG_FLAG << "initWiredNetworkMode";
    QStringList wiredDevList;
    KyNetworkDeviceResourse deviceResource;
    deviceResource.getNetworkDeviceList(NetworkManager::Device::Type::Ethernet, wiredDevList);
    if (wiredDevList.isEmpty()) {
        return;
    }

    for (auto devName : wiredDevList) {
        QList<KyConnectItem *> activedList;
        m_activatedConnectResource->getActiveConnectionList(devName,
                                                            NetworkManager::ConnectionSettings::Wired,
                                                            activedList);
        if (activedList.isEmpty()) {
            continue;
        }

        int configType = NetworkModeConfig::getInstance()->getNetworkModeConfig(activedList.at(0)->m_connectUuid);
        if (configType == DBUS_INVAILD) {
            return;
        } else if (configType == NO_CONFIG) {
            //已连接网络无配置 默认公有配置
            NetworkModeConfig::getInstance()->setNetworkModeConfig(activedList.at(0)->m_connectUuid, devName,
                                                                   activedList.at(0)->m_connectName, KSC_FIREWALL_PUBLIC);
        } else {
            NetworkModeConfig::getInstance()->setNetworkModeConfig(activedList.at(0)->m_connectUuid, devName,
                                                                   activedList.at(0)->m_connectName, configType);
        }
    }
}

void NetworkMode::initWirelessNetworkMode()
{
    qDebug()<< LOG_FLAG << "initWirelessNetworkMode";
    QStringList wirelessDevList;
    KyNetworkDeviceResourse deviceResource;
    deviceResource.getNetworkDeviceList(NetworkManager::Device::Type::Wifi, wirelessDevList);
    if (wirelessDevList.isEmpty()) {
        return;
    }

    for (auto devName : wirelessDevList) {
        KyWirelessNetItem wirelessNetItem;
        KyWirelessNetResource wirelessNetResource;
        bool ret = wirelessNetResource.getActiveWirelessNetItem(devName, wirelessNetItem);

        if (ret == true) {
            int configType = NetworkModeConfig::getInstance()->getNetworkModeConfig(wirelessNetItem.m_connectUuid);
            if (configType == DBUS_INVAILD) {
                return;
            } else if (configType == NO_CONFIG) {
                //已连接网络无配置 默认公有配置
                NetworkModeConfig::getInstance()->setNetworkModeConfig(wirelessNetItem.m_connectUuid, devName,
                                                                       wirelessNetItem.m_connName, KSC_FIREWALL_PUBLIC);
            } else {
                NetworkModeConfig::getInstance()->setNetworkModeConfig(wirelessNetItem.m_connectUuid, devName,
                                                                       wirelessNetItem.m_connName, configType);
            }
        }
    }
}

void NetworkMode::setFirstConnectNetworkMode(QString uuid, QString deviceName, QString ssid)
{
    NetworkModeConfig::getInstance()->setNetworkModeConfig(uuid, deviceName, ssid, KSC_FIREWALL_PUBLIC); //默认公有配置
    FirewallDialog *fireWallDialog = new FirewallDialog();
    fireWallDialog->setUuid(uuid);
    fireWallDialog->setWindowTitle(ssid);

    connect(fireWallDialog, &FirewallDialog::setPrivateNetMode, this, [=](){
        fireWallDialog->hide();
        NetworkModeConfig::getInstance()->setNetworkModeConfig(uuid, deviceName, ssid, KSC_FIREWALL_PRIVATE);
    });

    connect(fireWallDialog, &FirewallDialog::setPublicNetMode, this, [=](){
        fireWallDialog->hide();
        NetworkModeConfig::getInstance()->setNetworkModeConfig(uuid, deviceName, ssid, KSC_FIREWALL_PUBLIC);
    });

    connect(m_activatedConnectResource, &KyActiveConnectResourse::stateChangeReason, fireWallDialog, &FirewallDialog::closeMyself);

    fireWallDialog->show();
    fireWallDialog->centerToScreen();
}

void NetworkMode::onConnectionStateChanged(QString uuid,
                                           NetworkManager::ActiveConnection::State state,
                                           NetworkManager::ActiveConnection::Reason reason)
{
    if (state == NetworkManager::ActiveConnection::State::Activated) {
        QString deviceName = "";
        QString ssid = "";

        int configType = NetworkModeConfig::getInstance()->getNetworkModeConfig(uuid);
        KyConnectResourse connectResource;

        //有线网络连接
        if (connectResource.isWiredConnection(uuid)) {
            KyConnectItem *p_newItem = nullptr;
            p_newItem = m_activatedConnectResource->getActiveConnectionByUuid(uuid);
            if (nullptr == p_newItem) {
                //删除此网络
                qDebug()<< LOG_FLAG << "delete wired connect:" << uuid << ", call break_networkConnect";
                NetworkModeConfig::getInstance()->breakNetworkConnect(uuid, "", "");
                return;
            }

            deviceName = p_newItem->m_ifaceName;
            ssid = p_newItem->m_connectName;
            if (configType == NO_CONFIG) {
                //首次连接的网络
                setFirstConnectNetworkMode(uuid, deviceName, ssid);
            }  else if (configType == KSC_FIREWALL_PUBLIC) {
                NetworkModeConfig::getInstance()->setNetworkModeConfig(uuid, deviceName, ssid, KSC_FIREWALL_PUBLIC);
            } else if (configType == KSC_FIREWALL_PRIVATE) {
                NetworkModeConfig::getInstance()->setNetworkModeConfig(uuid, deviceName, ssid, KSC_FIREWALL_PRIVATE);
            }
        }
        //无线网络连接
        if (connectResource.isWirelessConnection(uuid)) {
            KyWirelessNetResource wirelessNetResource;
            wirelessNetResource.getSsidByUuid(uuid, ssid);
            wirelessNetResource.getDeviceByUuid(uuid, deviceName);
            if (ssid.isEmpty()) {
                //忘记此网络
                qDebug()<< LOG_FLAG << "forgrt wireless connect:" << uuid <<", call break_networkConnect";
                NetworkModeConfig::getInstance()->breakNetworkConnect(uuid, "", "");
            }

            if (configType == NO_CONFIG) {
                setFirstConnectNetworkMode(uuid, deviceName, ssid);
            }  else if (configType == KSC_FIREWALL_PUBLIC) {
                NetworkModeConfig::getInstance()->setNetworkModeConfig(uuid, deviceName, ssid, KSC_FIREWALL_PUBLIC);
            } else if (configType == KSC_FIREWALL_PRIVATE) {
                NetworkModeConfig::getInstance()->setNetworkModeConfig(uuid, deviceName, ssid, KSC_FIREWALL_PRIVATE);
            }
        }

    } else if (state == NetworkManager::ActiveConnection::State::Deactivated) {
        NetworkModeConfig::getInstance()->breakNetworkConnect(uuid, "", "");
    }
}
