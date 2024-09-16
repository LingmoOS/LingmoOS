// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "networkhandler.h"
#include "networkconfig.h"
#include "accountmanager.h"

#include <NetworkManagerQt/Manager>
#include <NetworkManagerQt/ActiveConnection>
#include <NetworkManagerQt/WirelessDevice>
#include <NetworkManagerQt/WirelessSetting>
#include <NetworkManagerQt/WirelessSecuritySetting>
#include <NetworkManagerQt/WiredSetting>
#include <NetworkManagerQt/ActiveConnection>

using namespace accountnetwork::systemservice;

NetworkHandler::NetworkHandler(AccountManager *accountManager, NetworkConfig *conf, QObject *parent)
    : QObject(parent)
    , m_accountManager(accountManager)
    , m_networkConfig(conf)
{
    initDevices();
    initConnection();
}

void NetworkHandler::disconnectNetwork()
{
    NetworkManager::Device::List devices = NetworkManager::networkInterfaces();
    for (NetworkManager::Device::Ptr device : devices) {
        if (device->type() != NetworkManager::Device::Type::Ethernet
                && device->type() != NetworkManager::Device::Type::Wifi)
            continue;

        NetworkManager::ActiveConnection::Ptr activeConnection = device->activeConnection();
        if (activeConnection) {
            QDBusPendingReply<> reply = NetworkManager::deactivateConnection(activeConnection->path());
            reply.waitForFinished();
        }
    }
}

NetworkHandler::~NetworkHandler() = default;

void NetworkHandler::initDevices()
{
    NetworkManager::Device::List devices = NetworkManager::networkInterfaces();
    for (NetworkManager::Device::Ptr device : devices) {
        if (device->type() != NetworkManager::Device::Type::Wifi
                && device->type() != NetworkManager::Device::Type::Ethernet)
            continue;

        connect(device.data(), &NetworkManager::Device::activeConnectionChanged, device.data(), [ this, device ] {
            deviceActiveHandler(device);
        });
        deviceActiveHandler(device);
    }
}

void NetworkHandler::initConnection()
{
    connect(NetworkManager::notifier(), &NetworkManager::Notifier::deviceAdded, this, &NetworkHandler::onDeviceAdded);
}

QString NetworkHandler::getSaveId(const QSharedPointer<NetworkManager::ActiveConnection> &activeConnection, bool isWireless)
{
    // 这里只保存需要加密的网络
    if (activeConnection.isNull())
        return QString();

    NetworkManager::ConnectionSettings::Ptr connectionSettings = activeConnection->connection()->settings();
    if (isWireless) {
        NetworkManager::WirelessSetting::Ptr wsSetting = connectionSettings->setting(NetworkManager::Setting::SettingType::Wireless)
                .dynamicCast<NetworkManager::WirelessSetting>();
        if (wsSetting) {
            // 获取当前无线网络的ssid作为无线网络的ID，保存到配置文件中
            return wsSetting->ssid();
        }
        return QString();
    }

    // 如果是有线网络，则直接保存id
    return connectionSettings->id();
}

QSharedPointer<NetworkManager::Device> NetworkHandler::getDeviceByInterfaceName(const QString &interfaceName) const
{
    NetworkManager::Device::List devices = NetworkManager::networkInterfaces();
    auto it = std::find_if(devices.begin(), devices.end(), [ interfaceName ](NetworkManager::Device::Ptr device) {
         return interfaceName == device->interfaceName();
    });

    if (it == devices.end())
        return nullptr;

    return (*it);
}

void NetworkHandler::deviceActiveHandler(const QSharedPointer<NetworkManager::Device> &device)
{
    NetworkManager::ActiveConnection::Ptr activeConnection = device->activeConnection();
    if (activeConnection.isNull())
        return;

    qDebug() << "active connection changed:" << activeConnection->id();
    connect(activeConnection.data(), &NetworkManager::ActiveConnection::stateChanged, activeConnection.data(), [ device, this, activeConnection ](NetworkManager::ActiveConnection::State state) {
        if (state == NetworkManager::ActiveConnection::State::Activated) {
            QString id = getSaveId(activeConnection, (device->type() == NetworkManager::Device::Type::Wifi));
            qDebug() << activeConnection->id() << "activated:" << id << "device:" << device->interfaceName()
                     << "account name:" << m_accountManager->account();
            // 连接成功的情况下，记录当前连接的网络的信息
            if (!id.isEmpty() && !m_accountManager->account().isEmpty()) {
                m_networkConfig->saveNetwork(m_accountManager->account(), id, device->interfaceName());
            }
        }
    });
}

void NetworkHandler::onDeviceAdded(const QString &uni)
{
    NetworkManager::Device::Ptr device = NetworkManager::findNetworkInterface(uni);
    if (device.isNull())
        return;

    if (device->type() != NetworkManager::Device::Type::Wifi
            && device->type() != NetworkManager::Device::Type::Ethernet)
        return;

    connect(device.data(), &NetworkManager::Device::activeConnectionChanged, device.data(), [ this, device ] {
        deviceActiveHandler(device);
    });

    deviceActiveHandler(device);
}
