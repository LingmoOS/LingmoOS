// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "networkactivator.h"
#include "aes.h"
#include "settingconfig.h"
#include "accountnetworkconfig.h"

#include <QDBusConnection>
#include <QDBusInterface>

#include <NetworkManagerQt/WirelessDevice>
#include <NetworkManagerQt/WiredDevice>
#include <NetworkManagerQt/Manager>
#include <NetworkManagerQt/WirelessNetwork>
#include <NetworkManagerQt/WirelessSetting>
#include <NetworkManagerQt/Security8021xSetting>
#include <NetworkManagerQt/WirelessSecuritySetting>
#include <NetworkManagerQt/Settings>

using namespace accountnetwork::sessionservice;

#define SYS_NETWORK_SERVICE "com.deepin.system.Network"
#define SYS_NETWORK_PATH "/com/deepin/system/Network"
#define SYS_NETWORK_INTERFACE "com.deepin.system.Network"

NetworkActivator::NetworkActivator(AccountNetworkConfig *config, QObject *parent)
    : QObject(parent)
    , m_config(config)
{
}

NetworkActivator::~NetworkActivator() = default;

void NetworkActivator::activeNetwork(const QMap<QString, QString> &network, const QVariantMap &certify)
{
    qDebug() << "active network" << network;
    if (network.isEmpty())
        return;

    auto findDeviceByInterface = [](const QString &deviceInterface)->NetworkManager::Device::Ptr {
        NetworkManager::Device::List devices = NetworkManager::networkInterfaces();
        auto itDevice = std::find_if(devices.begin(), devices.end(), [ deviceInterface ](NetworkManager::Device::Ptr device) {
             return device->interfaceName() == deviceInterface;
        });
        if (itDevice == devices.end())
            return nullptr;

        NetworkManager::Device::Ptr device = *itDevice;
        return device;
    };

    for (auto it = network.begin(); it != network.end(); it++) {
        // 遍历每一个设备，连接对应的网络
        NetworkManager::Device::Ptr device = findDeviceByInterface(it.key());
        if (device.isNull())
            continue;

        activeNetwork(device, it.value(), certify);
    }
}

void NetworkActivator::activeNetwork(const QSharedPointer<NetworkManager::Device> &device, const QString &id, const QVariantMap &certify)
{
    // 判断当前设备是否为禁用状态，如果是禁用状态，则先开启
    if (!deviceEnabled(device))
        return;

    // 先断开之前的连接
    NetworkManager::ActiveConnection::Ptr activeConnection = device->activeConnection();
    if (activeConnection)
        NetworkManager::deactivateConnection(activeConnection->path());

    switch (device->type()) {
    case NetworkManager::Device::Type::Wifi: {
        // 激活无线网络
        NetworkManager::WirelessDevice::Ptr wirelessDevice = device.staticCast<NetworkManager::WirelessDevice>();
        // 连接无线网络
        activeWirelessNetwork(wirelessDevice, wirelessDevice->findNetwork(id), certify);
        break;
    }
    case NetworkManager::Device::Type::Ethernet: {
        // 激活有线网络
        NetworkManager::WiredDevice::Ptr wiredDevice = device.staticCast<NetworkManager::WiredDevice>();
        activeWiredNetwork(wiredDevice, id, certify);
        break;
    }
    default:
        break;
    }
}

void NetworkActivator::activeWirelessNetwork(const QSharedPointer<NetworkManager::WirelessDevice> &wirelessDevice, const QSharedPointer<NetworkManager::WirelessNetwork> network, const QVariantMap &certify)
{
    if (!network)
        return;

    NetworkManager::Connection::Ptr connection = getConnection(wirelessDevice, network->ssid());
    if (connection.isNull())
        return;

    NetworkManager::WirelessSecuritySetting::Ptr wsSetting = connection->settings()->setting(NetworkManager::Setting::SettingType::WirelessSecurity).dynamicCast<NetworkManager::WirelessSecuritySetting>();
    NetworkManager::Security8021xSetting::Ptr sxSetting = connection->settings()->setting(NetworkManager::Setting::SettingType::Security8021x).dynamicCast<NetworkManager::Security8021xSetting>();
    // 如果不包含认证信息，则从本地读取认证信息
    QVariantMap authenInfo = certify;
    if (authenInfo.isEmpty()) {
        QPair<QString, QString> authInfo = m_config->certityInfo(wirelessDevice->interfaceName(), network->ssid());
        if (!authInfo.first.isEmpty() && ! authInfo.second.isEmpty()) {
            // 如果身份信息和密码信息不为空
            authenInfo["identity"] = authInfo.first;
            authenInfo["password"] = authInfo.second;
        }
    }
    // 这里只处理WPA/WPA2企业版的网络
    if (wsSetting && sxSetting && wsSetting->keyMgmt() == NetworkManager::WirelessSecuritySetting::KeyMgmt::WpaEap
            && authenInfo.contains("identity") && authenInfo.contains("password")) {
        // 如果包含认证信息，则更新本地保存的密码
        QString identity = authenInfo.value("identity").toString();
        // 保存账户名
        sxSetting->setIdentity(identity);
        // 保存密码方式（仅为当前用户保存）
        sxSetting->setPasswordFlags(NetworkManager::Setting::AgentOwned);
        // 保存密码，这里的密码是加密的，需要先解密然后再使用
        sxSetting->setPassword(decryptPassword(authenInfo.value("password").toString()));
        sxSetting->setInitialized(true);
        // 更新本地配置
        QDBusPendingReply<> reply = connection->update(connection->settings()->toMap());
        reply.waitForFinished();
    }
    NetworkManager::activateConnection(connection->path(), wirelessDevice->uni(), QString("/"));
}

void NetworkActivator::activeWiredNetwork(const QSharedPointer<NetworkManager::WiredDevice> &wiredDevice, const QString &id, const QVariantMap &certify)
{
    NetworkManager::Connection::List connections = wiredDevice->availableConnections();
    auto itConnection = std::find_if(connections.begin(), connections.end(), [id](NetworkManager::Connection::Ptr connection) {
         return connection->settings()->id() == id;
    });
    // 如果未找到，则不进行连接的操作
    if (itConnection == connections.end())
        return;

    QVariantMap authenInfo = certify;
    NetworkManager::Connection::Ptr connection = *itConnection;
    bool needAuthen = false;
    NetworkManager::Security8021xSetting::Ptr sxSetting = connection->settings()->setting(NetworkManager::Setting::SettingType::Security8021x).dynamicCast<NetworkManager::Security8021xSetting>();
    if (sxSetting && !sxSetting->toMap().isEmpty()) {
        if (authenInfo.isEmpty()) {
            QPair<QString, QString> authInfo = m_config->certityInfo(wiredDevice->interfaceName(), id);
            if (!authInfo.first.isEmpty() && ! authInfo.second.isEmpty()) {
                // 如果身份信息和密码信息不为空
                authenInfo["identity"] = authInfo.first;
                authenInfo["password"] = authInfo.second;
            }
        }
        const QList<NetworkManager::Security8021xSetting::EapMethod> &eapMethods = sxSetting->eapMethods();
        NetworkManager::Security8021xSetting::EapMethod currentEapMethod = eapMethods.isEmpty() ? NetworkManager::Security8021xSetting::EapMethodTls : eapMethods.first();
        // 如果当前的认证方式是受保护的eap，并且认证中包含用户名和密码
        needAuthen = currentEapMethod == NetworkManager::Security8021xSetting::EapMethod::EapMethodPeap
                && authenInfo.contains("identity") && authenInfo.contains("password");
    }
    if (needAuthen) {
        // 保存密码加密方式为EPA
        sxSetting->setEapMethods(QList<NetworkManager::Security8021xSetting::EapMethod>() << NetworkManager::Security8021xSetting::EapMethodPeap);
        // 如果包含认证信息，则更新本地保存的密码
        QString identity = authenInfo.value("identity").toString();
        // 保存账户名
        sxSetting->setIdentity(identity);
        // 保存密码方式（仅为当前用户保存）
        sxSetting->setPasswordFlags(NetworkManager::Setting::AgentOwned);
        // 保存密码，这里的密码是加密的，需要先解密然后再使用
        sxSetting->setPassword(decryptPassword(authenInfo.value("password").toString()));
        sxSetting->setInitialized(true);
        // 更新本地配置
        QDBusPendingReply<> reply = connection->update(connection->settings()->toMap());
        reply.waitForFinished();
    }

    NetworkManager::activateConnection(connection->path(), wiredDevice->uni(), QString("/"));
}

QString NetworkActivator::decryptPassword(const QString &password) const
{
    return network::service::PasswordHelper::aesDecryption(password, "uniontechnetwork", "ICBCCreditSuisse");
}

bool NetworkActivator::deviceEnabled(const QSharedPointer<NetworkManager::Device> &device) const
{
    QDBusInterface dbusInter(SYS_NETWORK_SERVICE, SYS_NETWORK_PATH, SYS_NETWORK_INTERFACE, QDBusConnection::systemBus());
    QDBusPendingCall reply = dbusInter.asyncCall("IsDeviceEnabled", device->uni());
    reply.waitForFinished();
    QDBusPendingReply<bool> replyResult = reply.reply();
    return replyResult.argumentAt(0).toBool();
}

QSharedPointer<NetworkManager::Connection> NetworkActivator::getConnection(const QSharedPointer<NetworkManager::WirelessDevice> &device, const QString &ssid) const
{
    NetworkManager::Connection::List connectionList = device->availableConnections();

    auto itConnection = std::find_if(connectionList.begin(), connectionList.end(), [ ssid ](const NetworkManager::Connection::Ptr &connection) {
        // 查找对应的连接
        NetworkManager::WirelessSetting::Ptr wSetting = connection->settings()->setting(NetworkManager::Setting::SettingType::Wireless).dynamicCast<NetworkManager::WirelessSetting>();
        if (!wSetting)
            return false;

        return wSetting->ssid() == ssid;
    });

    if (itConnection != connectionList.end())
        return *itConnection;

    NetworkManager::Connection::List allConnections = NetworkManager::listConnections();
    itConnection = std::find_if(allConnections.begin(), allConnections.end(), [ ssid, device ](const NetworkManager::Connection::Ptr &connection) {
        NetworkManager::WirelessSetting::Ptr wSetting = connection->settings()->setting(NetworkManager::Setting::SettingType::Wireless).dynamicCast<NetworkManager::WirelessSetting>();
        if (!wSetting)
            return false;

        QString settingMacaddress = wSetting->macAddress().toHex().toUpper();
        QString mac = device->permanentHardwareAddress().remove(":");
        // 如果配置中的Mac地址不为空并且和当前网卡的Mac地址不等，则认为当前网络不是当前网卡的网络
        if (!settingMacaddress.isEmpty() && settingMacaddress != mac)
            return false;

        return wSetting->ssid() == ssid;
    });

    if (itConnection != allConnections.end())
        return *itConnection;

    return QSharedPointer<NetworkManager::Connection>();
}
