// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "sessionipconfilct.h"
#include "settingconfig.h"

#include <QDebug>

#include <NetworkManagerQt/Ipv4Setting>

using namespace network::sessionservice;

SessionIPConflict::SessionIPConflict(QObject *parent)
    : IPConflictHandler(parent)
{
}

SessionIPConflict::~SessionIPConflict()
{

}

bool SessionIPConflict::canReconnect(const QSharedPointer<NetworkManager::ActiveConnection> &activeConnection)
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
    // 当密码选项为为当前用户保存密码的时候，如果当前服务为session服务，则会自动重连
    return secretSetting.testFlag(NetworkManager::Setting::AgentOwned);
}

void SessionIPConflict::onIPConflicted(const QString &ip, const QString &localMac, const QString &remoteMac)
{
    bool needReconnect = false;
    m_ipStore.doConfilcted(ip, localMac, remoteMac, needReconnect);
    if (needReconnect) {
        NetworkManager::Device::Ptr device = m_ipStore.getDevicePathByMac(localMac);
        requestReConnect(device);
    }
}

void SessionIPConflict::onIPConflictReslove(const QString &ip, const QString &localMac, const QString &remoteMac)
{
    m_ipStore.doConflictReslove(ip, localMac, remoteMac);
}
