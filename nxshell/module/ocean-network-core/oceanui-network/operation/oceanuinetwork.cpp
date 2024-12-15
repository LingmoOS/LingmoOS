// SPDX-FileCopyrightText: 2024 - 2027 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later
#include "oceanuinetwork.h"

#include "ocean-control-center/oceanuifactory.h"
#include "netitemmodel.h"
#include "netmanager.h"

#include <NetworkManagerQt/GenericTypes>

#include <QClipboard>
#include <QCoreApplication>
#include <QDebug>
#include <QGuiApplication>
#include <QHostAddress>
#include <QThread>
#include <QtQml/QQmlEngine>

// #include "oceanuifactory.h"

namespace ocean {
namespace network {
OceanUINetwork::OceanUINetwork(QObject *parent)
    : QObject(parent)
    , m_manager(nullptr)
{
    qRegisterMetaType<QList<QVariantMap>>("NMVariantMapList");
    qmlRegisterType<NetType>("org.lingmo.oceanui.network", 1, 0, "NetType");
    qmlRegisterType<NetItemModel>("org.lingmo.oceanui.network", 1, 0, "NetItemModel");
    qmlRegisterType<NetManager>("org.lingmo.oceanui.network", 1, 0, "NetManager");
    QMetaObject::invokeMethod(this, "init", Qt::QueuedConnection);
}

NetItem *OceanUINetwork::root() const
{
    return m_manager->root();
}

bool OceanUINetwork::CheckPasswordValid(const QString &key, const QString &password)
{
    return NetManager::CheckPasswordValid(key, password);
}

void OceanUINetwork::exec(NetManager::CmdType cmd, const QString &id, const QVariantMap &param)
{
    switch (cmd) {
    case NetManager::SetConnectInfo: {
        QVariantMap tmpParam = param;
        if (param.contains("ipv4")) {
            QVariantMap ipData = param.value("ipv4").toMap();
            if (ipData.contains("addresses")) {
                const QVariantList &addressData = ipData.value("addresses").toList();
                QList<QList<uint>> addressesData;
                for (auto it : addressData) {
                    QList<uint> address;
                    for (auto num : it.toList()) {
                        address.append(num.toUInt());
                    }
                    addressesData.append(address);
                }
                ipData["addresses"] = QVariant::fromValue(addressesData);
            }
            if (ipData.contains("dns")) {
                const QVariantList &dnsData = ipData.value("dns").toList();
                QList<uint> dns;
                for (auto it : dnsData) {
                    dns.append(it.toUInt());
                }
                ipData["dns"] = QVariant::fromValue(dns);
            }
            tmpParam["ipv4"] = QVariant::fromValue(ipData);
        }
        if (param.contains("ipv6") && param.value("ipv6").toMap().contains("address-data")) {
            const QVariantList &addressData = param.value("ipv6").toMap().value("address-data").toList();
            QString gatewayStr = param.value("ipv6").toMap().value("gateway").toString();
            IpV6DBusAddressList ipv6AddressList;
            for (auto it : addressData) {
                IpV6DBusAddress ipv6Address;
                QVariantMap ipv6Data = it.toMap();
                QHostAddress ip(ipv6Data.value("address").toString());
                QIPv6Address ipv6Addr = ip.toIPv6Address();
                QByteArray tmpAddress((char *)(ipv6Addr.c), 16);
                ipv6Address.address = tmpAddress;
                ipv6Address.prefix = ipv6Data.value("prefix").toUInt();
                QHostAddress gateway(ipv6AddressList.isEmpty() ? gatewayStr : QString());
                QByteArray tmpGateway((char *)(gateway.toIPv6Address().c), 16);
                ipv6Address.gateway = tmpGateway;
                ipv6AddressList.append(ipv6Address);
            }
            QVariantMap ipData = param.value("ipv6").toMap();
            ipData["addresses"] = QVariant::fromValue(ipv6AddressList);
            tmpParam["ipv6"] = QVariant::fromValue(ipData);
        }
        m_manager->exec(cmd, id, tmpParam);
    } break;
    default:
        m_manager->exec(cmd, id, param);
        break;
    }
}

void OceanUINetwork::setClipboard(const QString &text)
{
    QGuiApplication::clipboard()->setText(text);
}

QVariantMap OceanUINetwork::toMap(QMap<QString, QString> map)
{
    QVariantMap retMap;
    for (auto it = map.cbegin(); it != map.cend(); it++) {
        retMap.insert(it.key(), it.value());
    }
    return retMap;
}

QMap<QString, QString> OceanUINetwork::toStringMap(QVariantMap map)
{
    QMap<QString, QString> retMap;
    for (auto it = map.cbegin(); it != map.cend(); it++) {
        retMap.insert(it.key(), it.value().toString());
    }
    return retMap;
}

void OceanUINetwork::init()
{
    m_manager = new NetManager(NetType::Net_OceanUIFlags, this);
    m_manager->updateLanguage(QLocale().name());
    connect(m_manager, &NetManager::request, this, &OceanUINetwork::request);
    Q_EMIT managerChanged(m_manager);
    Q_EMIT rootChanged();
}
DCC_FACTORY_CLASS(OceanUINetwork)
} // namespace network
} // namespace ocean

#include "oceanuinetwork.moc"
