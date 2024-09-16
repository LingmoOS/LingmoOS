// Copyright (C) 2019 ~ 2023 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "netchecksyssrvmodel.h"
#include "window/modules/common/invokers/invokerfactory.h"
#include "window/modules/common/common.h"

#include <QDebug>
#include <QProcess>
#include <QHostInfo>
#include <QFile>
#include <QThread>

#include <arpa/inet.h>
#include <bitset>

#define NET_WORK_DEVICE_TYPE_WIRED 1
#define NET_WORK_DEVICE_TYPE_WIRELESS 2
#define BAIDU_NET "www.baidu.com"

#define IPV4_LOCALHOST_REGEX ("(127.0.0.1)( |\t){1,}(localhost)")
#define IPV4_LOCALPCHOST_REGEX ("(127.0.1.1)( |\t){1,}")
#define IPV6_LOCALALLROUTERS_REGEX ("((ff02::2)( |\t){1,}(ip6-allrouters))")
#define IPV6_LOCALALLNODES_REGEX ("((ff02::1)( |\t){1,}(ip6-allnodes))")
#define IPV6_LOCALCAST_REGEX ("((ff00::0)( |\t){1,}(ip6-mcastprefix))")
#define IPV6_LOCALNET_REGEX ("((fe00::0)( |\t){1,}(ip6-localnet))")
#define IPV6_LOCALHOST_REGEX ("((::1)( |\t){1,}(localhost|ip6-localhost|ip6-loopback)( |\t){1,}(localhost|ip6-localhost|ip6-loopback)( |\t){1,}(localhost|ip6-localhost|ip6-loopback))")
#define IPV6_ADDRESS_REGEX \
    ("(( " \
     "|\t){0,}([0-9a-fA-F]{1,4}:){7,7}[0-9a-fA-F]{1,4}|([0-9a-fA-F]{1,4}:){1,7}:|([0-9a-fA-F]{1,4}:){1,6}:[0-9a-fA-F]" \
     "{1,4}|([0-9a-fA-F]{1,4}:){1,5}(:[0-9a-fA-F]{1,4}){1,2}|([0-9a-fA-F]{1,4}:){1,4}(:[0-9a-fA-F]{1,4}){1,3}|([0-9a-" \
     "fA-F]{1,4}:){1,3}(:[0-9a-fA-F]{1,4}){1,4}|([0-9a-fA-F]{1,4}:){1,2}(:[0-9a-fA-F]{1,4}){1,5}|[0-9a-fA-F]{1,4}:((:" \
     "[0-9a-fA-F]{1,4}){1,6})|:((:[0-9a-fA-F]{1,4}){1,7}|:)|fe80:(:[0-9a-fA-F]{0,4}){0,4}%[0-9a-zA-Z]{1,}|::(ffff(:0{" \
     "1,4}){0,1}:){0,1}((25[0-5]|(2[0-4]|1{0,1}[0-9]){0,1}[0-9])\.){3,3}(25[0-5]|(2[0-4]|1{0,1}[0-9]){0,1}[0-9])|([0-" \
     "9a-fA-F]{1,4}:){1,4}:((25[0-5]|(2[0-4]|1{0,1}[0-9]){0,1}[0-9])\.){3,3}(25[0-5]|(2[0-4]|1{0,1}[0-9]){0,1}[0-9])(" \
     "( |\t){1,}([0-9a-zA-Z]{1,16}\.){1,}([0-9a-zA-Z]{1,16})){1,}) ")
#define IPV4_ADDRESS_REGEX \
    ("(( |\t){0,}(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)(\.(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)){3}(( " \
     "|\t){1,}([0-9a-zA-Z]{1,16}\.){1,}([0-9a-zA-Z]{1,16})){1,})")

NetCheckSysSrvModel::NetCheckSysSrvModel(QObject *parent)
    : QObject(parent)
    , m_netWorkManagerDBusInter(nullptr)
    , m_netWorkDBusInter(nullptr)
{
    // 创建网络管理服务对象
    m_netWorkManagerDBusInter = InvokerFactory::GetInstance().CreateInvoker("org.freedesktop.NetworkManager",
                                                                            "/org/freedesktop/NetworkManager",
                                                                            "org.freedesktop.NetworkManager",
                                                                            ConnectType::SYSTEM, this);

    m_netWorkDBusInter = InvokerFactory::GetInstance().CreateInvoker("org.deepin.dde.Network1",
                                                                     "/org/deepin/dde/Network1",
                                                                     "org.deepin.dde.Network1",
                                                                     ConnectType::SYSTEM, this);

    //    startCheckNetDevice();
    //    startCheckNetConnSetting();
    //    startCheckNetDHCP();
    //    startCheckNetDNS();
    //    startCheckNetHost();
    //    startCheckNetConn("");
}

NetCheckSysSrvModel::~NetCheckSysSrvModel()
{
}

void NetCheckSysSrvModel::initData()
{
}

// 开始网络硬件检测
void NetCheckSysSrvModel::startCheckNetDevice()
{
    bool flag = false;

    // 获取设备禁用状态
    QDBusMessage msgService = DBUS_BLOCK_INVOKE(m_netWorkManagerDBusInter, "GetAllDevices");
    GET_MESSAGE_VALUE(QList<QDBusObjectPath>, paths, msgService);
    m_allServicePaths = paths;
    for (QDBusObjectPath servicePath : m_allServicePaths) {
        DBusInvokerInterface *m_netWorkManagerServiceDBusInter = InvokerFactory::GetInstance().CreateInvoker("org.freedesktop.NetworkManager",
                                                                                                             servicePath.path(),
                                                                                                             "org.freedesktop.NetworkManager.Device",
                                                                                                             ConnectType::SYSTEM, this);
        QDBusMessage msgServiceType = DBUS_BLOCK_INVOKE(m_netWorkManagerServiceDBusInter, DBUS_PROPERTY_INVOKER_NAME, "DeviceType");
        QVariant var = Utils::checkAndTransQDbusVarIntoQVar(msgServiceType.arguments().first());
        int deviceType = var.toInt();
        if (NET_WORK_DEVICE_TYPE_WIRED == deviceType || NET_WORK_DEVICE_TYPE_WIRELESS == deviceType) {
            QDBusMessage msgNetWork = DBUS_BLOCK_INVOKE(m_netWorkDBusInter, "IsDeviceEnabled", servicePath.path());
            GET_MESSAGE_VALUE(bool, status, msgNetWork);
            if (status) {
                flag = true;

                // 获取网络连接状态
                QDBusMessage msgNetConnStatus = DBUS_BLOCK_INVOKE(m_netWorkManagerServiceDBusInter, DBUS_PROPERTY_INVOKER_NAME, "ActiveConnection");
                QVariant varNetConnStatus = Utils::checkAndTransQDbusVarIntoQVar(msgNetConnStatus.arguments().first());
                QString netConnStatus = varNetConnStatus.toString();
                if ("/" != netConnStatus) {
                    m_netConnStatus = true;
                }
                // 获取自动连接状态
                QDBusMessage msgAutoConnStatus = DBUS_BLOCK_INVOKE(m_netWorkManagerServiceDBusInter, DBUS_PROPERTY_INVOKER_NAME, "AutoConnect");
                QVariant varAutoConnStatus = Utils::checkAndTransQDbusVarIntoQVar(msgAutoConnStatus.arguments().first());
                m_netAutoConnStatus = varAutoConnStatus.toBool();
            }
        }
    }

    if (flag) {
        // 获取设备连接情况
        QDBusMessage msg = DBUS_BLOCK_INVOKE(m_netWorkManagerDBusInter, DBUS_PROPERTY_INVOKER_NAME, "PrimaryConnection");
        QVariant var = Utils::checkAndTransQDbusVarIntoQVar(msg.arguments().first());
        QDBusObjectPath objectPath = qvariant_cast<QDBusObjectPath>(var);
        m_primaryConnPath = objectPath.path();
        if ("/" == m_primaryConnPath) {
            Q_EMIT sendCheckNetDevice(ServiceResult::NoFound);
        } else {
            Q_EMIT sendCheckNetDevice(ServiceResult::CheckSucessed);
        }
    } else {
        Q_EMIT sendCheckNetDevice(ServiceResult::ServiceBlock);
    }
}

// 开始网络连接配置检测
void NetCheckSysSrvModel::startCheckNetConnSetting()
{
    // 网络连接检测
    if (!m_netConnStatus || m_primaryConnPath.isEmpty()) {
        Q_EMIT sendCheckNetConnSetting(NetSettingResult::NoNetConn);
        return;
    }

    DBusInvokerInterface *m_netWorkManagerActiveDBusInter = InvokerFactory::GetInstance().CreateInvoker("org.freedesktop.NetworkManager",
                                                                                                        m_primaryConnPath,
                                                                                                        "org.freedesktop.NetworkManager.Connection.Active",
                                                                                                        ConnectType::SYSTEM, this);
    QDBusMessage msgActive = DBUS_BLOCK_INVOKE(m_netWorkManagerActiveDBusInter, DBUS_PROPERTY_INVOKER_NAME, "Connection");
    QVariant varActive = Utils::checkAndTransQDbusVarIntoQVar(msgActive.arguments().first());
    QDBusObjectPath objectPath = qvariant_cast<QDBusObjectPath>(varActive);
    m_settingConnPath = objectPath.path();

    // 网络连接检测
    if (m_settingConnPath.isEmpty()) {
        Q_EMIT sendCheckNetConnSetting(NetSettingResult::NoNetConn);
        return;
    }
    // 自动连接，跳过检测，进行DHCP检测
    if (m_netAutoConnStatus) {
        Q_EMIT sendCheckNetConnSetting(NetSettingResult::NetSettingSkip);
        return;
    }

    DBusInvokerInterface *m_netWorkManagerSettingDBusInter = InvokerFactory::GetInstance().CreateInvoker("org.freedesktop.NetworkManager",
                                                                                                         m_settingConnPath,
                                                                                                         "org.freedesktop.NetworkManager.Settings.Connection",
                                                                                                         ConnectType::SYSTEM, this);
    QDBusMessage msgSetting = DBUS_BLOCK_INVOKE(m_netWorkManagerSettingDBusInter, "GetSettings");
    QDBusArgument argSetting = msgSetting.arguments().first().value<QDBusArgument>();
    argSetting >> m_settings;

    // 网络连接配置检测
    if (checkNetConnSetting()) {
        Q_EMIT sendCheckNetConnSetting(NetSettingResult::NetSettingCheckSucessed);
    } else {
        Q_EMIT sendCheckNetConnSetting(NetSettingResult::NetSettingCheckFailed);
    }
}

// 开始DHCP检测
void NetCheckSysSrvModel::startCheckNetDHCP()
{
    quint32 m_DHCP4IP = 0;
    quint32 m_DHCP4NetMask = 0;
    quint32 m_DHCP4NetGateWay = 0;
    DBusInvokerInterface *m_netWorkManagerActiveDBusInter = InvokerFactory::GetInstance().CreateInvoker("org.freedesktop.NetworkManager",
                                                                                                        m_primaryConnPath,
                                                                                                        "org.freedesktop.NetworkManager.Connection.Active",
                                                                                                        ConnectType::SYSTEM, this);
    QDBusMessage msgActive = DBUS_BLOCK_INVOKE(m_netWorkManagerActiveDBusInter, DBUS_PROPERTY_INVOKER_NAME, "Dhcp4Config");
    QVariant varActive = Utils::checkAndTransQDbusVarIntoQVar(msgActive.arguments().first());
    QDBusObjectPath objectPath = qvariant_cast<QDBusObjectPath>(varActive);
    m_dhcp4ConnPath = objectPath.path();

    DBusInvokerInterface *m_netWorkManagerDHCPDBusInter = InvokerFactory::GetInstance().CreateInvoker("org.freedesktop.NetworkManager",
                                                                                                      m_dhcp4ConnPath,
                                                                                                      "org.freedesktop.NetworkManager.DHCP4Config",
                                                                                                      ConnectType::SYSTEM, this);
    QDBusMessage msgDHCP = DBUS_BLOCK_INVOKE(m_netWorkManagerDHCPDBusInter, DBUS_PROPERTY_INVOKER_NAME, "Options");
    QList<QVariant> argDHCP = msgDHCP.arguments();

    if (!argDHCP.isEmpty()) {
        QDBusVariant dbusDHCP = argDHCP.at(0).value<QDBusVariant>();
        QVariant varDHCP = dbusDHCP.variant();
        const QDBusArgument &argDHCPS = varDHCP.value<QDBusArgument>();
        QMap<QString, QVariant> dhcpList;
        argDHCPS >> dhcpList;
        if (!dhcpList.isEmpty()) {
            for (QString inner_key : dhcpList.keys()) {
                if (inner_key == "ip_address") {
                    QString ipv4 = dhcpList.value(inner_key).toString();
                    m_DHCP4IP = ipv4StringToInteger(ipv4);
                }
                if (inner_key == "subnet_mask") {
                    QString subnet_mask = dhcpList.value(inner_key).toString();
                    m_DHCP4NetMask = subMaskStringToInteger(subnet_mask);
                }
                if (inner_key == "routers") {
                    QString routers = dhcpList.value(inner_key).toString();
                    m_DHCP4NetGateWay = ipv4StringToInteger(routers);
                }
            }

            quint32 mask = 0;
            for (quint32 i = 31; i > (32 - m_DHCP4NetMask); --i) {
                mask |= 1 << i;
            }

            if ((m_DHCP4IP & mask) != (m_DHCP4NetGateWay & mask)) {
                Q_EMIT sendCheckNetDHCP(DHCPResult::DHCPCheckFailed);
            } else {
                Q_EMIT sendCheckNetDHCP(DHCPResult::DHCPCheckSucessed);
            }
        } else {
            Q_EMIT sendCheckNetDHCP(DHCPResult::DHCPCheckFailed);
        }
    } else {
        Q_EMIT sendCheckNetDHCP(DHCPResult::DHCPCheckFailed);
    }
}
// 开始DNS检测
void NetCheckSysSrvModel::startCheckNetDNS()
{
    DBusInvokerInterface *m_netWorkManagerActiveDBusInter = InvokerFactory::GetInstance().CreateInvoker("org.freedesktop.NetworkManager",
                                                                                                        m_primaryConnPath,
                                                                                                        "org.freedesktop.NetworkManager.Connection.Active",
                                                                                                        ConnectType::SYSTEM, this);
    QDBusMessage msgActive = DBUS_BLOCK_INVOKE(m_netWorkManagerActiveDBusInter, DBUS_PROPERTY_INVOKER_NAME, "Ip4Config");
    QVariant varActive = Utils::checkAndTransQDbusVarIntoQVar(msgActive.arguments().first());
    QDBusObjectPath objectPath = qvariant_cast<QDBusObjectPath>(varActive);
    m_ip4ConfigConnPath = objectPath.path();

    DBusInvokerInterface *m_netWorkManagerIp4ConfigDBusInter = InvokerFactory::GetInstance().CreateInvoker("org.freedesktop.NetworkManager",
                                                                                                           m_ip4ConfigConnPath,
                                                                                                           "org.freedesktop.NetworkManager.IP4Config",
                                                                                                           ConnectType::SYSTEM, this);
    QDBusMessage msgIp4Config = DBUS_BLOCK_INVOKE(m_netWorkManagerIp4ConfigDBusInter, DBUS_PROPERTY_INVOKER_NAME, "NameserverData");
    QList<QVariant> argIp4Config = msgIp4Config.arguments();

    if (!argIp4Config.isEmpty()) {
        QDBusVariant dbusIp4Config = argIp4Config.at(0).value<QDBusVariant>();
        QVariant varIp4Config = dbusIp4Config.variant();
        const QDBusArgument &argIp4Configs = varIp4Config.value<QDBusArgument>();
        QList<QMap<QString, QVariant>> ip4ConfigList;
        argIp4Configs >> ip4ConfigList;

        QStringList ip4DNSList;
        if (!ip4ConfigList.isEmpty()) {
            for (const auto &config : ip4ConfigList) {
                QStringList keys = config.keys();
                for (const auto &key : keys) {
                    if (key == "address") {
                        QString dnsAddress = config.value("address").toString();
                        if (!dnsAddress.isEmpty())
                            ip4DNSList.append(dnsAddress);
                    }
                }
            }

            if (ip4DNSList.count() > 0) {
                if (!pingCheck(ip4DNSList.at(0))) {
                    Q_EMIT sendCheckNetDNS(DNSResult::DNSCheckError);
                } else {
                    if (!nslookupDNS(ip4DNSList.at(0))) {
                        Q_EMIT sendCheckNetDNS(DNSResult::DNSCheckAbnormal);
                    } else {
                        Q_EMIT sendCheckNetDNS(DNSResult::DNSCheckSucessed);
                    }
                }
            } else {
                Q_EMIT sendCheckNetDNS(DNSResult::DNSCheckError);
            }
        } else {
            Q_EMIT sendCheckNetDNS(DNSResult::DNSCheckError);
        }
    } else {
        Q_EMIT sendCheckNetDNS(DNSResult::DNSCheckError);
    }
}
// 开始host检测
void NetCheckSysSrvModel::startCheckNetHost()
{
    QString hostName = QHostInfo::localHostName();
    bool result = true;
    bool result1 = false;
    bool result2 = false;
    bool result3 = false;
    bool result4 = false;
    bool result5 = false;
    bool result6 = false;

    QFile hostFile("/etc/hosts");
    if (!hostFile.exists()) {
        Q_EMIT sendCheckNetHost(HostResult::HostCheckFailed);
        return;
    }
    if (!hostFile.open(QFile::ReadOnly)) {
        Q_EMIT sendCheckNetHost(HostResult::HostCheckFailed);
        return;
    } else {
        QTextStream out(&hostFile);
        QString line;

        while (!out.atEnd()) {
            line = out.readLine();

            if (line.startsWith("#")) {
                continue;
            } else if (line.isEmpty()) {
                continue;
            } else if (line.contains("127.0.0.1")) {
                QRegExp ipv4Regex(IPV4_LOCALHOST_REGEX);
                result1 = ipv4Regex.exactMatch(line);
            } else if (line.contains("::1") && (!line.contains("ff02::1"))) {
                QRegExp rx(IPV6_LOCALHOST_REGEX);
                result2 = rx.exactMatch(line);
            } else if (line.contains("fe00::0")) {
                QRegExp rx(IPV6_LOCALNET_REGEX);
                result3 = rx.exactMatch(line);
            } else if (line.contains("ff00::0")) {
                QRegExp rx(IPV6_LOCALCAST_REGEX);
                result4 = rx.exactMatch(line);
            } else if (line.contains("ff02::1")) {
                QRegExp rx(IPV6_LOCALALLNODES_REGEX);
                result5 = rx.exactMatch(line);
            } else if (line.contains("ff02::2")) {
                QRegExp rx(IPV6_LOCALALLROUTERS_REGEX);
                result6 = rx.exactMatch(line);
            } else {
                QRegExp ipv4Rx(IPV4_ADDRESS_REGEX);
                QRegExp ipv6Rx(IPV6_ADDRESS_REGEX);
                bool ipv4 = ipv4Rx.exactMatch(line);
                bool ipv6 = ipv6Rx.exactMatch(line);
                if (!ipv4 && !ipv6) {
                    result = false;
                }
            }
        }
        hostFile.close();

        if (result && result1 && result2 && result3 && result4 & result5 && result6) {
            Q_EMIT sendCheckNetHost(HostResult::HostCheckSucessed);
        } else {
            Q_EMIT sendCheckNetHost(HostResult::HostCheckFailed);
        }
    }
}

// 开始网络连接访问
void NetCheckSysSrvModel::startCheckNetConn(QString url)
{
    if (pingCheck(url) || pingCheck(BAIDU_NET)) {
        Q_EMIT sendCheckNetConn(NetConnResult::NetConnSucessed);
    } else {
        Q_EMIT sendCheckNetConn(NetConnResult::NetConnFailed);
    }
}

bool NetCheckSysSrvModel::checkNetConnSetting()
{
    QString ip; /* 点分十进制 */
    QString gateway; /* 点分十进制 */
    QString mask; /* 32位二进制 */
    if (m_settings.contains("ipv4")) {
        QMap<QString, QVariant> ipv4 = m_settings.value("ipv4");
        if (ipv4.contains("address-data")) {
            const QDBusArgument addressDataDbusArg = ipv4.value("address-data").value<QDBusArgument>();
            QVector<QMap<QString, QVariant>> addressData;
            addressDataDbusArg >> addressData;
            if (addressData.size() != 1) {
                qDebug() << "multiple network cards";
            } else {
                QMap<QString, QVariant> addressItem = addressData.at(0);
                if (addressItem.contains("address"))
                    ip = addressItem.value("address").toString();

                if (addressItem.contains("prefix")) {
                    uint prefix = addressItem.value("prefix").toUInt();
                    for (uint i = 0; i < prefix; i++)
                        mask.append('1');

                    for (uint j = 0; j < 32 - prefix; j++)
                        mask.append('0');
                }
            }
        }

        if (ipv4.contains("gateway"))
            gateway = ipv4.value("gateway").toString();
    }

    qDebug() << "ip: " << ip;
    qDebug() << "gateway: " << gateway;
    qDebug() << "mask: " << mask;

    struct in_addr ipAddr;
    struct in_addr gatewayAddr;
    if (inet_pton(AF_INET, ip.toStdString().c_str(), (void *)&ipAddr) != 1) {
        qCritical() << "ip addr conversion fail !";
        return false;
    }
    if (inet_pton(AF_INET, gateway.toStdString().c_str(), (void *)&gatewayAddr) != 1) {
        qCritical() << "gateway conversion fail !";
        return false;
    }

    uint32_t ipn = ntohl(ipAddr.s_addr);
    uint32_t gatewayn = ntohl(gatewayAddr.s_addr);

    std::bitset<32> bitIp(ipn);
    std::bitset<32> bitGateway(gatewayn);
    std::bitset<32> bitMask(mask.toStdString());

    if ((bitIp & bitMask) == (bitGateway & bitMask))
        return true;

    return false;
}

quint32 NetCheckSysSrvModel::ipv4StringToInteger(const QString &ip)
{
    QStringList ips = ip.split(".");
    if (ips.size() == 4) {
        return ips.at(0).toInt() << 24 | ips.at(1).toInt() << 16 | ips.at(2).toInt() << 8 | ips.at(3).toInt();
    }
    return 0;
}
int NetCheckSysSrvModel::subMaskStringToInteger(const QString &ip)
{
    int subDigital = 0;
    QStringList ips = ip.split(".");
    if (ips.size() == 4) {
        for (auto ipStr : ips) {
            int ip = ipStr.toInt();
            if (ip == 255) {
                subDigital += 8;
            } else if (ip == 0) {
                continue;
            } else {
                while (ip > 0) {
                    subDigital += ip % 2;
                    ip /= 2;
                }
            }
        }
    }
    return subDigital;
}

bool NetCheckSysSrvModel::pingCheck(QString url)
{
    if (url.isEmpty()) {
        return false;
    }

    QString sProcessResult;
    QString cmdStr = QString("ping -c 5 -q %1").arg(url);
    QProcess *process = new QProcess();
    process->setProcessChannelMode(QProcess::ProcessChannelMode::MergedChannels);
    process->start(cmdStr);
    bool isFinish = process->waitForFinished();
    if (isFinish) {
        QThread::msleep(1000);
        sProcessResult = QString::fromLocal8Bit(process->readAllStandardOutput()).trimmed();
    } else {
        return false;
    }

    QStringList list = sProcessResult.split(",");
    for (int i = 0; i < list.count(); ++i) {
        QString str = list.at(i);
        if (str.contains("%")) {
            int packetLost = str.split("%").at(0).toInt();
            if (100 != packetLost) {
                return true;
            }
        }
    }
    return false;
}

bool NetCheckSysSrvModel::nslookupDNS(QString dns)
{
    //    QProcess *process = new QProcess;
    //    process->setProgram("nslookup");
    //    process->setArguments(QStringList() << "-ty=ptr" << dns);
    //    process->setProcessChannelMode(QProcess::ProcessChannelMode::MergedChannels);
    //    process->waitForStarted();
    //    process->waitForFinished();
    //    QString sProcessResult = QString::fromLocal8Bit(process->readAllStandardOutput()).trimmed();
    //    process->close();

    //    if (sProcessResult.contains("Non-authoritative")) {
    //        return true;
    //    }
    return true;
}

bool NetCheckSysSrvModel::checkHostLineFormat(QString needCheck, QString headReg, QStringList list)
{
    if (needCheck.isEmpty()) {
        return false;
    }
    QString real = needCheck.trimmed().replace("\t", " ");
    QStringList needList = real.split(" ");
    if (needList.size() < 2) {
        return false;
    }
    if (needList.first() == headReg) {
        if (list.isEmpty()) {
            return false;
        }
        for (auto child : list) {
            if (!needList.contains(child)) {
                return false;
            }
        }
        return true;
    } else {
        return false;
    }
}
