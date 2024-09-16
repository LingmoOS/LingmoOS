// SPDX-FileCopyrightText: 2018 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "accesspointproxynm.h"
#include "netutils.h"

#include <NetworkManagerQt/WirelessSetting>

using namespace dde::network;

AccessPointProxyNM::AccessPointProxyNM(NetworkManager::WirelessDevice::Ptr device, NetworkManager::WirelessNetwork::Ptr network, QObject *parent)
    : AccessPointProxy(parent)
    , m_device(device)
    , m_network(network)
    , m_status(ConnectionStatus::Unknown)
    , m_isHidden(false)
    , m_strength(0)
    , m_secured(false)
{
    initState();
    initConnection();
    updateInfo();
    updateHiddenInfo();
}

AccessPointProxyNM::~AccessPointProxyNM()
{
}

void AccessPointProxyNM::updateStatus(ConnectionStatus status)
{
    if (m_status == status)
        return;

    m_status = status;
    Q_EMIT connectionStatusChanged(status);
}

void AccessPointProxyNM::updateNetwork(NetworkManager::WirelessNetwork::Ptr network)
{
    m_network = network;
}

bool AccessPointProxyNM::contains(const QString &uni) const
{
    if (path() == uni)
        return true;

    for (NetworkManager::AccessPoint::Ptr ap : m_network->accessPoints()) {
        if (ap->uni() == uni)
            return true;
    }

    return false;
}

QString AccessPointProxyNM::ssid() const
{
    return m_network->ssid();
}

int AccessPointProxyNM::strength() const
{
    return m_strength;
}

bool AccessPointProxyNM::secured() const
{
    return m_secured;
}

bool AccessPointProxyNM::securedInEap() const
{
    return false;
}

int AccessPointProxyNM::frequency() const
{
    return static_cast<int>(m_network->referenceAccessPoint()->frequency());
}

QString AccessPointProxyNM::path() const
{
    return m_network->referenceAccessPoint()->uni();
}

QString AccessPointProxyNM::devicePath() const
{
    return m_device->uni();
}

bool AccessPointProxyNM::connected() const
{
    return m_status == ConnectionStatus::Activated;
}

ConnectionStatus AccessPointProxyNM::status() const
{
    return m_status;
}

bool AccessPointProxyNM::hidden() const
{
    return m_isHidden;
}

bool AccessPointProxyNM::isWlan6() const
{
#if 0 // not support
    NetworkManager::AccessPoint::Capabilities capability = m_network->referenceAccessPoint()->capabilities();
    if (capability.testFlag(NetworkManager::AccessPoint::Capability::He))
        return true;
#endif

    return false;
}

void AccessPointProxyNM::initState()
{
    // 获取当前网络的连接状态
    NetworkManager::ActiveConnection::Ptr activeConnection = m_device->activeConnection();
    if (activeConnection.isNull())
        return;

    NetworkManager::WirelessSetting::Ptr wirelessSetting = activeConnection->connection()->settings()->setting(NetworkManager::Setting::SettingType::Wireless).dynamicCast<NetworkManager::WirelessSetting>();
    if (wirelessSetting.isNull())
        return;

    if (wirelessSetting->ssid() != m_network->ssid())
        return;

    updateStatus(convertStateFromNetworkManager(activeConnection->state()));
}

void AccessPointProxyNM::initConnection()
{
    connect(m_network.data(), &NetworkManager::WirelessNetwork::referenceAccessPointChanged, this, [ this ] {
        qCDebug(DNC) << "referenceAccessPoint changed:ssid:" << ssid() << "path" << m_network->referenceAccessPoint()->uni();
        // 先更新网络信息
        onUpdateNetwork();
        // 更新信号和槽的连接信息
        updateConnection();
    });
    updateConnection();
}

void AccessPointProxyNM::updateInfo()
{
    m_strength = m_network->signalStrength();
    NetworkManager::AccessPoint::Ptr ap = m_network->referenceAccessPoint();
    m_secured = (ap->capabilities() == NetworkManager::AccessPoint::Capability::Privacy || ap->wpaFlags() != 0 || ap->rsnFlags() != 0);
}

void AccessPointProxyNM::updateConnection()
{
    // 重新连接加密信息变化的信号，如果之前连接过，就先断开之前的连接
    for(auto &&conn: m_connectionList){
        disconnect(conn);
    }
    m_connectionList.clear();

    m_connectionList << connect(m_network->referenceAccessPoint().data(), &NetworkManager::AccessPoint::capabilitiesChanged, this, &AccessPointProxyNM::onUpdateNetwork, Qt::UniqueConnection);
    m_connectionList << connect(m_network->referenceAccessPoint().data(), &NetworkManager::AccessPoint::wpaFlagsChanged, this, &AccessPointProxyNM::onUpdateNetwork, Qt::UniqueConnection);
    m_connectionList << connect(m_network->referenceAccessPoint().data(), &NetworkManager::AccessPoint::rsnFlagsChanged, this, &AccessPointProxyNM::onUpdateNetwork, Qt::UniqueConnection);
    m_connectionList << connect(m_network->referenceAccessPoint().data(), &NetworkManager::AccessPoint::signalStrengthChanged, this, &AccessPointProxyNM::onUpdateNetwork, Qt::UniqueConnection);
}

void AccessPointProxyNM::updateHiddenInfo()
{
    NetworkManager::Connection::List connections = m_device->availableConnections();
    NetworkManager::Connection::List::iterator itConnection = std::find_if(connections.begin(), connections.end(), [ this ](NetworkManager::Connection::Ptr connection) {
        NetworkManager::WirelessSetting::Ptr wirelessSetting = connection->settings()->setting(NetworkManager::Setting::SettingType::Wireless).dynamicCast<NetworkManager::WirelessSetting>();
        if (wirelessSetting.isNull())
            return false;

        return wirelessSetting->ssid() == m_network->ssid();
    });
    // 如果没有找到连接，就是非隐藏网络
    if (itConnection == connections.end())
        return;
    // 如果找到了连接，就获取Hidden的内容
    NetworkManager::WirelessSetting::Ptr setting = (*itConnection)->settings()->setting(NetworkManager::Setting::SettingType::Wireless).dynamicCast<NetworkManager::WirelessSetting>();
    if (setting.isNull())
        return;

    m_isHidden = setting->hidden();
    qCDebug(DNC) << "update accesspoint hidden info, ssid:" << m_isHidden << ", hidden:" << m_isHidden;
}

void AccessPointProxyNM::onUpdateNetwork()
{
    int strength = m_strength;
    bool secured = m_secured;
    updateInfo();
    if (strength != m_strength)
        emit strengthChanged(m_strength);

    if (secured != m_secured)
        emit securedChanged(m_secured);
}
