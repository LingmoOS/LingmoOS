// SPDX-FileCopyrightText: 2011 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
#include "networkdetailnmrealize.h"
#include "devicemanagerrealize.h"
#include "ipmanager.h"

#include <NetworkManagerQt/WirelessDevice>
#include <NetworkManagerQt/WirelessSecuritySetting>
#include <NetworkManagerQt/Security8021xSetting>
#include <NetworkManagerQt/VpnSetting>
#include <NetworkManagerQt/Utils>

using namespace dde::network;

NetworkDetailNMRealize::NetworkDetailNMRealize(NetworkManager::Device::Ptr device, NetworkManager::ActiveConnection::Ptr activeConnection, QObject *parent)
    : NetworkDetailRealize(parent)
    , m_device(device)
    , m_activeConnection(activeConnection)
    , m_ipConfig(new IpManager(m_device, this))
{
    initProperties();
    initConnection();
}

NetworkDetailNMRealize::~NetworkDetailNMRealize()
{
}

void NetworkDetailNMRealize::initConnection()
{
    connect(m_activeConnection.get(), &NetworkManager::ActiveConnection::ipV4ConfigChanged, this, &NetworkDetailNMRealize::onUpdateInfo);
    connect(m_activeConnection.get(), &NetworkManager::ActiveConnection::ipV6ConfigChanged, this, &NetworkDetailNMRealize::onUpdateInfo);
    connect(m_ipConfig, &IpManager::ipChanged, this, [ this ] {
        onUpdateInfo();
        emit infoChanged();
    });
}

QString NetworkDetailNMRealize::name()
{
    return m_name;
}

QList<QPair<QString, QString> > NetworkDetailNMRealize::items()
{
    return m_items;
}

void NetworkDetailNMRealize::initProperties()
{
    m_name = m_activeConnection->connection()->name();
    bool isHotspot = false;
    NetworkManager::Connection::Ptr connection = m_activeConnection->connection();
    if (m_device->type() == NetworkManager::Device::Type::Wifi) {
        // 获取设备的活动连接网络
        NetworkManager::WirelessDevice::Ptr wirelessDevice = m_device.staticCast<NetworkManager::WirelessDevice>();

        QString ssid;
        int channel = 0;
        QString frequencyBand = tr("automatic");
        NetworkManager::AccessPoint::Ptr activeAccessPoint = wirelessDevice->activeAccessPoint();
        if (activeAccessPoint) {
            ssid = activeAccessPoint->ssid();
            switch (NetworkManager::findFrequencyBand(static_cast<int>(activeAccessPoint->frequency()))) {
            case NetworkManager::WirelessSetting::FrequencyBand::A:
                frequencyBand = "5 GHz";
                break;
            case NetworkManager::WirelessSetting::FrequencyBand::Bg:
                frequencyBand = "2.4 GHz";
                break;
            default:
                break;
            }
            channel = NetworkManager::findChannel(static_cast<int>(activeAccessPoint->frequency()));
        }

        isHotspot = (wirelessDevice->mode() == NetworkManager::WirelessDevice::OperationMode::ApMode);
        if (isHotspot) {
            m_name = tr("Hotspot");
            NetworkManager::AccessPoint::Ptr activeAp = wirelessDevice->activeAccessPoint();
            if (activeAp)
                appendInfo(tr("SSID"), ssid);
        } else {
            const QString protocol = NetworkManager::ConnectionSettings::typeAsString(connection->settings()->connectionType());
            if (!protocol.isEmpty())
                appendInfo(tr("Protocol"), protocol);

            // 安全类型
            appendInfo(tr("Security Type"), getSecurity(m_device));
            // 网络通道
            if (channel != 0)
                appendInfo(tr("Channel"), QString::number(channel));
        }
        appendInfo(tr("Band"), frequencyBand);
        if (isHotspot) {
            appendInfo(tr("Security Type"), getSecurity(m_device));
        }
    }
    // 接口名
    appendInfo(tr("Interface"), m_device->interfaceName());
    // MAC地址
    const QString mac = macAddress();
    if (!mac.isEmpty())
        appendInfo(tr("MAC"), mac);

    if (isHotspot)
        return;

    // 获取IPV4
    NetworkManager::IpConfig ipV4Config = m_activeConnection->ipV4Config();
    QList<NetworkManager::IpAddress> addresses;
    if (m_ipConfig) {
        addresses = m_ipConfig->ipAddresses();
    } else {
        addresses = ipV4Config.addresses();
    }
    for (NetworkManager::IpAddress address : addresses) {
        QString ipv4 = address.ip().toString();
        ipv4 = ipv4.remove("\"");
        appendInfo(tr("IPv4"), ipv4);
        QString netMask = prefixToNetMask(address.prefixLength());
        if (!netMask.isEmpty())
            appendInfo(tr("Netmask"), netMask);
    }
    QString gateWay = ipV4Config.gateway();
    if (!gateWay.isEmpty())
        appendInfo(tr("Gateway"), gateWay);

    QList<QHostAddress> ipV4NameServers = ipV4Config.nameservers();
    if (!ipV4NameServers.isEmpty()) {
        appendInfo(tr("Primary DNS"), ipV4NameServers.first().toString());
    }
    // 获取IPV6
    NetworkManager::IpConfig ipV6Config = m_activeConnection->ipV6Config();
    addresses = ipV6Config.addresses();
    for (NetworkManager::IpAddress address : addresses) {
        QString ip = address.ip().toString().remove("\"");
        appendInfo(tr("IPv6"), ip);
        appendInfo(tr("Prefix"), QString::number(address.prefixLength()));
    }
    gateWay = ipV6Config.gateway();
    if (!gateWay.isEmpty() && gateWay != "::")
        appendInfo(tr("Gateway"), gateWay);
    QList<QHostAddress> ipV6NameServers = ipV6Config.nameservers();
    if (!ipV6NameServers.isEmpty()) {
        appendInfo(tr("Primary DNS"), ipV6NameServers.first().toString());
    }

    appendInfo(tr("Speed"), getSpeedStr());
}

void NetworkDetailNMRealize::appendInfo(const QString &title, const QString &value)
{
    m_items << qMakePair(title, value);
}

QString NetworkDetailNMRealize::getSecurity(const NetworkManager::Device::Ptr &device) const
{
    NetworkManager::WirelessDevice::Ptr wirelessDevice = device.dynamicCast<NetworkManager::WirelessDevice>();
    if (wirelessDevice.isNull())
        return QString();

    NetworkManager::AccessPoint::Ptr activeAccessPoint = wirelessDevice->activeAccessPoint();
    NetworkManager::ActiveConnection::Ptr activeConnection = wirelessDevice->activeConnection();
    if (activeConnection.isNull() || activeAccessPoint.isNull())
        return QString();

    NetworkManager::ConnectionSettings::ConnectionType connectionType = activeConnection->connection()->settings()->connectionType();
    if (connectionType != NetworkManager::ConnectionSettings::ConnectionType::Wireless || activeAccessPoint.isNull())
        return QString();

    NetworkManager::WirelessSecuritySetting::KeyMgmt keyMemt;
    NetworkManager::WirelessSecuritySetting::Ptr securitySetting = activeConnection->connection()->settings()->setting(
                NetworkManager::Setting::SettingType::WirelessSecurity).dynamicCast<NetworkManager::WirelessSecuritySetting>();
    if (securitySetting.isNull()) {
        keyMemt = WirelessDeviceManagerRealize::getKeyMgmtByAp(activeAccessPoint);
    } else {
        keyMemt = securitySetting->keyMgmt();
    }
    QString security = QObject::tr("None");
    switch (keyMemt) {
    case NetworkManager::WirelessSecuritySetting::KeyMgmt::Wep:
        security = QObject::tr("WEP 40/128-bit Key");
        break;
    case NetworkManager::WirelessSecuritySetting::KeyMgmt::WpaPsk:
        security = QObject::tr("WPA/WPA2 Personal");
        break;
    case NetworkManager::WirelessSecuritySetting::KeyMgmt::SAE:
        security = QObject::tr("WPA3 Personal");
        break;
    case NetworkManager::WirelessSecuritySetting::KeyMgmt::WpaEap: {
        NetworkManager::Security8021xSetting::Ptr wirelessSecuritySetting = activeConnection->connection()->settings()->setting(NetworkManager::Setting::SettingType::Security8021x).dynamicCast<NetworkManager::Security8021xSetting>();
        if (!wirelessSecuritySetting.isNull()) {
            const QList<NetworkManager::Security8021xSetting::EapMethod> &eapMethods = wirelessSecuritySetting->eapMethods();
            NetworkManager::Security8021xSetting::EapMethod currentEapMethod = eapMethods.isEmpty() ? NetworkManager::Security8021xSetting::EapMethodTls : eapMethods.first();
            switch (currentEapMethod) {
            case NetworkManager::Security8021xSetting::EapMethodTls:
                security = "EAP" + QObject::tr("TLS");
                break;
            case NetworkManager::Security8021xSetting::EapMethodMd5:
                security = "EAP/" + QObject::tr("MD5");
                break;
            case NetworkManager::Security8021xSetting::EapMethodLeap:
                security = "EAP/" + QObject::tr("LEAP");
                break;
            case NetworkManager::Security8021xSetting::EapMethodFast:
                security = "EAP/" + QObject::tr("FAST");
                break;
            case NetworkManager::Security8021xSetting::EapMethodTtls:
                security = "EAP/" + QObject::tr("Tunneled TLS");
                break;
            case NetworkManager::Security8021xSetting::EapMethodPeap:
                security = "EAP/" + QObject::tr("Protected EAP");
                break;
            default:
                break;
            }
        }
        break;
    }
    default:
        break;
    }
    return security;
}

QString NetworkDetailNMRealize::macAddress() const
{
    switch (m_device->type()) {
    case NetworkManager::Device::Type::Ethernet: {
        NetworkManager::WiredDevice::Ptr wiredDevice = m_device.dynamicCast<NetworkManager::WiredDevice>();
        if (wiredDevice)
            return wiredDevice->hardwareAddress();
        break;
    }
    case NetworkManager::Device::Type::Wifi: {
        NetworkManager::WirelessDevice::Ptr wirelessDevice = m_device.dynamicCast<NetworkManager::WirelessDevice>();
        if (wirelessDevice)
            return wirelessDevice->hardwareAddress();
        break;
    }
    default:
        break;
    }

    return QString();
}

QString NetworkDetailNMRealize::prefixToNetMask(int prefixLength) const
{
    uint32_t mask = (0xFFFFFFFF << (32 - prefixLength)) & 0xFFFFFFFF;
    uint8_t m1 = mask >> 24;
    uint8_t m2 = (mask >> 16) & 0xFF;
    uint8_t m3 = (mask >> 8) & 0xFF;
    uint8_t m4 = mask & 0xFF;
    return QString("%1.%2.%3.%4").arg(m1).arg(m2).arg(m3).arg(m4);
}

QString NetworkDetailNMRealize::getSpeedStr() const
{
    if (m_device->type() == NetworkManager::Device::Ethernet) {
        NetworkManager::WiredDevice::Ptr wiredDevice = m_device.dynamicCast<NetworkManager::WiredDevice>();
        if (wiredDevice) {
            uint speed = static_cast<uint>(wiredDevice->bitRate() / 1000);
            return QString("%1 Mbps").arg(speed);
        }
    }

    if (m_device->type() == NetworkManager::Device::Wifi) {
        NetworkManager::WirelessDevice::Ptr wirelessDevice = m_device.dynamicCast<NetworkManager::WirelessDevice>();
        if (wirelessDevice) {
            uint speed = static_cast<uint>(wirelessDevice->bitRate() / 1000);
            return QString("%1 Mbps").arg(speed);
        }
    }

    return "0 Mbps";
}

void NetworkDetailNMRealize::onUpdateInfo()
{
    // 由于目前使用的qlist+qpair的数据形式，该数据有顺序显示，所以每次数据新增只能刷新所有数据，不能直接在末尾增加
    m_items.clear();
    initProperties();
}
