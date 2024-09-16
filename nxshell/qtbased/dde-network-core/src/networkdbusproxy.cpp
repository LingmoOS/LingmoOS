// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "networkdbusproxy.h"
#include "dccdbusinterface.h"

#include <QMetaObject>
#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusPendingReply>
#include <QDebug>

const QString NetworkService = QStringLiteral("org.deepin.dde.Network1");
const QString NetworkPath = QStringLiteral("/org/deepin/dde/Network1");
const QString NetworkInterface = QStringLiteral("org.deepin.dde.Network1");

const QString ProxyChainsService = QStringLiteral("org.deepin.dde.Network1");
const QString ProxyChainsPath = QStringLiteral("/org/deepin/dde/Network1/ProxyChains");
const QString ProxyChainsInterface = QStringLiteral("org.deepin.dde.Network1.ProxyChains");

const QString AirplaneModeService = QStringLiteral("org.deepin.dde.AirplaneMode1");
const QString AirplaneModePath = QStringLiteral("/org/deepin/dde/AirplaneMode1");
const QString AirplaneModeInterface = QStringLiteral("org.deepin.dde.AirplaneMode1");

const QString PropertiesInterface = QStringLiteral("org.freedesktop.DBus.Properties");
const QString PropertiesChanged = QStringLiteral("PropertiesChanged");
using namespace dde::network;

NetworkDBusProxy::NetworkDBusProxy(QObject *parent)
    : QObject(parent)
    , m_networkInter(new DCCDBusInterface(NetworkService, NetworkPath, NetworkInterface, QDBusConnection::sessionBus(), this))
    , m_proxyChainsInter(new DCCDBusInterface(ProxyChainsService, ProxyChainsPath, ProxyChainsInterface, QDBusConnection::sessionBus(), this))
    , m_airplaneModeInter(new DCCDBusInterface(AirplaneModeService, AirplaneModePath, AirplaneModeInterface, QDBusConnection::systemBus(), this))
{
}

// networkInter property
QString NetworkDBusProxy::activeConnections()
{
    return qvariant_cast<QString>(m_networkInter->property("ActiveConnections"));
}

QString NetworkDBusProxy::connections()
{
    return qvariant_cast<QString>(m_networkInter->property("Connections"));
}

uint NetworkDBusProxy::connectivity()
{
    return qvariant_cast<uint>(m_networkInter->property("Connectivity"));
}

QString NetworkDBusProxy::devices()
{
    return qvariant_cast<QString>(m_networkInter->property("Devices"));
}

bool NetworkDBusProxy::networkingEnabled()
{
    return qvariant_cast<bool>(m_networkInter->property("NetworkingEnabled"));
}
void NetworkDBusProxy::setNetworkingEnabled(bool value)
{
    m_networkInter->setProperty("NetworkingEnabled", QVariant::fromValue(value));
}

uint NetworkDBusProxy::state()
{
    return qvariant_cast<uint>(m_networkInter->property("State"));
}

bool NetworkDBusProxy::vpnEnabled()
{
    return qvariant_cast<bool>(m_networkInter->property("VpnEnabled"));
}
void NetworkDBusProxy::setVpnEnabled(bool value)
{
    m_networkInter->setProperty("VpnEnabled", QVariant::fromValue(value));
}

QString NetworkDBusProxy::wirelessAccessPoints()
{
    return qvariant_cast<QString>(m_networkInter->property("WirelessAccessPoints"));
}
// proxyChains property
QString NetworkDBusProxy::iP()
{
    return qvariant_cast<QString>(m_proxyChainsInter->property("IP"));
}

QString NetworkDBusProxy::password()
{
    return qvariant_cast<QString>(m_proxyChainsInter->property("Password"));
}

uint NetworkDBusProxy::port()
{
    return qvariant_cast<uint>(m_proxyChainsInter->property("Port"));
}

QString NetworkDBusProxy::type()
{
    return qvariant_cast<QString>(m_proxyChainsInter->property("Type"));
}

QString NetworkDBusProxy::user()
{
    return qvariant_cast<QString>(m_proxyChainsInter->property("User"));
}

bool NetworkDBusProxy::enabled()
{
    return qvariant_cast<bool>(m_airplaneModeInter->property("Enabled"));
}

void NetworkDBusProxy::ShowPage(const QString &url)
{
    QDBusMessage message = QDBusMessage::createMethodCall("org.deepin.dde.ControlCenter1", "/org/deepin/dde/ControlCenter1", "org.deepin.dde.ControlCenter1", "ShowPage");
    message << QVariant::fromValue(url);
    QDBusConnection::sessionBus().asyncCall(message);
}
// networkInter property
void NetworkDBusProxy::EnableDevice(const QDBusObjectPath &devPath, bool enabled)
{
    m_networkInter->asyncCall(QStringLiteral("EnableDevice"), QVariant::fromValue(devPath), QVariant::fromValue(enabled));
}

QString NetworkDBusProxy::GetProxyMethod()
{
    return QDBusPendingReply<QString>(m_networkInter->asyncCall(QStringLiteral("GetProxyMethod")));
}

void NetworkDBusProxy::SetProxyMethod(const QString &proxyMode)
{
    m_networkInter->asyncCall(QStringLiteral("SetProxyMethod"), QVariant::fromValue(proxyMode));
}

void NetworkDBusProxy::SetProxyMethod(const QString &proxyMode, QObject *receiver, const char *member)
{
    QList<QVariant> argumentList;
    argumentList << QVariant::fromValue(proxyMode);
    m_networkInter->callWithCallback(QStringLiteral("SetProxyMethod"), argumentList, receiver, member);
}

QString NetworkDBusProxy::GetProxyIgnoreHosts()
{
    return QDBusPendingReply<QString>(m_networkInter->asyncCall(QStringLiteral("GetProxyIgnoreHosts")));
}

void NetworkDBusProxy::SetProxyIgnoreHosts(const QString &ignoreHosts)
{
    m_networkInter->asyncCall(QStringLiteral("SetProxyIgnoreHosts"), QVariant::fromValue(ignoreHosts));
}

void NetworkDBusProxy::SetProxyIgnoreHosts(const QString &ignoreHosts, QObject *receiver, const char *member)
{
    QList<QVariant> argumentList;
    argumentList << QVariant::fromValue(ignoreHosts);
    m_networkInter->callWithCallback(QStringLiteral("SetProxyIgnoreHosts"), argumentList, receiver, member);
}

QString NetworkDBusProxy::GetAutoProxy()
{
    return QDBusPendingReply<QString>(m_networkInter->asyncCall(QStringLiteral("GetAutoProxy")));
}

void NetworkDBusProxy::SetAutoProxy(const QString &proxyAuto)
{
    m_networkInter->asyncCall(QStringLiteral("SetAutoProxy"), QVariant::fromValue(proxyAuto));
}

void NetworkDBusProxy::SetAutoProxy(const QString &proxyAuto, QObject *receiver, const char *member)
{
    QList<QVariant> argumentList;
    argumentList << QVariant::fromValue(proxyAuto);
    m_networkInter->callWithCallback(QStringLiteral("SetAutoProxy"), argumentList, receiver, member);
}

QStringList NetworkDBusProxy::GetProxy(const QString &proxyType)
{
    QStringList out;
    QDBusMessage reply = m_networkInter->call(QDBus::Block, QStringLiteral("GetProxy"), QVariant::fromValue(proxyType));
    if (reply.type() == QDBusMessage::ReplyMessage && reply.arguments().count() == 2) {
        out << qdbus_cast<QString>(reply.arguments().at(0));
        out << qdbus_cast<QString>(reply.arguments().at(1));
    }
    return out;
}

void NetworkDBusProxy::SetProxy(const QString &proxyType, const QString &host, const QString &port)
{
    m_networkInter->asyncCall(QStringLiteral("SetProxy"), QVariant::fromValue(proxyType), QVariant::fromValue(host), QVariant::fromValue(port));
}

void NetworkDBusProxy::SetProxy(const QString &proxyType, const QString &host, const QString &port, QObject *receiver, const char *member)
{
    QList<QVariant> argumentList;
    argumentList << QVariant::fromValue(proxyType) << QVariant::fromValue(host) << QVariant::fromValue(port);
    m_networkInter->callWithCallback(QStringLiteral("SetProxy"), argumentList, receiver, member);
}

QString NetworkDBusProxy::GetActiveConnectionInfo()
{
    return QDBusPendingReply<QString>(m_networkInter->asyncCall(QStringLiteral("GetActiveConnectionInfo")));
}

QDBusObjectPath NetworkDBusProxy::ActivateConnection(const QString &uuid, const QDBusObjectPath &devicePath)
{
    return QDBusPendingReply<QDBusObjectPath>(m_networkInter->asyncCall(QStringLiteral("ActivateConnection"), QVariant::fromValue(uuid), QVariant::fromValue(devicePath)));
}

QDBusObjectPath NetworkDBusProxy::ActivateAccessPoint(const QString &uuid, const QDBusObjectPath &apPath, const QDBusObjectPath &devPath)
{
    return QDBusPendingReply<QDBusObjectPath>(m_networkInter->asyncCall(QStringLiteral("ActivateAccessPoint"), QVariant::fromValue(uuid), QVariant::fromValue(apPath), QVariant::fromValue(devPath)));
}

bool NetworkDBusProxy::ActivateAccessPoint(const QString &uuid, const QDBusObjectPath &apPath, const QDBusObjectPath &devPath, QObject *receiver, const char *member, const char *errorSlot)
{
    QList<QVariant> argumentList;
    argumentList << QVariant::fromValue(uuid) << QVariant::fromValue(apPath) << QVariant::fromValue(devPath);
    return m_networkInter->callWithCallback(QStringLiteral("ActivateAccessPoint"), argumentList, receiver, member, errorSlot);
}

void NetworkDBusProxy::DisconnectDevice(const QDBusObjectPath &devPath)
{
    m_networkInter->asyncCall(QStringLiteral("DisconnectDevice"), QVariant::fromValue(devPath));
}

void NetworkDBusProxy::RequestIPConflictCheck(const QString &ip, const QString &ifc)
{
    m_networkInter->asyncCall(QStringLiteral("RequestIPConflictCheck"), QVariant::fromValue(ip), QVariant::fromValue(ifc));
}

bool NetworkDBusProxy::IsDeviceEnabled(const QDBusObjectPath &devPath)
{
    return QDBusPendingReply<bool>(m_networkInter->asyncCall(QStringLiteral("IsDeviceEnabled"), QVariant::fromValue(devPath)));
}

void NetworkDBusProxy::RequestWirelessScan()
{
    m_networkInter->asyncCall(QStringLiteral("RequestWirelessScan"));
}

void NetworkDBusProxy::Set(const QString &type0, const QString &ip, uint port, const QString &user, const QString &password)
{
    m_proxyChainsInter->asyncCall(QStringLiteral("Set"), QVariant::fromValue(type0), QVariant::fromValue(ip), QVariant::fromValue(port), QVariant::fromValue(user), QVariant::fromValue(password));
}

uint NetworkDBusProxy::Notify(const QString &in0, uint in1, const QString &in2, const QString &in3, const QString &in4, const QStringList &in5, const QVariantMap &in6, int in7)
{
    QDBusMessage message = QDBusMessage::createMethodCall("org.freedesktop.Notifications", "/org/freedesktop/Notifications", "org.freedesktop.Notifications", "Notify");
    message << QVariant::fromValue(in0) << QVariant::fromValue(in1) << QVariant::fromValue(in2) << QVariant::fromValue(in3) << QVariant::fromValue(in4) << QVariant::fromValue(in5) << QVariant::fromValue(in6) << QVariant::fromValue(in7);
    return QDBusPendingReply<uint>(QDBusConnection::sessionBus().asyncCall(message));
}
