/*
 * Copyright (C) 2020 Tianjin LINGMO Information Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/&gt;.
 *
 */
#include "lingmoconnectsetting.h"
#include <QDebug>

KyConnectSetting::KyConnectSetting(/*QObject *parent) : QObject(parent*/)
{
    m_connectName = "";
    m_ifaceName = "";

    m_ipv4ConfigIpType = CONFIG_IP_DHCP;
    m_ipv4Address.clear();
    m_ipv4Dns.clear();

    m_ipv6ConfigIpType = CONFIG_IP_DHCP;
    m_ipv6Address.clear();
    m_ipv6Dns.clear();

    m_isAutoConnect = true;
}

KyConnectSetting::~KyConnectSetting()
{

}

void KyConnectSetting::setIfaceName(QString &ifaceName)
{
    m_ifaceName = ifaceName;
}

void KyConnectSetting::setConnectName(QString &connectName)
{
    m_connectName = connectName;
}

int KyConnectSetting::setIpConfigType(KyIpAddressType ipType, KyIpConfigType ipConfigType)
{
    if (ipType != IPADDRESS_V4 && ipType != IPADDRESS_V6) {
        qWarning()<<"set config ip type failed, the ip address type undefined"<<ipType;
        return -EINVAL;
    }

    if (CONFIG_IP_DHCP != ipConfigType && CONFIG_IP_MANUAL != ipConfigType) {
        qWarning()<<"set config ip type failed, the config ip type undefined"<<ipConfigType;
        return -EINVAL;
    }

    if (IPADDRESS_V4 == ipType) {
        m_ipv4ConfigIpType = ipConfigType;
    } else {
        m_ipv6ConfigIpType = ipConfigType;
    }

    return 0;
}

void KyConnectSetting::ipv4AddressConstruct(QString &ipv4Address, QString &ipv4NetMask, QString &ipv4GateWay)
{
    qDebug()<<"ipv4 address"<<ipv4Address << ipv4NetMask << ipv4GateWay;
    NetworkManager::IpAddress  nmIpv4Address;
    nmIpv4Address.setIp(QHostAddress(ipv4Address));
    nmIpv4Address.setGateway(QHostAddress(ipv4GateWay));
    nmIpv4Address.setNetmask(QHostAddress(ipv4NetMask));
    m_ipv4Address.clear();
    m_ipv4Address << nmIpv4Address;
}

void KyConnectSetting::ipv4DnsConstruct(QList<QHostAddress> &ipv4Dns)
{
    m_ipv4Dns = ipv4Dns;
}

void KyConnectSetting::ipv6AddressConstruct(QString &ipv6Address, QString &ipv6NetMask, QString &ipv6GateWay)
{
    NetworkManager::IpAddress  nmIpv6Address;
    nmIpv6Address.setIp(QHostAddress(ipv6Address));
    nmIpv6Address.setGateway(QHostAddress(ipv6GateWay));
    nmIpv6Address.setPrefixLength(ipv6NetMask.toInt());
    m_ipv6Address.clear();
    m_ipv6Address << nmIpv6Address;
}

void KyConnectSetting::ipv6DnsConstruct(QList<QHostAddress> &ipv6Dns)
{
    m_ipv6Dns = ipv6Dns;
}

void KyConnectSetting::dumpInfo()
{
    qDebug()<<"connect setting info:";
    qDebug()<<"connect name"<< m_connectName;
    qDebug()<<"iface name" << m_ifaceName;
    qDebug()<<"ipv4 method" <<m_ipv4ConfigIpType;

    for (int index = 0 ; index < m_ipv4Address.size(); index++) {
        qDebug()<<"ipv4 address" << m_ipv4Address.at(index).ip().toString();
        qDebug()<<"ipv4 gate way" << m_ipv4Address.at(index).gateway().toString();
        qDebug()<<"ipv4 net mask" << m_ipv4Address.at(index).netmask().toString();
    }

    for (int index = 0; index < m_ipv4Dns.size(); ++index) {
        qDebug()<<"ipv4 dns" << m_ipv4Dns.at(index).toString();
    }

    for (int index = 0 ; index < m_ipv6Address.size(); index++) {
        qDebug()<<"ipv6 address" << m_ipv6Address.at(index).ip().toString();
        qDebug()<<"ipv6 gate way" << m_ipv6Address.at(index).gateway().toString();
        qDebug()<<"ipv6 net mask" << m_ipv6Address.at(index).netmask().toString();
    }

    for (int index = 0; index < m_ipv6Dns.size(); ++index) {
        qDebug()<<"ipv6 dns" << m_ipv6Dns.at(index).toString();
    }
}
