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

#include "kyvpnconnectoperation.h"


KyVpnConnectOperation::KyVpnConnectOperation(QObject *parent) : KyConnectOperation(parent)
{
}


void KyVpnConnectOperation::createVpnConnect(KyVpnConfig &vpnSettings)
{
    qDebug()<<"[KyVpnConnectOperation]" << "create connect ";

    NetworkManager::ConnectionSettings::Ptr vpnConnectionSettings = NetworkManager::ConnectionSettings::Ptr(new NetworkManager::ConnectionSettings(NetworkManager::ConnectionSettings::Vpn));
    setConnectionSetting(vpnConnectionSettings, vpnSettings);

    QDBusPendingCallWatcher * watcher;
    watcher = new QDBusPendingCallWatcher{NetworkManager::addConnection(vpnConnectionSettings->toMap()), this};
    connect(watcher, &QDBusPendingCallWatcher::finished, [this](QDBusPendingCallWatcher * watcher) {
        if (watcher->isError() || !watcher->isValid()) {
            QString errorMessage = tr("create vpn connection failed: ") + watcher->error().message();
            qWarning()<<errorMessage;
            Q_EMIT this->createConnectionError(errorMessage);
         } else {
            qDebug()<<"create vpn connect complete";
         }
         watcher->deleteLater();
    });

    return;
}

void KyVpnConnectOperation::getConnectionSetting(QString connectUuid, KyVpnConfig &connectSetting)
{
    qDebug() <<"[KyConnectResourse]" << connectUuid <<"get connect setting info, connect uuid";
    KyNetworkResourceManager *networkResourceInstance = KyNetworkResourceManager::getInstance();

    NetworkManager::Connection::Ptr connectPtr =
                            networkResourceInstance->getConnect(connectUuid);

    if (nullptr == connectPtr || !connectPtr->isValid()) {
        qWarning() <<"[KyConnectResourse]" << "it can not find valid connection" << connectUuid;
        return;
    }

    connectSetting.m_connectName = connectPtr->name();

    NetworkManager::ConnectionSettings::Ptr connectionSettings = connectPtr->settings();
    connectSetting.m_ifaceName = connectionSettings->interfaceName();
    connectSetting.m_isAutoConnect = connectionSettings->autoconnect();

    if (NetworkManager::ConnectionSettings::ConnectionType::Vpn != connectPtr->settings()->connectionType()
        && NetworkManager::ConnectionSettings::ConnectionType::Bond != connectPtr->settings()->connectionType()
        && NetworkManager::ConnectionSettings::ConnectionType::Bridge != connectPtr->settings()->connectionType()
        && NetworkManager::ConnectionSettings::ConnectionType::Vlan != connectPtr->settings()->connectionType()
        && NetworkManager::ConnectionSettings::ConnectionType::Team != connectPtr->settings()->connectionType()
        && NetworkManager::ConnectionSettings::ConnectionType::IpTunnel != connectPtr->settings()->connectionType()
        && NetworkManager::ConnectionSettings::ConnectionType::Wired != connectPtr->settings()->connectionType()) {
        return;
    }

    QDBusPendingReply<NMVariantMapMap> reply = connectPtr->secrets(KYVPN_VPN_KEY);
    QMap<QString,QVariantMap> secretMap(reply.value());
    QVariantMap vpnSecretMap;
    vpnSecretMap.clear();
    if (secretMap.contains(KYVPN_VPN_KEY)) {
        vpnSecretMap = secretMap.value(KYVPN_VPN_KEY);
    }

    connectSetting.m_vpnName = connectionSettings->id();
    NetworkManager::VpnSetting::Ptr vpnSettings = connectionSettings->setting(NetworkManager::Setting::Vpn).dynamicCast<NetworkManager::VpnSetting>();
    m_vpnData.clear();
    m_vpnData = vpnSettings->data();
    m_vpnSecrets.clear();
    if (!vpnSecretMap.isEmpty()) {
        vpnSettings->secretsFromMap(vpnSecretMap);
    }
    m_vpnSecrets = vpnSettings->secrets();
    connectSetting.m_gateway.clear();
    if (vpnSettings->serviceType() == VPN_SERVERTYPE_L2TP) {
        connectSetting.m_vpnType = KYVPNTYPE_L2TP;
        if (m_vpnData.contains(KYVPN_GATEWAY_KEY)) {
            connectSetting.m_gateway = m_vpnData.value(KYVPN_GATEWAY_KEY);
        }
    } else if (vpnSettings->serviceType() == VPN_SERVERTYPE_PPTP) {
        connectSetting.m_vpnType = KYVPNTYPE_PPTP;
        if (m_vpnData.contains(KYVPN_GATEWAY_KEY)) {
            connectSetting.m_gateway = m_vpnData.value(KYVPN_GATEWAY_KEY);
        }
    } else if (vpnSettings->serviceType() == VPN_SERVERTYPE_OPENVPN) {
        connectSetting.m_vpnType = KYVPNTYPE_OPENVPN;
        if (m_vpnData.contains(KYVPN_REMOTE_KEY)) {
            connectSetting.m_gateway = m_vpnData.value(KYVPN_REMOTE_KEY);
        }
    } else if (vpnSettings->serviceType() == VPN_SERVERTYPE_STRONGSWAN) {
        connectSetting.m_vpnType = KYVPNTYPE_STRONGSWAN;
        if (m_vpnData.contains(KYVPN_ADDRESS_KEY)) {
            connectSetting.m_gateway = m_vpnData.value(KYVPN_ADDRESS_KEY);
        }
    } else {
        connectSetting.m_vpnType = KYVPNTYPE_UNKNOWN;
    }

    NetworkManager::Ipv4Setting::Ptr ipv4Settings = connectionSettings->setting(NetworkManager::Setting::Ipv4).dynamicCast<NetworkManager::Ipv4Setting>();
    connectSetting.m_ipv4DnsSearch = ipv4Settings->dnsSearch();
    connectSetting.m_ipv4DhcpClientId = ipv4Settings->dhcpClientId();
    if (NetworkManager::Ipv4Setting::Automatic == ipv4Settings->method()) {
        connectSetting.m_ipv4ConfigIpType = CONFIG_IP_DHCP;
        connectSetting.m_ipv4Dns = ipv4Settings->dns();
    } else {
        connectSetting.m_ipv4ConfigIpType = CONFIG_IP_MANUAL;
        connectSetting.m_ipv4Address = ipv4Settings->addresses();
        connectSetting.m_ipv4Dns = ipv4Settings->dns();
    }

    NetworkManager::Ipv6Setting::Ptr ipv6Settings = connectionSettings->setting(NetworkManager::Setting::Ipv6).dynamicCast<NetworkManager::Ipv6Setting>();
    connectSetting.m_ipv6DnsSearch = ipv6Settings->dnsSearch();
    if (NetworkManager::Ipv6Setting::Automatic == ipv6Settings->method()) {
        connectSetting.m_ipv6ConfigIpType = CONFIG_IP_DHCP;
        connectSetting.m_ipv6Dns = ipv6Settings->dns();
    } else {
        connectSetting.m_ipv6ConfigIpType = CONFIG_IP_MANUAL;
        connectSetting.m_ipv6Address = ipv6Settings->addresses();
        connectSetting.m_ipv6Dns = ipv6Settings->dns();
    }

    return;
}

KyVpnConfig KyVpnConnectOperation::getVpnConfig(QString connectUuid)
{
    KyVpnConfig vpnConfig;
    getConnectionSetting(connectUuid, vpnConfig);

    switch (vpnConfig.m_vpnType) {
    case KYVPNTYPE_L2TP:
        getL2tpConfig(vpnConfig);
        break;
    case KYVPNTYPE_PPTP:
        getPptpConfig(vpnConfig);
        break;
    case KYVPNTYPE_STRONGSWAN:
        getStrongswanConfig(vpnConfig);
        break;
    case KYVPNTYPE_OPENVPN:
        getOpenvpnConfig(vpnConfig);
        break;
    default :
        break;
    }
    return vpnConfig;
}

NMVariantMapMap KyVpnConnectOperation::setIpConfig(NetworkManager::ConnectionSettings::Ptr connectionSettings, KyVpnConfig &vpnConfig)
{
    NetworkManager::Ipv4Setting::Ptr ipv4Settings = connectionSettings->setting(NetworkManager::Setting::Ipv4).dynamicCast<NetworkManager::Ipv4Setting>();
    ipv4SettingSet(ipv4Settings, vpnConfig);
    if (!vpnConfig.m_ipv4DnsSearch.isEmpty()) {
        ipv4Settings->setDnsSearch(vpnConfig.m_ipv4DnsSearch);
    } else {
        QStringList emptyList;
        emptyList.clear();
        ipv4Settings->setDnsSearch({""});
    }
    if (!vpnConfig.m_ipv4DhcpClientId.isEmpty()) {
        ipv4Settings->setDhcpClientId(vpnConfig.m_ipv4DhcpClientId);
    } else {
        ipv4Settings->setDhcpClientId("");
    }

    NetworkManager::Ipv6Setting::Ptr ipv6Settings = connectionSettings->setting(NetworkManager::Setting::Ipv6).dynamicCast<NetworkManager::Ipv6Setting>();
    ipv6SettingSet(ipv6Settings, vpnConfig);
    if (!vpnConfig.m_ipv6DnsSearch.isEmpty()) {
        ipv6Settings->setDnsSearch(vpnConfig.m_ipv6DnsSearch);
    }

    NMVariantMapMap connectionSettingMap = connectionSettings->toMap();
    if (connectionSettingMap.contains("ipv4")) {
        QVariantMap ipv4Map = connectionSettingMap.value(QLatin1String("ipv4"));
        bool isAuto = false;
        if (ipv4Map.contains("method") && ipv4Map["method"] == "auto") {
            qDebug() << "[KyVpnConnectOperation] set ipv4 method auto, clear address-data && addresses && gateway";
            isAuto = true;
        }
        if (isAuto) {
            if (ipv4Map.contains("address-data")) {
                ipv4Map.remove("address-data");
            }
            if (ipv4Map.contains("addresses")) {
                ipv4Map.remove("addresses");
            }
            if (ipv4Map.contains("gateway")) {
                ipv4Map.remove("gateway");
            }
            connectionSettingMap["ipv4"] = ipv4Map;
        }
    }

    if (connectionSettingMap.contains("ipv6")) {
        QVariantMap ipv6Map = connectionSettingMap.value(QLatin1String("ipv6"));
        bool isAuto = false;
        if (ipv6Map.contains("method") && ipv6Map["method"] == "auto") {
            qDebug() << "[KyVpnConnectOperation] set ipv6 method auto, clear address-data && addresses && gateway";
            isAuto = true;
        }
        if (isAuto) {
            if (ipv6Map.contains("address-data")) {
                ipv6Map.remove("address-data");
            }
            if (ipv6Map.contains("addresses")) {
                ipv6Map.remove("addresses");
            }
            if (ipv6Map.contains("gateway")) {
                ipv6Map.remove("gateway");
            }
            connectionSettingMap["ipv6"] = ipv6Map;
        }
    }

    return connectionSettingMap;
}

void KyVpnConnectOperation::setVpnConfig(QString connectUuid, KyVpnConfig &vpnConfig)
{
    qDebug() <<"[KyVpnConnectOperation]" << connectUuid <<"get connect setting info, connect uuid";
    KyNetworkResourceManager *networkResourceInstance = KyNetworkResourceManager::getInstance();

    NetworkManager::Connection::Ptr connectPtr =
                            networkResourceInstance->getConnect(connectUuid);

    if (nullptr == connectPtr || !connectPtr->isValid()) {
        qWarning() <<"[KyVpnConnectOperation]" << "it can not find valid connection" << connectUuid;
        return;
    }
    NetworkManager::ConnectionSettings::Ptr connectionSettings = connectPtr->settings();
    setConnectionSetting(connectionSettings, vpnConfig);
    NMVariantMapMap connectionSettingMap = setIpConfig(connectionSettings, vpnConfig);
    connectPtr->update(connectionSettingMap);
    return ;
}

void KyVpnConnectOperation::setConnectionSetting(NetworkManager::ConnectionSettings::Ptr connectionSettings, KyVpnConfig &vpnConfig)
{
    connectionSettings->setId(vpnConfig.m_connectName);
    connectionSettings->setUuid(NetworkManager::ConnectionSettings::createNewUuid());
    if (!vpnConfig.m_ifaceName.isEmpty()) {
        connectionSettings->setInterfaceName(vpnConfig.m_ifaceName);
    }
    connectionSettings->setAutoconnect(vpnConfig.m_isAutoConnect);

    NetworkManager::VpnSetting::Ptr vpnSettings = connectionSettings->setting(NetworkManager::Setting::Vpn).dynamicCast<NetworkManager::VpnSetting>();
    vpnSettings->setInitialized(true);
    m_vpnData.clear();
    m_vpnSecrets.clear();

    switch (vpnConfig.m_vpnType) {
    case KYVPNTYPE_L2TP:
        vpnSettings->setServiceType(VPN_SERVERTYPE_L2TP);
        setL2tpConfig(vpnConfig);
        break;
    case KYVPNTYPE_PPTP:
        vpnSettings->setServiceType(VPN_SERVERTYPE_PPTP);
        setPptpConfig(vpnConfig);
        break;
    case KYVPNTYPE_STRONGSWAN:
        vpnSettings->setServiceType(VPN_SERVERTYPE_STRONGSWAN);
        setStrongswanConfig(vpnConfig);
        break;
    case KYVPNTYPE_OPENVPN:
        vpnSettings->setServiceType(VPN_SERVERTYPE_OPENVPN);
        setOpenvpnConfig(vpnConfig);
        break;
    default :
        break;
    }

    vpnSettings->setData(m_vpnData);
    vpnSettings->setSecrets(m_vpnSecrets);
    return;
}

void KyVpnConnectOperation::getL2tpConfig(KyVpnConfig &vpnConfig)
{
    vpnConfig.m_authMethod = KYAUTH_PASSWD;

    //获取用户名
    vpnConfig.m_userName.clear();
    if (m_vpnData.contains(KYVPN_USER_KEY)) {
        vpnConfig.m_userName = m_vpnData.value(KYVPN_USER_KEY);
    }
    //获取密码和加密策略
    getUsrPasswdAndPolicy(vpnConfig);
    //获取NT域
    vpnConfig.m_ntDomain.clear();
    if (m_vpnData.contains(KYVPN_DOMAIN_KEY) && !m_vpnData.value(KYVPN_DOMAIN_KEY).isEmpty()) {
        vpnConfig.m_ntDomain = m_vpnData.value(KYVPN_DOMAIN_KEY);
    }

    if (m_vpnData.contains(KYVPN_MRU_KEY) && !m_vpnData.value(KYVPN_MRU_KEY).isEmpty()) {
        vpnConfig.m_mru = m_vpnData.value(KYVPN_MRU_KEY);
    }
    if (m_vpnData.contains(KYVPN_MTU_KEY) && !m_vpnData.value(KYVPN_MTU_KEY).isEmpty()) {
        vpnConfig.m_mtu = m_vpnData.value(KYVPN_MTU_KEY);
    }

    vpnConfig.m_authMethods.setFlag(KYAUTH2_ALL);
    if (m_vpnData.contains(KYVPN_REFUSEPAP_KEY) && m_vpnData.value(KYVPN_REFUSEPAP_KEY) == KYVPN_YES) {
        vpnConfig.m_authMethods.setFlag(KYAUTH2_PAP, false);
    }
    if (m_vpnData.contains(KYVPN_REFUSECHAP_KEY) && m_vpnData.value(KYVPN_REFUSECHAP_KEY) == KYVPN_YES) {
        vpnConfig.m_authMethods.setFlag(KYAUTH2_CHAP, false);
    }
    if (m_vpnData.contains(KYVPN_REFUSEMSCHAP_KEY) && m_vpnData.value(KYVPN_REFUSEMSCHAP_KEY) == KYVPN_YES) {
        vpnConfig.m_authMethods.setFlag(KYAUTH2_MSCHAP, false);
    }
    if (m_vpnData.contains(KYVPN_REFUSEMSCHAPV2_KEY) && m_vpnData.value(KYVPN_REFUSEMSCHAPV2_KEY) == KYVPN_YES) {
        vpnConfig.m_authMethods.setFlag(KYAUTH2_MSCHAPV2, false);
    }
    if (m_vpnData.contains(KYVPN_REFUSEEAP_KEY) && m_vpnData.value(KYVPN_REFUSEEAP_KEY) == KYVPN_YES) {
        vpnConfig.m_authMethods.setFlag(KYAUTH2_EAP, false);
    }

    vpnConfig.m_compressMethods.setFlag(KYCOMP_ALL);
    if (m_vpnData.contains(KYVPN_NOBSDCOMP_KEY) && m_vpnData.value(KYVPN_NOBSDCOMP_KEY) == KYVPN_YES) {
        vpnConfig.m_compressMethods.setFlag(KYCOMP_BSD, false);
    }
    if (m_vpnData.contains(KYVPN_NODEFLATE_KEY) && m_vpnData.value(KYVPN_NODEFLATE_KEY) == KYVPN_YES) {
        vpnConfig.m_compressMethods.setFlag(KYCOMP_DEFLATE, false);
    }
    if (m_vpnData.contains(KYVPN_NOVJCOMP_KEY) && m_vpnData.value(KYVPN_NOVJCOMP_KEY) == KYVPN_YES) {
        vpnConfig.m_compressMethods.setFlag(KYCOMP_TCP, false);
    }
    if (m_vpnData.contains(KYVPN_NOPCOMP_KEY) && m_vpnData.value(KYVPN_NOPCOMP_KEY) == KYVPN_YES) {
        vpnConfig.m_compressMethods.setFlag(KYCOMP_PROTO, false);
    }
    if (m_vpnData.contains(KYVPN_NOACCOMP_KEY) && m_vpnData.value(KYVPN_NOACCOMP_KEY) == KYVPN_YES) {
        vpnConfig.m_compressMethods.setFlag(KYCOMP_ADDR, false);
    }

    vpnConfig.m_mppeEnable = false;
    if (m_vpnData.contains(KYVPN_REQUIREMPPE_KEY)) {
        vpnConfig.m_mppeEnable = true;
        if (m_vpnData.contains(KYVPN_REQUIREMPPE128_KEY)) {
            vpnConfig.m_mppeEncryptMethod = KYMPPE_REQUIRE128;
        } else if (m_vpnData.contains(KYVPN_REQUIREMPPE40_KEY)) {
            vpnConfig.m_mppeEncryptMethod = KYMPPE_REQUIRE40;
        } else {
            vpnConfig.m_mppeEncryptMethod = KYMPPE_DEFAULT;
        }
        vpnConfig.m_authMethods.setFlag(KYAUTH2_EAP, false);
        vpnConfig.m_authMethods.setFlag(KYAUTH2_PAP, false);
        vpnConfig.m_authMethods.setFlag(KYAUTH2_CHAP, false);
    }

    vpnConfig.m_isAllowStatefulEncryption = false;
    if (m_vpnData.contains(KYVPN_MPPESTATEFUL_KEY)) {
        vpnConfig.m_isAllowStatefulEncryption = true;
    }

    vpnConfig.m_sendPppPackage = false;
    if (m_vpnData.contains(KYVPN_LCPECHOFAILURE_KEY) && m_vpnData.contains(KYVPN_LCPECHOINTERVAL_KEY)) {
        vpnConfig.m_sendPppPackage = true;
    }
}

void KyVpnConnectOperation::setL2tpConfig(KyVpnConfig &vpnConfig)
{
    //设置用户名
    if (!vpnConfig.m_userName.isEmpty()) {
        m_vpnData.insert(KYVPN_USER_KEY, vpnConfig.m_userName);
    }
    //设置密码和密码策略
    setUsrPasswdAndPolicy(vpnConfig);
    //设置地址
    if (!vpnConfig.m_gateway.isEmpty()) {
        m_vpnData.insert(KYVPN_GATEWAY_KEY, vpnConfig.m_gateway);
    }
    //设置NT域
    if (!vpnConfig.m_ntDomain.isEmpty()) {
        m_vpnData.insert(KYVPN_DOMAIN_KEY, vpnConfig.m_ntDomain);
    }

    //设置MRU 最大接收单元
    if (!vpnConfig.m_mru.isEmpty()) {
        m_vpnData.insert(KYVPN_MRU_KEY, vpnConfig.m_mru);
    }
    //设置MTU 最大传输单元
    if (!vpnConfig.m_mtu.isEmpty()) {
        m_vpnData.insert(KYVPN_MTU_KEY, vpnConfig.m_mtu);
    }

    //设置认证方式
    //不设置PAP认证，若未勾选，则对应字段设置YES
    if (!vpnConfig.m_authMethods.testFlag(KYAUTH2_PAP)) {
        m_vpnData.insert(KYVPN_REFUSEPAP_KEY, KYVPN_YES);
    }
    //不设置CHAP认证，若未勾选，则对应字段设置YES
    if (!vpnConfig.m_authMethods.testFlag(KYAUTH2_CHAP)) {
        m_vpnData.insert(KYVPN_REFUSECHAP_KEY, KYVPN_YES);
    }
    //不设置MSCHAP认证，若未勾选，则对应字段设置YES
    if (!vpnConfig.m_authMethods.testFlag(KYAUTH2_MSCHAP)) {
        m_vpnData.insert(KYVPN_REFUSEMSCHAP_KEY, KYVPN_YES);
    }
    //不设置MSCHAPV2认证，若未勾选，则对应字段设置YES
    if (!vpnConfig.m_authMethods.testFlag(KYAUTH2_MSCHAPV2)) {
        m_vpnData.insert(KYVPN_REFUSEMSCHAPV2_KEY, KYVPN_YES);
    }
    //不设置EAP认证，若未勾选，则对应字段设置YES
    if (!vpnConfig.m_authMethods.testFlag(KYAUTH2_EAP)) {
        m_vpnData.insert(KYVPN_REFUSEEAP_KEY, KYVPN_YES);
    }

    //设置压缩方式
    //不允许BSD压缩，若未勾选，则对应字段设置YES
    if (!vpnConfig.m_compressMethods.testFlag(KYCOMP_BSD)) {
        m_vpnData.insert(KYVPN_NOBSDCOMP_KEY, KYVPN_YES);
    }
    //不允许Deflate压缩，若未勾选，则对应字段设置YES
    if (!vpnConfig.m_compressMethods.testFlag(KYCOMP_DEFLATE)) {
        m_vpnData.insert(KYVPN_NODEFLATE_KEY, KYVPN_YES);
    }
    //不允许TCP头压缩，若未勾选，则对应字段设置YES
    if (!vpnConfig.m_compressMethods.testFlag(KYCOMP_TCP)) {
        m_vpnData.insert(KYVPN_NOVJCOMP_KEY, KYVPN_YES);
    }
    //不使用协议域压缩协商，若未勾选，则对应字段设置YES
    if (!vpnConfig.m_compressMethods.testFlag(KYCOMP_PROTO)) {
        m_vpnData.insert(KYVPN_NOPCOMP_KEY, KYVPN_YES);
    }
    //不使用地址/控制压缩，若未勾选，则对应字段设置YES
    if (!vpnConfig.m_compressMethods.testFlag(KYCOMP_ADDR)) {
        m_vpnData.insert(KYVPN_NOACCOMP_KEY, KYVPN_YES);
    }

    //设置MPPE加密算法
    if (vpnConfig.m_mppeEnable) {
        //设置MPPE加密算法方法为默认加密
        m_vpnData.insert(KYVPN_REQUIREMPPE_KEY, KYVPN_YES);
        switch (vpnConfig.m_mppeEncryptMethod) {
        case KYMPPE_REQUIRE128:
            //设置MPPE加密算法方法为128位加密
            m_vpnData.insert(KYVPN_REQUIREMPPE128_KEY, KYVPN_YES);
            break;
        case KYMPPE_REQUIRE40:
            //设置MPPE加密算法方法为40位加密
            m_vpnData.insert(KYVPN_REQUIREMPPE40_KEY, KYVPN_YES);
            break;
        case KYMPPE_DEFAULT:
        default:
            break;
        }

        //若使用MPPE加密算法，则不设置PAP认证、不设置CHAP认证、不设置EAP认证
        m_vpnData.insert(KYVPN_REFUSEPAP_KEY, KYVPN_YES);
        m_vpnData.insert(KYVPN_REFUSECHAP_KEY, KYVPN_YES);
        m_vpnData.insert(KYVPN_REFUSEEAP_KEY, KYVPN_YES);
    }

    //允许有状态的加密
    if (vpnConfig.m_isAllowStatefulEncryption) {
        m_vpnData.insert(KYVPN_MPPESTATEFUL_KEY, KYVPN_YES);
    }
    //发送PPP回显包
    if (vpnConfig.m_sendPppPackage) {
        m_vpnData.insert(KYVPN_LCPECHOFAILURE_KEY, "5");
        m_vpnData.insert(KYVPN_LCPECHOINTERVAL_KEY, "30");
    }
}

void KyVpnConnectOperation::getPptpConfig(KyVpnConfig &vpnConfig)
{
    vpnConfig.m_authMethod = KYAUTH_PASSWD;

    vpnConfig.m_userName.clear();
    if (m_vpnData.contains(KYVPN_USER_KEY)) {
        vpnConfig.m_userName = m_vpnData.value(KYVPN_USER_KEY);
    }

    getUsrPasswdAndPolicy(vpnConfig);

    vpnConfig.m_ntDomain.clear();
    if (m_vpnData.contains(KYVPN_DOMAIN_KEY)) {
        vpnConfig.m_ntDomain = m_vpnData.value(KYVPN_DOMAIN_KEY);
    }

    vpnConfig.m_authMethods.setFlag(KYAUTH2_ALL);
    if (m_vpnData.contains(KYVPN_REFUSEPAP_KEY) && m_vpnData.value(KYVPN_REFUSEPAP_KEY) == KYVPN_YES) {
        vpnConfig.m_authMethods.setFlag(KYAUTH2_PAP, false);
    }
    if (m_vpnData.contains(KYVPN_REFUSECHAP_KEY) && m_vpnData.value(KYVPN_REFUSECHAP_KEY) == KYVPN_YES) {
        vpnConfig.m_authMethods.setFlag(KYAUTH2_CHAP, false);
    }
    if (m_vpnData.contains(KYVPN_REFUSEMSCHAP_KEY) && m_vpnData.value(KYVPN_REFUSEMSCHAP_KEY) == KYVPN_YES) {
        vpnConfig.m_authMethods.setFlag(KYAUTH2_MSCHAP, false);
    }
    if (m_vpnData.contains(KYVPN_REFUSEMSCHAPV2_KEY) && m_vpnData.value(KYVPN_REFUSEMSCHAPV2_KEY) == KYVPN_YES) {
        vpnConfig.m_authMethods.setFlag(KYAUTH2_MSCHAPV2, false);
    }
    if (m_vpnData.contains(KYVPN_REFUSEEAP_KEY) && m_vpnData.value(KYVPN_REFUSEEAP_KEY) == KYVPN_YES) {
        vpnConfig.m_authMethods.setFlag(KYAUTH2_EAP, false);
    }

    vpnConfig.m_compressMethods.setFlag(KYCOMP_ALL);
    vpnConfig.m_compressMethods.setFlag(KYCOMP_PROTO, false);
    vpnConfig.m_compressMethods.setFlag(KYCOMP_ADDR, false);
    if (m_vpnData.contains(KYVPN_NOBSDCOMP_KEY) && m_vpnData.value(KYVPN_NOBSDCOMP_KEY) == KYVPN_YES) {
        vpnConfig.m_compressMethods.setFlag(KYCOMP_BSD, false);
    }
    if (m_vpnData.contains(KYVPN_NODEFLATE_KEY) && m_vpnData.value(KYVPN_NODEFLATE_KEY) == KYVPN_YES) {
        vpnConfig.m_compressMethods.setFlag(KYCOMP_DEFLATE, false);
    }
    if (m_vpnData.contains(KYVPN_NOVJCOMP_KEY) && m_vpnData.value(KYVPN_NOVJCOMP_KEY) == KYVPN_YES) {
        vpnConfig.m_compressMethods.setFlag(KYCOMP_TCP, false);
    }

    vpnConfig.m_mppeEnable = false;
    if (m_vpnData.contains(KYVPN_REQUIREMPPE_KEY)) {
        vpnConfig.m_mppeEnable = true;
        if (m_vpnData.contains(KYVPN_REQUIREMPPE128_KEY)) {
            vpnConfig.m_mppeEncryptMethod = KYMPPE_REQUIRE128;
        } else if (m_vpnData.contains(KYVPN_REQUIREMPPE40_KEY)) {
            vpnConfig.m_mppeEncryptMethod = KYMPPE_REQUIRE40;
        } else {
            vpnConfig.m_mppeEncryptMethod = KYMPPE_DEFAULT;
        }
        vpnConfig.m_authMethods.setFlag(KYAUTH2_EAP, false);
        vpnConfig.m_authMethods.setFlag(KYAUTH2_PAP, false);
        vpnConfig.m_authMethods.setFlag(KYAUTH2_CHAP, false);
    }

    vpnConfig.m_isAllowStatefulEncryption = false;
    if (m_vpnData.contains(KYVPN_MPPESTATEFUL_KEY)) {
        vpnConfig.m_isAllowStatefulEncryption = true;
    }

    vpnConfig.m_sendPppPackage = false;
    if (m_vpnData.contains(KYVPN_LCPECHOFAILURE_KEY) && m_vpnData.contains(KYVPN_LCPECHOINTERVAL_KEY)) {
        vpnConfig.m_sendPppPackage = true;
    }
}

void KyVpnConnectOperation::setPptpConfig(KyVpnConfig &vpnConfig)
{
    //设置用户名
    if (!vpnConfig.m_userName.isEmpty()) {
        m_vpnData.insert(KYVPN_USER_KEY, vpnConfig.m_userName);
    }
    //设置密码和密码策略
    setUsrPasswdAndPolicy(vpnConfig);
    //设置地址
    if (!vpnConfig.m_gateway.isEmpty()) {
        m_vpnData.insert(KYVPN_GATEWAY_KEY, vpnConfig.m_gateway);
    }
    //设置NT域
    if (!vpnConfig.m_ntDomain.isEmpty()) {
        m_vpnData.insert(KYVPN_DOMAIN_KEY, vpnConfig.m_ntDomain);
    }

    //设置认证方式
    //不设置PAP认证，若未勾选，则对应字段设置YES
    if (!vpnConfig.m_authMethods.testFlag(KYAUTH2_PAP)) {
        m_vpnData.insert(KYVPN_REFUSEPAP_KEY, KYVPN_YES);
    }
    //不设置CHAP认证，若未勾选，则对应字段设置YES
    if (!vpnConfig.m_authMethods.testFlag(KYAUTH2_CHAP)) {
        m_vpnData.insert(KYVPN_REFUSECHAP_KEY, KYVPN_YES);
    }
    //不设置MSCHAP认证，若未勾选，则对应字段设置YES
    if (!vpnConfig.m_authMethods.testFlag(KYAUTH2_MSCHAP)) {
        m_vpnData.insert(KYVPN_REFUSEMSCHAP_KEY, KYVPN_YES);
    }
    //不设置MSCHAPV2认证，若未勾选，则对应字段设置YES
    if (!vpnConfig.m_authMethods.testFlag(KYAUTH2_MSCHAPV2)) {
        m_vpnData.insert(KYVPN_REFUSEMSCHAPV2_KEY, KYVPN_YES);
    }
    //不设置EAP认证，若未勾选，则对应字段设置YES
    if (!vpnConfig.m_authMethods.testFlag(KYAUTH2_EAP)) {
        m_vpnData.insert(KYVPN_REFUSEEAP_KEY, KYVPN_YES);
    }

    //设置压缩方式
    //不允许BSD压缩，若未勾选，则对应字段设置YES
    if (!vpnConfig.m_compressMethods.testFlag(KYCOMP_BSD)) {
        m_vpnData.insert(KYVPN_NOBSDCOMP_KEY, KYVPN_YES);
    }
    //不允许Deflate压缩，若未勾选，则对应字段设置YES
    if (!vpnConfig.m_compressMethods.testFlag(KYCOMP_DEFLATE)) {
        m_vpnData.insert(KYVPN_NODEFLATE_KEY, KYVPN_YES);
    }
    //不允许TCP头压缩，若未勾选，则对应字段设置YES
    if (!vpnConfig.m_compressMethods.testFlag(KYCOMP_TCP)) {
        m_vpnData.insert(KYVPN_NOVJCOMP_KEY, KYVPN_YES);
    }

    //设置MPPE加密算法
    if (vpnConfig.m_mppeEnable) {
        //设置MPPE加密算法方法为默认加密
        m_vpnData.insert(KYVPN_REQUIREMPPE_KEY, KYVPN_YES);
        switch (vpnConfig.m_mppeEncryptMethod) {
        case KYMPPE_REQUIRE128:
            //设置MPPE加密算法方法为128位加密
            m_vpnData.insert(KYVPN_REQUIREMPPE128_KEY, KYVPN_YES);
            break;
        case KYMPPE_REQUIRE40:
            //设置MPPE加密算法方法为40位加密
            m_vpnData.insert(KYVPN_REQUIREMPPE40_KEY, KYVPN_YES);
            break;
        case KYMPPE_DEFAULT:
        default:
            break;
        }

        //若使用MPPE加密算法，则不设置PAP认证、不设置CHAP认证、不设置EAP认证
        m_vpnData.insert(KYVPN_REFUSEPAP_KEY, KYVPN_YES);
        m_vpnData.insert(KYVPN_REFUSECHAP_KEY, KYVPN_YES);
        m_vpnData.insert(KYVPN_REFUSEEAP_KEY, KYVPN_YES);
    }

    //允许有状态的加密
    if (vpnConfig.m_isAllowStatefulEncryption) {
        m_vpnData.insert(KYVPN_MPPESTATEFUL_KEY, KYVPN_YES);
    }
    //发送PPP回显包
    if (vpnConfig.m_sendPppPackage) {
        m_vpnData.insert(KYVPN_LCPECHOFAILURE_KEY, "5");
        m_vpnData.insert(KYVPN_LCPECHOINTERVAL_KEY, "30");
    }
}

void KyVpnConnectOperation::getStrongswanConfig(KyVpnConfig &vpnConfig)
{
    vpnConfig.m_authMethod = KYAUTH_ERROR;
    if (m_vpnData.contains(KYVPN_METHOD_KEY)) {
        vpnConfig.m_authMethod = getAuthMethod(m_vpnData.value(KYVPN_METHOD_KEY));
    }

    vpnConfig.m_userCertificate.clear();
    vpnConfig.m_userKey.clear();
    vpnConfig.m_userName.clear();
    vpnConfig.m_pinId.clear();
    switch (vpnConfig.m_authMethod) {
    case KYAUTH_KEY:
        if (m_vpnData.contains(KYVPN_USERCERT_KEY)) {
            vpnConfig.m_userCertificate = m_vpnData.value(KYVPN_USERCERT_KEY);
        }
        if (m_vpnData.contains(KYVPN_USERKEY_KEY)) {
            vpnConfig.m_userKey = m_vpnData.value(KYVPN_USERKEY_KEY);
        }
        getCertPasswdAndPolicy(vpnConfig);
        break;
    case KYAUTH_AGENT:
        if (m_vpnData.contains(KYVPN_USERCERT_KEY)) {
            vpnConfig.m_userCertificate = m_vpnData.value(KYVPN_USERCERT_KEY);
        }
        break;
    case KYAUTH_SMARTCARD:
        if (m_vpnData.contains(KYVPN_PIN_KEY)) {
            vpnConfig.m_pinId = m_vpnData.value(KYVPN_PIN_KEY);
        }
        break;
    case KYAUTH_EAP:
        if (m_vpnData.contains(KYVPN_USER_KEY)) {
            vpnConfig.m_userName = m_vpnData.value(KYVPN_USER_KEY);
        }
        getUsrPasswdAndPolicy(vpnConfig);
        break;
    default :
        break;
    }

    vpnConfig.m_virtual = false;
    if (m_vpnData.contains(KYVPN_VIRTUAL_KEY) && m_vpnData.value(KYVPN_VIRTUAL_KEY) == KYVPN_YES) {
        vpnConfig.m_virtual = true;
    }

    vpnConfig.m_encap = false;
    if (m_vpnData.contains(KYVPN_ENCAP_KEY) && m_vpnData.value(KYVPN_ENCAP_KEY) == KYVPN_YES) {
        vpnConfig.m_encap = true;
    }

    vpnConfig.m_ipcomp = false;
    if (m_vpnData.contains(KYVPN_IPCOMP_KEY) && m_vpnData.value(KYVPN_IPCOMP_KEY) == KYVPN_YES) {
        vpnConfig.m_ipcomp = true;
    }

    vpnConfig.m_proposal = false;
    if (m_vpnData.contains(KYVPN_PROPOSAL_KEY) && m_vpnData.value(KYVPN_PROPOSAL_KEY) == KYVPN_YES) {
        vpnConfig.m_proposal = true;
    }

    vpnConfig.m_ike.clear();
    if (m_vpnData.contains(KYVPN_IKE_KEY)) {
        vpnConfig.m_ike = m_vpnData.value(KYVPN_IKE_KEY);
    }

    vpnConfig.m_esp.clear();
    if (m_vpnData.contains(KYVPN_ESP_KEY)) {
        vpnConfig.m_esp = m_vpnData.value(KYVPN_ESP_KEY);
    }
}

void KyVpnConnectOperation::setStrongswanConfig(KyVpnConfig &vpnConfig)
{
    m_vpnData.insert(KYVPN_METHOD_KEY, m_authMethodMap.key(vpnConfig.m_authMethod));

    if (!vpnConfig.m_gateway.isEmpty()) {
        m_vpnData.insert(KYVPN_ADDRESS_KEY, vpnConfig.m_gateway);
    }

    switch (vpnConfig.m_authMethod) {
    case KYAUTH_KEY:
        if (!vpnConfig.m_userCertificate.isEmpty()) {
            m_vpnData.insert(KYVPN_USERCERT_KEY, vpnConfig.m_userCertificate);
        } else {
            m_vpnData.remove(KYVPN_USERCERT_KEY);
        }
        if (!vpnConfig.m_userKey.isEmpty()) {
            m_vpnData.insert(KYVPN_USERKEY_KEY, vpnConfig.m_userKey);
        } else {
            m_vpnData.remove(KYVPN_USERKEY_KEY);
        }
        setCertPasswdAndPolicy(vpnConfig);
        break;
    case KYAUTH_AGENT:
        if (!vpnConfig.m_userCertificate.isEmpty()) {
            m_vpnData.insert(KYVPN_USERCERT_KEY, vpnConfig.m_userCertificate);
        }
        break;
    case KYAUTH_SMARTCARD:
        if (!vpnConfig.m_pinId.isEmpty()) {
            m_vpnData.insert(KYVPN_PIN_KEY, vpnConfig.m_pinId);
        }
        break;
    case KYAUTH_EAP:
        if (!vpnConfig.m_userName.isEmpty()) {
            m_vpnData.insert(KYVPN_USER_KEY, vpnConfig.m_userName);
        }
        vpnConfig.m_passwdPolicy = KYPASSWD_FORALLUSER;
        setUsrPasswdAndPolicy(vpnConfig);
        break;
    default :
        break;
    }

    if (vpnConfig.m_virtual) {
        m_vpnData.insert(KYVPN_VIRTUAL_KEY, KYVPN_YES);
    } else {
        m_vpnData.insert(KYVPN_VIRTUAL_KEY, KYVPN_NO);
    }

    if (vpnConfig.m_encap) {
        m_vpnData.insert(KYVPN_ENCAP_KEY, KYVPN_YES);
    } else {
        m_vpnData.insert(KYVPN_ENCAP_KEY, KYVPN_NO);
    }

    if (vpnConfig.m_ipcomp) {
        m_vpnData.insert(KYVPN_IPCOMP_KEY, KYVPN_YES);
    } else {
        m_vpnData.insert(KYVPN_IPCOMP_KEY, KYVPN_NO);
    }

    if (vpnConfig.m_proposal) {
        m_vpnData.insert(KYVPN_PROPOSAL_KEY, KYVPN_YES);
        if (!vpnConfig.m_ike.isEmpty()) {
            m_vpnData.insert(KYVPN_IKE_KEY, vpnConfig.m_ike);
        }

        if (!vpnConfig.m_esp.isEmpty()) {
            m_vpnData.insert(KYVPN_ESP_KEY, vpnConfig.m_esp);
        }
    } else {
        m_vpnData.insert(KYVPN_PROPOSAL_KEY, KYVPN_NO);
        m_vpnData.remove(KYVPN_IKE_KEY);
        m_vpnData.remove(KYVPN_ESP_KEY);
    }
}

void KyVpnConnectOperation::getOpenvpnConfig(KyVpnConfig &vpnConfig)
{
    vpnConfig.m_authMethod = KYAUTH_ERROR;
    if (m_vpnData.contains(KYVPN_CONNECTIONTYPE_KEY)) {
        vpnConfig.m_authMethod = getAuthMethod(m_vpnData.value(KYVPN_CONNECTIONTYPE_KEY));
    }

    vpnConfig.m_caCertificate.clear();
    vpnConfig.m_userCertificate.clear();
    vpnConfig.m_userKey.clear();
    vpnConfig.m_userName.clear();
    vpnConfig.m_staticKey.clear();
    vpnConfig.m_vpnKeyDir.clear();
    vpnConfig.m_localAddress.clear();
    vpnConfig.m_remoteAddress.clear();
    switch (vpnConfig.m_authMethod) {
    case KYAUTH_CERTIFICATE:
        if (m_vpnData.contains(KYVPN_CA_KEY)) {
            vpnConfig.m_caCertificate = m_vpnData.value(KYVPN_CA_KEY);
        }
        if (m_vpnData.contains(KYVPN_CERT_KEY)) {
            vpnConfig.m_userCertificate = m_vpnData.value(KYVPN_CERT_KEY);
        }
        if (m_vpnData.contains(KYVPN_USERKEY_KEY)) {
            vpnConfig.m_userKey = m_vpnData.value(KYVPN_USERKEY_KEY);
        }
        getCertPasswdAndPolicy(vpnConfig);
        break;

    case KYAUTH_PASSWD:
        if (m_vpnData.contains(KYVPN_CA_KEY)) {
            vpnConfig.m_caCertificate = m_vpnData.value(KYVPN_CA_KEY);
        }
        if (m_vpnData.contains(KYVPN_USERNAME_KEY)) {
            vpnConfig.m_userName = m_vpnData.value(KYVPN_USERNAME_KEY);
        }
        getUsrPasswdAndPolicy(vpnConfig);
        break;

    case KYAUTH_CERTIFICATEANDPASSWD:
        if (m_vpnData.contains(KYVPN_CA_KEY)) {
            vpnConfig.m_caCertificate = m_vpnData.value(KYVPN_CA_KEY);
        }
        if (m_vpnData.contains(KYVPN_CERT_KEY)) {
            vpnConfig.m_userCertificate = m_vpnData.value(KYVPN_CERT_KEY);
        }
        if (m_vpnData.contains(KYVPN_USERKEY_KEY)) {
            vpnConfig.m_userKey = m_vpnData.value(KYVPN_USERKEY_KEY);
        }
        if (m_vpnData.contains(KYVPN_USERNAME_KEY)) {
            vpnConfig.m_userName = m_vpnData.value(KYVPN_USERNAME_KEY);
        }
        getUsrPasswdAndPolicy(vpnConfig);
        getCertPasswdAndPolicy(vpnConfig);
        break;

    case KYAUTH_STATICPASSWD:
        if (m_vpnData.contains(KYVPN_STATICKEY_KEY)) {
            vpnConfig.m_staticKey = m_vpnData.value(KYVPN_STATICKEY_KEY);
        }
        if (m_vpnData.contains(KYVPN_STATICKEYDIR_KEY)) {
            vpnConfig.m_vpnKeyDir = m_vpnData.value(KYVPN_STATICKEYDIR_KEY);
        }
        if (m_vpnData.contains(KYVPN_LOCALIP_KEY)) {
            vpnConfig.m_localAddress = m_vpnData.value(KYVPN_LOCALIP_KEY);
        }
        if (m_vpnData.contains(KYVPN_REMOTEIP_KEY)) {
            vpnConfig.m_remoteAddress = m_vpnData.value(KYVPN_REMOTEIP_KEY);
        }
        break;

    default :
        break;
    }

    vpnConfig.m_useAssignPort = false;
    vpnConfig.m_assignPort.clear();
    if (m_vpnData.contains(KYVPN_PORT_KEY)) {
        vpnConfig.m_useAssignPort = true;
        vpnConfig.m_assignPort = m_vpnData.value(KYVPN_PORT_KEY);
    }

    vpnConfig.m_useRenegSeconds = false;
    vpnConfig.m_renegSeconds.clear();
    if (m_vpnData.contains(KYVPN_RENEGSEC_KEY)) {
        vpnConfig.m_useRenegSeconds = true;
        vpnConfig.m_renegSeconds = m_vpnData.value(KYVPN_RENEGSEC_KEY);
    }

    vpnConfig.m_useCompress = false;
    vpnConfig.m_openvpnCompress = KYCOMP2_LZODISABLE;
    if (m_vpnData.contains(KYVPN_COMPLZO_KEY)) {
        vpnConfig.m_useCompress = true;
        if (m_vpnData.value(KYVPN_COMPLZO_KEY) == COMPLZO_LZODISABLE) {
            vpnConfig.m_openvpnCompress = KYCOMP2_LZODISABLE;
        } else if (m_vpnData.value(KYVPN_COMPLZO_KEY) == COMPLZO_ADAPTIVE) {
            vpnConfig.m_openvpnCompress = KYCOMP2_LZOADAPTIVE;
        }
    } else if (m_vpnData.contains(KYVPN_COMPRESS_KEY)) {
        vpnConfig.m_useCompress = true;
        if (m_vpnData.value(KYVPN_COMPRESS_KEY) == COMPRESS_LZO) {
            vpnConfig.m_openvpnCompress = KYCOMP2_LZO;
        } else if (m_vpnData.value(KYVPN_COMPRESS_KEY) == COMPRESS_LZ4) {
            vpnConfig.m_openvpnCompress = KYCOMP2_LZ4;
        } else if (m_vpnData.value(KYVPN_COMPRESS_KEY) == COMPRESS_LZ4V2) {
            vpnConfig.m_openvpnCompress = KYCOMP2_LZ4V2;
        } else if (m_vpnData.value(KYVPN_COMPRESS_KEY) == KYVPN_YES) {
            vpnConfig.m_openvpnCompress = KYCOMP2_AUTO;
        }
    }

    vpnConfig.m_useTcpLink = false;
    if (m_vpnData.contains(KYVPN_PROTOTCP_KEY) && m_vpnData.value(KYVPN_PROTOTCP_KEY) == KYVPN_YES) {
        vpnConfig.m_useTcpLink = true;
    }

    vpnConfig.m_setDevType = false;
    if (m_vpnData.contains(KYVPN_DEVTYPE_KEY)) {
        vpnConfig.m_setDevType = true;
        if (m_vpnData.value(KYVPN_DEVTYPE_KEY) == DEVTYPE_TUN) {
            vpnConfig.m_devType = KYVIRDEVTYPE_TUN;
        } else if (m_vpnData.value(KYVPN_DEVTYPE_KEY) == DEVTYPE_TAP) {
            vpnConfig.m_devType = KYVIRDEVTYPE_TAP;
        }
    }

    vpnConfig.m_setDevName = false;
    vpnConfig.m_devName.clear();
    if (m_vpnData.contains(KYVPN_DEV_KEY)) {
        vpnConfig.m_setDevName = true;
        vpnConfig.m_devName = m_vpnData.value(KYVPN_DEV_KEY);
    }

    vpnConfig.m_useTunnelMtu = false;
    vpnConfig.m_tunnelMtu.clear();
    if (m_vpnData.contains(KYVPN_TUNNELMTU_KEY)) {
        vpnConfig.m_useTunnelMtu = true;
        vpnConfig.m_tunnelMtu = m_vpnData.value(KYVPN_TUNNELMTU_KEY);
    }

    vpnConfig.m_useFragmentSize = false;
    vpnConfig.m_fragmentSize.clear();
    if (m_vpnData.contains(KYVPN_FRAGMENTSIZE_KEY)) {
        vpnConfig.m_useFragmentSize = true;
        vpnConfig.m_fragmentSize = m_vpnData.value(KYVPN_FRAGMENTSIZE_KEY);
    }

    vpnConfig.m_mssfix = false;
    if (m_vpnData.contains(KYVPN_MSSFIX_KEY) && m_vpnData.value(KYVPN_MSSFIX_KEY) == KYVPN_YES) {
        vpnConfig.m_mssfix = true;
    }

    vpnConfig.m_remoteRandom = false;
    if (m_vpnData.contains(KYVPN_REMOTERANDOM_KEY) && m_vpnData.value(KYVPN_REMOTERANDOM_KEY) == KYVPN_YES) {
        vpnConfig.m_remoteRandom = true;
    }

    vpnConfig.m_ipv6TunLink = false;
    if (m_vpnData.contains(KYVPN_TUNIPV6_KEY) && m_vpnData.value(KYVPN_TUNIPV6_KEY) == KYVPN_YES) {
        vpnConfig.m_ipv6TunLink = true;
    }

    vpnConfig.m_setPingCycle = false;
    vpnConfig.m_pingCycle.clear();
    if (m_vpnData.contains(KYVPN_PING_KEY)) {
        vpnConfig.m_setPingCycle = true;
        vpnConfig.m_pingCycle = m_vpnData.value(KYVPN_PING_KEY);
    }

    vpnConfig.m_usePingMethod = false;
    vpnConfig.m_pingMethod = KYVPNPING_EXIT;
    vpnConfig.m_pingMethodTime.clear();
    if (m_vpnData.contains(KYVPN_PINGEXIT_KEY)) {
        vpnConfig.m_usePingMethod = true;
        vpnConfig.m_pingMethod = KYVPNPING_EXIT;
        vpnConfig.m_pingMethodTime = m_vpnData.value(KYVPN_PINGEXIT_KEY);
    } else if (m_vpnData.contains(KYVPN_PINGRESTART_KEY)) {
        vpnConfig.m_usePingMethod = true;
        vpnConfig.m_pingMethod = KYVPNPING_RESTART;
        vpnConfig.m_pingMethodTime = m_vpnData.value(KYVPN_PINGRESTART_KEY);
    }

    vpnConfig.m_float = false;
    if (m_vpnData.contains(KYVPN_FLOAT_KEY) && m_vpnData.value(KYVPN_FLOAT_KEY) == KYVPN_YES) {
        vpnConfig.m_float = true;
    }

    vpnConfig.m_setMaxRoute = false;
    vpnConfig.m_maxRoute.clear();
    if (m_vpnData.contains(KYVPN_MAXROUTES_KEY)) {
        vpnConfig.m_setMaxRoute = true;
        vpnConfig.m_maxRoute = m_vpnData.value(KYVPN_MAXROUTES_KEY);
    }

    vpnConfig.m_checkServerCa = KYCHECKSERVER_NONE;
    vpnConfig.m_verifyName.clear();
    if (m_vpnData.contains(KYVPN_VERIFYX509NAME_KEY)) {
        QString nameValue = m_vpnData.value(KYVPN_VERIFYX509NAME_KEY);
        QString nameType = nameValue.section(":", 0, 0);
        if (nameType == X509NAME_SUBJECT) {
            vpnConfig.m_checkServerCa = KYCHECKSERVER_ENTIRETHEME;
        } else if (nameType == X509NAME_NAME) {
            vpnConfig.m_checkServerCa = KYCHECKSERVER_ENTIRENAME;
        } else if (nameType == X509NAME_NAMEPREFIX) {
            vpnConfig.m_checkServerCa = KYCHECKSERVER_PRENAME;
        }
        vpnConfig.m_verifyName = nameValue.section(":", 1, 1);
    }

    vpnConfig.m_useRemoteCertTls = false;
    vpnConfig.m_remoteCertType = KYVPNCERT_SERVER;
    if (m_vpnData.contains(KYVPN_REMOTECERTTLS_KEY)) {
        vpnConfig.m_useRemoteCertTls = true;
        if (m_vpnData.value(KYVPN_REMOTECERTTLS_KEY) == KYVPN_SERVER) {
            vpnConfig.m_remoteCertType = KYVPNCERT_SERVER;
        } else if (m_vpnData.value(KYVPN_REMOTECERTTLS_KEY) == KYVPN_CLIENT) {
            vpnConfig.m_remoteCertType = KYVPNCERT_CLIENT;
        }
    }

    vpnConfig.m_useNsCertTls = false;
    vpnConfig.m_nsCertType = KYVPNCERT_SERVER;
    if (m_vpnData.contains(KYVPN_NSCERTTYPE_KEY)) {
        vpnConfig.m_useNsCertTls = true;
        if (m_vpnData.value(KYVPN_NSCERTTYPE_KEY) == KYVPN_SERVER) {
            vpnConfig.m_nsCertType = KYVPNCERT_SERVER;
        } else if (m_vpnData.value(KYVPN_NSCERTTYPE_KEY) == KYVPN_CLIENT) {
            vpnConfig.m_nsCertType = KYVPNCERT_CLIENT;
        }
    }

    vpnConfig.m_vpnTlsMode = KYVPNTLS_NONE;
    if (m_vpnData.contains(KYVPN_TA_KEY)) {
        vpnConfig.m_vpnTlsMode = KYVPNTLS_AUTHENTICATION;
        vpnConfig.m_vpnKeyFile = m_vpnData.value(KYVPN_TA_KEY);
    } else if (m_vpnData.contains(KYVPN_TLSCRYPT_KEY)) {
        vpnConfig.m_vpnTlsMode = KYVPNTLS_CRYPT;
        vpnConfig.m_vpnKeyFile = m_vpnData.value(KYVPN_TLSCRYPT_KEY);
    }

    vpnConfig.m_vpnTlsTaDir.clear();
    if (m_vpnData.contains(KYVPN_TADIR_KEY)) {
        vpnConfig.m_vpnTlsTaDir = m_vpnData.value(KYVPN_TADIR_KEY);
    }

    vpnConfig.m_vpnProxyType = KYVPNPROXY_NONE;
    vpnConfig.m_vpnProxyServer.clear();
    vpnConfig.m_vpnProxyPort.clear();
    vpnConfig.m_vpnProxyRetry = false;
    vpnConfig.m_vpnProxyName.clear();
    if (m_vpnData.contains(KYVPN_PROXYTYPE_KEY)) {
        if (m_vpnData.value(KYVPN_PROXYTYPE_KEY) == PROXYTYPE_HTTP) {
            vpnConfig.m_vpnProxyType = KYVPNPROXY_HTTP;
        } else if (m_vpnData.value(KYVPN_PROXYTYPE_KEY) == PROXYTYPE_SOCKS) {
            vpnConfig.m_vpnProxyType = KYVPNPROXY_SOCKS;
        }
        if (m_vpnData.contains(KYVPN_PROXYSERVER_KEY)) {
            vpnConfig.m_vpnProxyServer = m_vpnData.value(KYVPN_PROXYSERVER_KEY);
        }
        if (m_vpnData.contains(KYVPN_PROXYPORT_KEY)) {
            vpnConfig.m_vpnProxyPort = m_vpnData.value(KYVPN_PROXYPORT_KEY);
        }
        if (m_vpnData.contains(KYVPN_PROXYRETRY_KEY) && m_vpnData.value(KYVPN_PROXYRETRY_KEY) == KYVPN_YES) {
            vpnConfig.m_vpnProxyRetry = true;
        }
        if (m_vpnData.contains(KYVPN_HTTPPROXYUSERNAME_KEY)) {
            vpnConfig.m_vpnProxyName = m_vpnData.value(KYVPN_HTTPPROXYUSERNAME_KEY);
        }
        if (m_vpnData.contains(KYVPN_HTTPPROXYPASSWDFLAGS_KEY)) {
            switch (m_vpnData.value(KYVPN_HTTPPROXYPASSWDFLAGS_KEY).toInt()) {
            case 1:
                vpnConfig.m_vpnProxyPasswdPolicy = KYPASSWD_FORTHISUSER;
                break;
            case 2:
                vpnConfig.m_vpnProxyPasswdPolicy = KYPASSWD_ASKEVERYTIME;
                break;
            case 4:
                vpnConfig.m_vpnProxyPasswdPolicy = KYPASSWD_ISNOTREQUIRED;
                break;
            default :
                break;
            }
        }
        if (m_vpnSecrets.contains(KYVPN_HTTPPROXYPASSWD_KEY)) {
            vpnConfig.m_vpnProxyPasswd = m_vpnSecrets.value(KYVPN_HTTPPROXYPASSWD_KEY);
        }
    }

    vpnConfig.m_useKeysize = false;
    vpnConfig.m_keySize.clear();
    vpnConfig.m_hmacAuthMethod = KYHMACAUTH_DEFAULT;
    if (m_vpnData.contains(KYVPN_KEYSIZE_KEY)) {
        vpnConfig.m_useKeysize = true;
        vpnConfig.m_keySize = m_vpnData.value(KYVPN_KEYSIZE_KEY);
        if (m_vpnData.contains(KYVPN_AUTH_KEY)) {
            vpnConfig.m_hmacAuthMethod = getHmacAuthMethod(m_vpnData.value(KYVPN_AUTH_KEY));

        }
    }
}

void KyVpnConnectOperation::setOpenvpnConfig(KyVpnConfig &vpnConfig)
{
    m_vpnData.insert(KYVPN_CONNECTIONTYPE_KEY, m_authMethodMap.key(vpnConfig.m_authMethod));

    if (!vpnConfig.m_gateway.isEmpty()) {
        m_vpnData.insert(KYVPN_REMOTE_KEY, vpnConfig.m_gateway);
    }

    switch (vpnConfig.m_authMethod) {
    case KYAUTH_CERTIFICATE:
        if (!vpnConfig.m_caCertificate.isEmpty()) {
            m_vpnData.insert(KYVPN_CA_KEY, vpnConfig.m_caCertificate);
        }
        if (!vpnConfig.m_userCertificate.isEmpty()) {
            m_vpnData.insert(KYVPN_CERT_KEY, vpnConfig.m_userCertificate);
        }
        if (!vpnConfig.m_userKey.isEmpty()) {
            m_vpnData.insert(KYVPN_USERKEY_KEY, vpnConfig.m_userKey);
        }
        setCertPasswdAndPolicy(vpnConfig);
        break;
    case KYAUTH_PASSWD:
        if (!vpnConfig.m_caCertificate.isEmpty()) {
            m_vpnData.insert(KYVPN_CA_KEY, vpnConfig.m_caCertificate);
        }
        if (!vpnConfig.m_userName.isEmpty()) {
            m_vpnData.insert(KYVPN_USERNAME_KEY, vpnConfig.m_userName);
        }
        setUsrPasswdAndPolicy(vpnConfig);
        break;
    case KYAUTH_CERTIFICATEANDPASSWD:
        if (!vpnConfig.m_caCertificate.isEmpty()) {
            m_vpnData.insert(KYVPN_CA_KEY, vpnConfig.m_caCertificate);
        }
        if (!vpnConfig.m_userCertificate.isEmpty()) {
            m_vpnData.insert(KYVPN_CERT_KEY, vpnConfig.m_userCertificate);
        }
        if (!vpnConfig.m_userKey.isEmpty()) {
            m_vpnData.insert(KYVPN_USERKEY_KEY, vpnConfig.m_userKey);
        }
        if (!vpnConfig.m_userName.isEmpty()) {
            m_vpnData.insert(KYVPN_USERNAME_KEY, vpnConfig.m_userName);
        }
        setUsrPasswdAndPolicy(vpnConfig);
        setCertPasswdAndPolicy(vpnConfig);
        break;
    case KYAUTH_STATICPASSWD:
        if (!vpnConfig.m_staticKey.isEmpty()) {
            m_vpnData.insert(KYVPN_STATICKEY_KEY, vpnConfig.m_staticKey);
        }
        if (!vpnConfig.m_vpnKeyDir.isEmpty()) {
            m_vpnData.insert(KYVPN_STATICKEYDIR_KEY, vpnConfig.m_vpnKeyDir);
        }
        if (!vpnConfig.m_localAddress.isEmpty()) {
            m_vpnData.insert(KYVPN_LOCALIP_KEY, vpnConfig.m_localAddress);
        }
        if (!vpnConfig.m_remoteAddress.isEmpty()) {
            m_vpnData.insert(KYVPN_REMOTEIP_KEY, vpnConfig.m_remoteAddress);
        }
        break;

    default :
        break;
    }

    if (vpnConfig.m_useAssignPort && !vpnConfig.m_assignPort.isEmpty()) {
        m_vpnData.insert(KYVPN_PORT_KEY, vpnConfig.m_assignPort);
    }

    if (vpnConfig.m_useRenegSeconds && !vpnConfig.m_renegSeconds.isEmpty()) {
        m_vpnData.insert(KYVPN_RENEGSEC_KEY, vpnConfig.m_renegSeconds);
    }

    if (vpnConfig.m_useCompress) {
        switch (vpnConfig.m_openvpnCompress) {
        case KYCOMP2_LZODISABLE:
            m_vpnData.insert(KYVPN_COMPLZO_KEY, COMPLZO_LZODISABLE);
            break;
        case KYCOMP2_LZOADAPTIVE:
            m_vpnData.insert(KYVPN_COMPLZO_KEY, COMPLZO_ADAPTIVE);
            break;
        case KYCOMP2_LZO:
            m_vpnData.insert(KYVPN_COMPRESS_KEY, COMPRESS_LZO);
            break;
        case KYCOMP2_LZ4:
            m_vpnData.insert(KYVPN_COMPRESS_KEY, COMPRESS_LZ4);
            break;
        case KYCOMP2_LZ4V2:
            m_vpnData.insert(KYVPN_COMPRESS_KEY, COMPRESS_LZ4V2);
            break;
        case KYCOMP2_AUTO:
            m_vpnData.insert(KYVPN_COMPRESS_KEY, KYVPN_YES);
            break;
        }
    }

    if (vpnConfig.m_useTcpLink) {
        m_vpnData.insert(KYVPN_PROTOTCP_KEY, KYVPN_YES);
    }

    if (vpnConfig.m_setDevType) {
        switch (vpnConfig.m_devType) {
        case KYVIRDEVTYPE_TUN:
            m_vpnData.insert(KYVPN_DEVTYPE_KEY, DEVTYPE_TUN);
            break;
        case KYVIRDEVTYPE_TAP:
            m_vpnData.insert(KYVPN_DEVTYPE_KEY, DEVTYPE_TAP);
            break;
        default :
            break;
        }
    }

    if (vpnConfig.m_setDevName && !vpnConfig.m_devName.isEmpty()) {
        m_vpnData.insert(KYVPN_DEV_KEY, vpnConfig.m_devName);
    }

    if (vpnConfig.m_useTunnelMtu && !vpnConfig.m_tunnelMtu.isEmpty()) {
        m_vpnData.insert(KYVPN_TUNNELMTU_KEY, vpnConfig.m_tunnelMtu);
    }

    if (vpnConfig.m_useFragmentSize && !vpnConfig.m_fragmentSize.isEmpty()) {
        m_vpnData.insert(KYVPN_FRAGMENTSIZE_KEY, vpnConfig.m_fragmentSize);
    }

    if (vpnConfig.m_mssfix) {
        m_vpnData.insert(KYVPN_MSSFIX_KEY, KYVPN_YES);
    }

    if (vpnConfig.m_remoteRandom) {
        m_vpnData.insert(KYVPN_REMOTERANDOM_KEY, KYVPN_YES);
    }

    if (vpnConfig.m_ipv6TunLink) {
        m_vpnData.insert(KYVPN_TUNIPV6_KEY, KYVPN_YES);
    }

    if (vpnConfig.m_setPingCycle && !vpnConfig.m_pingCycle.isEmpty()) {
        m_vpnData.insert(KYVPN_PING_KEY, vpnConfig.m_pingCycle);
    }

    if (vpnConfig.m_usePingMethod) {
        switch (vpnConfig.m_pingMethod) {
        case KYVPNPING_EXIT:
            m_vpnData.insert(KYVPN_PINGEXIT_KEY, vpnConfig.m_pingMethodTime);
            break;
        case KYVPNPING_RESTART:
            m_vpnData.insert(KYVPN_PINGRESTART_KEY, vpnConfig.m_pingMethodTime);
            break;
        default :
            break;
        }
    }

    if (vpnConfig.m_float) {
        m_vpnData.insert(KYVPN_FLOAT_KEY, KYVPN_YES);
    }

    if (vpnConfig.m_setMaxRoute && !vpnConfig.m_maxRoute.isEmpty()) {
        m_vpnData.insert(KYVPN_MAXROUTES_KEY, vpnConfig.m_maxRoute);
    }

    if (vpnConfig.m_checkServerCa != KYCHECKSERVER_NONE && !vpnConfig.m_verifyName.isEmpty()) {
        QString nameStr;
        nameStr.clear();
        switch (vpnConfig.m_checkServerCa) {
        case KYCHECKSERVER_ENTIRETHEME:
            nameStr = X509NAME_SUBJECT + QString(":") + vpnConfig.m_verifyName;
            break;
        case KYCHECKSERVER_ENTIRENAME:
            nameStr = X509NAME_NAME + QString(":") + vpnConfig.m_verifyName;
            break;
        case KYCHECKSERVER_PRENAME:
            nameStr = X509NAME_NAMEPREFIX + QString(":") + vpnConfig.m_verifyName;
            break;
        default :
            break;
        }
        m_vpnData.insert(KYVPN_VERIFYX509NAME_KEY, nameStr);
    }

    if (vpnConfig.m_useRemoteCertTls) {
        switch (vpnConfig.m_remoteCertType) {
        case KYVPNCERT_SERVER:
            m_vpnData.insert(KYVPN_REMOTECERTTLS_KEY, KYVPN_SERVER);
            break;
        case KYVPNCERT_CLIENT:
            m_vpnData.insert(KYVPN_REMOTECERTTLS_KEY, KYVPN_CLIENT);
            break;
        default :
            break;
        }
    }

    if (vpnConfig.m_useNsCertTls) {
        switch (vpnConfig.m_nsCertType) {
        case KYVPNCERT_SERVER:
            m_vpnData.insert(KYVPN_NSCERTTYPE_KEY, KYVPN_SERVER);
            break;
        case KYVPNCERT_CLIENT:
            m_vpnData.insert(KYVPN_NSCERTTYPE_KEY, KYVPN_CLIENT);
            break;
        default :
            break;
        }
    }

    switch (vpnConfig.m_vpnTlsMode) {
    case KYVPNTLS_AUTHENTICATION:
        m_vpnData.insert(KYVPN_TA_KEY, vpnConfig.m_vpnKeyFile);
        break;
    case KYVPNTLS_CRYPT:
        m_vpnData.insert(KYVPN_TLSCRYPT_KEY, vpnConfig.m_vpnKeyFile);
        break;
    default :
        break;
    }

    if (!vpnConfig.m_vpnTlsTaDir.isEmpty() && vpnConfig.m_vpnTlsTaDir != "None") {
        m_vpnData.insert(KYVPN_TADIR_KEY, vpnConfig.m_vpnTlsTaDir);
    }

    switch (vpnConfig.m_vpnProxyType) {
    case KYVPNPROXY_HTTP:
        m_vpnData.insert(KYVPN_PROXYTYPE_KEY, PROXYTYPE_HTTP);
        break;
    case KYVPNPROXY_SOCKS:
        m_vpnData.insert(KYVPN_PROXYTYPE_KEY, PROXYTYPE_SOCKS);
        break;
    default :
        break;
    }
    if (!vpnConfig.m_vpnProxyServer.isEmpty()) {
        m_vpnData.insert(KYVPN_PROXYSERVER_KEY, vpnConfig.m_vpnProxyServer);
    }
    if (!vpnConfig.m_vpnProxyPort.isEmpty()) {
        m_vpnData.insert(KYVPN_PROXYPORT_KEY, vpnConfig.m_vpnProxyPort);
    }
    if (vpnConfig.m_vpnProxyRetry) {
        m_vpnData.insert(KYVPN_PROXYRETRY_KEY, KYVPN_YES);
    }
    if (!vpnConfig.m_vpnProxyName.isEmpty()) {
        m_vpnData.insert(KYVPN_HTTPPROXYUSERNAME_KEY, vpnConfig.m_vpnProxyName);
    }

    vpnConfig.m_vpnProxyPasswdPolicy = KYPASSWD_FORALLUSER;
    if (vpnConfig.m_vpnProxyPasswdPolicy == KYPASSWD_FORALLUSER && !vpnConfig.m_vpnProxyPasswd.isEmpty()) {
        m_vpnData.insert(KYVPN_HTTPPROXYPASSWDFLAGS_KEY, "0");
        m_vpnSecrets.insert(KYVPN_HTTPPROXYPASSWD_KEY, vpnConfig.m_vpnProxyPasswd);
    }

    if (vpnConfig.m_useKeysize) {
        m_vpnData.insert(KYVPN_KEYSIZE_KEY, vpnConfig.m_keySize);
        m_vpnData.insert(KYVPN_AUTH_KEY, m_hmacMap.key(vpnConfig.m_hmacAuthMethod));
    }
}

KyHMACAuthMethod KyVpnConnectOperation::getHmacAuthMethod(const QString method)
{
    if (m_hmacMap.contains(method)) {
        return m_hmacMap.value(method);
    }
    return KYHMACAUTH_DEFAULT;
}

KyAuthMethod KyVpnConnectOperation::getAuthMethod(const QString method)
{
    if (m_authMethodMap.contains(method)) {
        return m_authMethodMap.value(method);
    }
    return KYAUTH_ERROR;
}

void KyVpnConnectOperation::getUsrPasswdAndPolicy(KyVpnConfig &vpnConfig)
{
    vpnConfig.m_userPasswd.clear();
    vpnConfig.m_passwdPolicy = KYPASSWD_ASKEVERYTIME;
    if (m_vpnData.contains(KYVPN_PASSWDFLAGS_KEY)) {
        switch (m_vpnData.value(KYVPN_PASSWDFLAGS_KEY).toInt()) {
        case 0:
            vpnConfig.m_passwdPolicy = KYPASSWD_FORALLUSER;
            if (m_vpnSecrets.contains(KYVPN_PASSWD_KEY)) {
                vpnConfig.m_userPasswd = m_vpnSecrets.value(KYVPN_PASSWD_KEY);
            }
            break;
        case 1:
            vpnConfig.m_passwdPolicy = KYPASSWD_FORTHISUSER;
            break;
        case 2:
            vpnConfig.m_passwdPolicy = KYPASSWD_ASKEVERYTIME;
            break;
        case 4:
            vpnConfig.m_passwdPolicy = KYPASSWD_ISNOTREQUIRED;
            break;
        default :
            break;
        }
    }
}

void KyVpnConnectOperation::setUsrPasswdAndPolicy(KyVpnConfig &vpnConfig)
{
    switch (vpnConfig.m_passwdPolicy) {
    case KYPASSWD_FORTHISUSER:
        m_vpnData.insert(KYVPN_PASSWDFLAGS_KEY, "1");
        break ;
    case KYPASSWD_FORALLUSER:
        m_vpnData.insert(KYVPN_PASSWDFLAGS_KEY, "0");
        break ;
    case KYPASSWD_ASKEVERYTIME:
        m_vpnData.insert(KYVPN_PASSWDFLAGS_KEY, "2");
        break ;
    case KYPASSWD_ISNOTREQUIRED:
        m_vpnData.insert(KYVPN_PASSWDFLAGS_KEY, "4");
        break ;
    default :
        break;
    }
    if (vpnConfig.m_passwdPolicy == KYPASSWD_FORALLUSER && !vpnConfig.m_userPasswd.isEmpty()) {
        m_vpnSecrets.insert(KYVPN_PASSWD_KEY, vpnConfig.m_userPasswd);
    }
}

void KyVpnConnectOperation::getCertPasswdAndPolicy(KyVpnConfig &vpnConfig)
{
    vpnConfig.m_privatePasswd.clear();
    vpnConfig.m_privatePasswdPolicy = KYPASSWD_ASKEVERYTIME;
    if (m_vpnData.contains(KYVPN_CERTPASSWDFLAGS_KEY)) {
        switch (m_vpnData.value(KYVPN_CERTPASSWDFLAGS_KEY).toInt()) {
        case 0:
            vpnConfig.m_privatePasswdPolicy = KYPASSWD_FORALLUSER;
            if (m_vpnSecrets.contains(KYVPN_CERTPASSWD_KEY)) {
                vpnConfig.m_privatePasswd = m_vpnSecrets.value(KYVPN_CERTPASSWD_KEY);
            }
            break;
        case 1:
            vpnConfig.m_privatePasswdPolicy = KYPASSWD_FORTHISUSER;
            break;
        case 2:
            vpnConfig.m_privatePasswdPolicy = KYPASSWD_ASKEVERYTIME;
            break;
        case 4:
            vpnConfig.m_privatePasswdPolicy = KYPASSWD_ISNOTREQUIRED;
            break;
        default :
            break;
        }
    }
}

void KyVpnConnectOperation::setCertPasswdAndPolicy(KyVpnConfig &vpnConfig)
{
    switch (vpnConfig.m_privatePasswdPolicy) {
    case KYPASSWD_FORTHISUSER:
        m_vpnData.insert(KYVPN_CERTPASSWDFLAGS_KEY, "1");
        break ;
    case KYPASSWD_FORALLUSER:
        m_vpnData.insert(KYVPN_CERTPASSWDFLAGS_KEY, "0");
        break ;
    case KYPASSWD_ASKEVERYTIME:
        m_vpnData.insert(KYVPN_CERTPASSWDFLAGS_KEY, "2");
        break ;
    case KYPASSWD_ISNOTREQUIRED:
        m_vpnData.insert(KYVPN_CERTPASSWDFLAGS_KEY, "4");
        break ;
    default :
        break;
    }

    if (vpnConfig.m_privatePasswdPolicy == KYPASSWD_FORALLUSER && !vpnConfig.m_privatePasswd.isEmpty()) {
        m_vpnSecrets.insert(KYVPN_CERTPASSWD_KEY, vpnConfig.m_privatePasswd);
    }
}


void KyVpnConnectOperation::activateVpnConnection(const QString connectUuid)
{
    QString connectPath = "";
    QString deviceIdentifier = "";
    QString connectName = "";
    QString specificObject = "";
    NetworkManager::Connection::Ptr connectPtr = nullptr;

    qDebug()<<"it will activate vpn connect"<<connectUuid;
    connectPtr = NetworkManager::findConnectionByUuid(connectUuid);
    if (nullptr == connectPtr) {
        QString errorMessage = "the connect uuid " + connectUuid + "is not exsit";
        qWarning()<<errorMessage;
        Q_EMIT activateConnectionError(errorMessage);
        return;
    }

    if (NetworkManager::ConnectionSettings::ConnectionType::Vpn != connectPtr->settings()->connectionType()
        && NetworkManager::ConnectionSettings::ConnectionType::Bond != connectPtr->settings()->connectionType()
        && NetworkManager::ConnectionSettings::ConnectionType::Bridge != connectPtr->settings()->connectionType()
        && NetworkManager::ConnectionSettings::ConnectionType::Vlan != connectPtr->settings()->connectionType()
        && NetworkManager::ConnectionSettings::ConnectionType::Team != connectPtr->settings()->connectionType()
        && NetworkManager::ConnectionSettings::ConnectionType::IpTunnel != connectPtr->settings()->connectionType()
        && NetworkManager::ConnectionSettings::ConnectionType::Wired != connectPtr->settings()->connectionType()) {
        QString errorMessage = tr("the connect type is")
                                + connectPtr->settings()->connectionType()
                                + tr(", but it is not vpn");
        qWarning()<<errorMessage;
        Q_EMIT activateConnectionError(errorMessage);
        return;
    }

    connectPath = connectPtr->path();
    connectName = connectPtr->name();
    specificObject = deviceIdentifier = QStringLiteral("/");

    QDBusPendingCallWatcher * watcher;
    watcher = new QDBusPendingCallWatcher{NetworkManager::activateConnection(connectPath, deviceIdentifier, specificObject), this};
    connect(watcher, &QDBusPendingCallWatcher::finished, [this, connectName] (QDBusPendingCallWatcher * watcher) {
        if (watcher->isError() || !watcher->isValid()) {
            QString errorMessage = tr("activate vpn connection failed: ") + watcher->error().message();
            qWarning()<<errorMessage;
            Q_EMIT this->activateConnectionError(errorMessage);
         } else {
            qWarning()<<"active vpn connect complete.";
         }

         watcher->deleteLater();
    });

    return;
}

void KyVpnConnectOperation::deactivateVpnConnection(const QString activeConnectName, const QString &activeConnectUuid)
{
    qDebug()<<"deactivetate connect name"<<activeConnectName<<"uuid"<<activeConnectUuid;

    deactivateConnection(activeConnectName, activeConnectUuid);

    return;
}

void KyVpnConnectOperation::deleteVpnConnect(const QString &connectUuid)
{
    qDebug()<<"delete vpn connect uuid " << connectUuid;

    deleteConnect(connectUuid);

    return ;
}
