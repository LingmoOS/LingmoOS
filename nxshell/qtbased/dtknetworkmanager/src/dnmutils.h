// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DNMUTILS_H
#define DNMUTILS_H

#include "dnetworkmanager_global.h"
#include "dnetworkmanagertypes.h"
#include <QDBusObjectPath>
#include <QHostAddress>

DNETWORKMANAGER_BEGIN_NAMESPACE

quint64 getIdFromObjectPath(const QDBusObjectPath &path);

const char *enumToProxyType(const ProxyType type);

ProxyType proxyTypeToEnum(const QString &type);

SystemProxyMethod proxyMethodToEnum(const QString &method);

const char *enumToProxyMethod(const SystemProxyMethod method);

QString macAddressToString(const QByteArray &ba);

QByteArray stringToMacAddress(const QString &s);

QHostAddress ipv6AddressToHostAddress(const QByteArray &address);

QByteArray HostAddressToipv6Address(const QHostAddress &address);

DNETWORKMANAGER_END_NAMESPACE

#endif
