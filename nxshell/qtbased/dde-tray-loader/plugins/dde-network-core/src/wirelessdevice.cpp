// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "wirelessdevice.h"
#include "netutils.h"
#include "netinterface.h"

#include <utility>

using namespace dde::network;

bool WirelessDevice::isConnected() const
{
    QList<AccessPoints *> aps = deviceRealize()->accessPointItems();
    for (AccessPoints *ap : aps) {
        if (ap->status() == ConnectionStatus::Activated)
            return true;
    }

    return false;
}

DeviceType WirelessDevice::deviceType() const
{
    return DeviceType::Wireless;
}

QList<AccessPoints *> WirelessDevice::accessPointItems() const
{
    /* 理论上，如果网卡是禁用状态，这里应该直接返回空列表
     * 但是由于在禁用网卡或者打开网卡的时候，后台的反应比较慢，控制中心无法实时获取，所以这里暂时让其先返回所有的网络列表，
     * 等后台反应慢的问题改好后，再把注释打开
     * if (!isEnabled())
        return QList<UAccessPoints *>();*/

    return deviceRealize()->accessPointItems();
}

void WirelessDevice::scanNetwork()
{
    deviceRealize()->scanNetwork();
}

void WirelessDevice::connectNetwork(const AccessPoints *item)
{
    deviceRealize()->connectNetwork(item);
}

AccessPoints *WirelessDevice::activeAccessPoints() const
{
    return deviceRealize()->activeAccessPoints();
}

bool WirelessDevice::hotspotEnabled() const
{
    return deviceRealize()->hotspotEnabled();
}

void WirelessDevice::disconnectNetwork()
{
    return deviceRealize()->disconnectNetwork();
}

void WirelessDevice::connectNetwork(const QString &ssid)
{
    AccessPoints *apConnection = findAccessPoint(ssid);
    if (apConnection)
        connectNetwork(apConnection);
}

QList<WirelessConnection *> WirelessDevice::items() const
{
    return deviceRealize()->wirelessItems();
}

WirelessDevice::WirelessDevice(NetworkDeviceRealize *networkInter, QObject *parent)
    : NetworkDeviceBase(networkInter, parent)
{
    connect(networkInter, &NetworkDeviceRealize::networkAdded, this, &WirelessDevice::networkAdded);
    connect(networkInter, &NetworkDeviceRealize::networkRemoved, this, &WirelessDevice::networkRemoved);
    connect(networkInter, &NetworkDeviceRealize::availableChanged, this, &WirelessDevice::availableChanged);
    connect(networkInter, &NetworkDeviceRealize::connectionFailed, this, &WirelessDevice::connectionFailed);
    connect(networkInter, &NetworkDeviceRealize::connectionSuccess, this, &WirelessDevice::connectionSuccess);
    connect(networkInter, &NetworkDeviceRealize::hotspotEnableChanged, this, &WirelessDevice::hotspotEnableChanged);
    connect(networkInter, &NetworkDeviceRealize::accessPointInfoChanged, this, &WirelessDevice::accessPointInfoChanged);
    connect(networkInter, &NetworkDeviceRealize::wirelessConnectionAdded, this, &WirelessDevice::wirelessConnectionAdded);
    connect(networkInter, &NetworkDeviceRealize::wirelessConnectionRemoved, this, &WirelessDevice::wirelessConnectionRemoved);
    connect(networkInter, &NetworkDeviceRealize::wirelessConnectionPropertyChanged, this, &WirelessDevice::wirelessConnectionPropertyChanged);
}

WirelessDevice::~WirelessDevice() = default;

AccessPoints *WirelessDevice::findAccessPoint(const QString &ssid)
{
    QList<AccessPoints *> accessPoints = deviceRealize()->accessPointItems();
    for (AccessPoints *accessPoint : accessPoints) {
        if (accessPoint->ssid() == ssid)
            return accessPoint;
    }

    return Q_NULLPTR;
}

/**
 * 无线热点类
 *
 */
AccessPoints::AccessPoints(AccessPointProxy *proxy, QObject *parent)
    : QObject(parent)
    , m_proxy(proxy)
{
    connect(proxy, &AccessPointProxy::strengthChanged, this, &AccessPoints::strengthChanged);
    connect(proxy, &AccessPointProxy::connectionStatusChanged, this, &AccessPoints::connectionStatusChanged);
    connect(proxy, &AccessPointProxy::securedChanged, this, &AccessPoints::securedChanged);
}

AccessPoints::~AccessPoints()
{
}

QString AccessPoints::ssid() const
{
    return m_proxy->ssid();
}

int AccessPoints::strength() const
{
    return m_proxy->strength();
}

bool AccessPoints::secured() const
{
    return m_proxy->secured();
}

bool AccessPoints::securedInEap() const
{
    return m_proxy->securedInEap();
}

int AccessPoints::frequency() const
{
    return m_proxy->frequency();
}

QString AccessPoints::path() const
{
    return m_proxy->path();
}

QString AccessPoints::devicePath() const
{
    return m_proxy->devicePath();
}

bool AccessPoints::connected() const
{
    return m_proxy->connected();
}

ConnectionStatus AccessPoints::status() const
{
    return m_proxy->status();
}

bool AccessPoints::hidden() const
{
    return m_proxy->hidden();
}

AccessPoints::WlanType AccessPoints::type() const
{
    return (m_proxy->isWlan6() ? WlanType::wlan6 : WlanType::wlan);
}

/**
 * @brief 无线连接信息
 */
AccessPoints *WirelessConnection::accessPoints() const
{
    return m_accessPoints;
}

bool WirelessConnection::connected()
{
    return m_accessPoints != nullptr && m_accessPoints->connected();
}

WirelessConnection::WirelessConnection()
    : ControllItems()
    , m_accessPoints(Q_NULLPTR)
{
}

WirelessConnection::~WirelessConnection() = default;

WirelessConnection *WirelessConnection::createConnection(AccessPoints *ap)
{
    auto *wirelessConnection = new WirelessConnection;
    QJsonObject json;
    json.insert("Ssid", ap->ssid());
    wirelessConnection->setConnection(json);
    wirelessConnection->m_accessPoints = ap;
    return wirelessConnection;
}
