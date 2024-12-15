// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "systemipconfilct.h"
#include "settingconfig.h"

#include <QDebug>

#include <NetworkManagerQt/Ipv4Setting>

using namespace network::systemservice;

SystemIPConflict::SystemIPConflict(QObject *parent)
    : IPConflictHandler(parent)
{
}

SystemIPConflict::~SystemIPConflict()
{
}

bool SystemIPConflict::ipConflicted(const QString &devicePath) const
{
    return m_ipConfilctData.isConflicted(devicePath);
}

bool SystemIPConflict::canReconnect(const QSharedPointer<NetworkManager::ActiveConnection> &activeConnection)
{
    if (activeConnection.isNull())
        return false;

    NetworkManager::Ipv4Setting::Ptr setting =
            activeConnection->connection()->settings()->setting(NetworkManager::Setting::Ipv4).dynamicCast<NetworkManager::Ipv4Setting>();

    if (setting) {
        //获取IP地址的方式，这里只处理自动获取IP地址的方式，如果当前是手动获取IP地址，则无需再次连接
        NetworkManager::Ipv4Setting::ConfigMethod method = setting->method();
        if (method != NetworkManager::Ipv4Setting::ConfigMethod::Automatic)
            return false;
    }

    NetworkManager::Setting::SecretFlags secretSetting = secretFlags(activeConnection);
    // 当密码选项为为所用用户保存密码或者总是询问的时候，系统服务会让其自动重连
    return (secretSetting.testFlag(NetworkManager::Setting::None)
            || secretSetting.testFlag(NetworkManager::Setting::NotSaved));
}

void SystemIPConflict::onIPConflicted(const QString &ip, const QString &localMac, const QString &remoteMac)
{
    QSharedPointer<NetworkManager::Device> device = m_ipConfilctData.getDevicePathByMac(localMac);
    if (device.isNull())
        return;

    bool needReconnect = false;
    if (m_ipConfilctData.doConfilcted(ip, localMac, remoteMac, needReconnect))
        emit ipConflictChanged(device->uni(), ip, true);

    if (needReconnect)
        requestReConnect(device);
}

void SystemIPConflict::onIPConflictReslove(const QString &ip, const QString &localMac, const QString &remoteMac)
{
    QSharedPointer<NetworkManager::Device> device = m_ipConfilctData.getDevicePathByMac(localMac);
    if (device && m_ipConfilctData.doConflictReslove(ip, localMac, remoteMac))
        emit ipConflictChanged(device->uni(), ip, false);
}

bool SystemIPConflict::ipConflicted(const QString &devicePath, const QString &ip) const
{
    return m_ipConfilctData.isConflicted(devicePath, ip);
}

void SystemIPConflict::ipv4Changed(const NetworkManager::Device::Ptr &device)
{
    NetworkManager::IpAddresses ipAddresses = device->ipV4Config().addresses();
    QStringList ipV4s;
    for (const NetworkManager::IpAddress &ip : ipAddresses) {
        ipV4s << ip.ip().toString();
    }
    QStringList resloveIps;
    m_ipConfilctData.updateIpv4(device, ipV4s, resloveIps);
    for (const QString &resloveIp : resloveIps) {
        emit ipConflictChanged(device->uni(), resloveIp, false);
    }
}
