// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dnmutils.h"

DNETWORKMANAGER_BEGIN_NAMESPACE

quint64 getIdFromObjectPath(const QDBusObjectPath &path)
{
    const auto &list = path.path().split("/");
    bool ok{false};
    auto intId = list.constLast().toLongLong(&ok);
    return ok ? intId : 0;
}

const char *enumToProxyType(const ProxyType type)
{
    switch (type) {
        case ProxyType::HTTP:
            return "http";
        case ProxyType::Socks4:
            return "socks4";
        case ProxyType::Socks5:
            return "socks5";
    }
    Q_UNREACHABLE();
}

ProxyType proxyTypeToEnum(const QString &type)
{
    if (type == "http")
        return ProxyType::HTTP;
    else if (type == "socks4")
        return ProxyType::Socks4;
    else if (type == "socks5")
        return ProxyType::Socks5;
    Q_UNREACHABLE();
}

SystemProxyMethod proxyMethodToEnum(const QString &method)
{
    if (method == "auto")
        return SystemProxyMethod::Auto;
    else if (method == "manual")
        return SystemProxyMethod::Manual;
    else if (method == "none")
        return SystemProxyMethod::None;
    Q_UNREACHABLE();
}

const char *enumToProxyMethod(const SystemProxyMethod method)
{
    switch (method) {
        case SystemProxyMethod::Auto:
            return "auto";
        case SystemProxyMethod::Manual:
            return "manual";
        case SystemProxyMethod::None:
            return "none";
    }
    Q_UNREACHABLE();
}

QString macAddressToString(const QByteArray &ba)
{
    QStringList mac;
    for (auto i : ba)
        mac << QString("%1").arg((quint8)i, 2, 16, QLatin1Char('0')).toUpper();
    return mac.join(":");
}

QByteArray stringToMacAddress(const QString &s)
{
    const QStringList macStringList = s.split(':');
    QByteArray ba;
    if (!s.isEmpty()) {
        ba.resize(6);
        int i = 0;

        for (const QString &macPart : macStringList) {
            ba[i++] = static_cast<char>(macPart.toUInt(nullptr, 16));
        }
    }
    return ba;
}

QHostAddress ipv6AddressToHostAddress(const QByteArray &address)
{
    Q_IPV6ADDR tmp;
    for (int i = 0; i < 16; ++i)
        tmp[i] = address[i];
    QHostAddress hostaddress(tmp);
    Q_ASSERT(hostaddress.protocol() == QAbstractSocket::IPv6Protocol);

    return hostaddress;
}

QByteArray HostAddressToipv6Address(const QHostAddress &address)
{
    Q_IPV6ADDR tmp = address.toIPv6Address();
    QByteArray assembledAddress;
    assembledAddress.reserve(16);
    for (int i = 0; i < 16; ++i)
        assembledAddress.push_back(static_cast<char>(tmp[i]));

    return assembledAddress;
}

DNETWORKMANAGER_END_NAMESPACE
