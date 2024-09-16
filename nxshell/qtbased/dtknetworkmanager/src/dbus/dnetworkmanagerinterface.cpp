// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dnetworkmanagerinterface.h"
#include <QDBusMetaType>

DNETWORKMANAGER_BEGIN_NAMESPACE

DNetworkManagerInterface::DNetworkManagerInterface(QObject *parent)
    : QObject(parent)
{
#ifdef USE_FAKE_INTERFACE
    const QString &Service = QStringLiteral("com.deepin.FakeNetworkManager");
    const QString &Path = QStringLiteral("/com/deepin/FakeNetworkManager");
    const QString &Interface = QStringLiteral("com.deepin.FakeNetworkManager");
    QDBusConnection Connection = QDBusConnection::sessionBus();
#else
    const QString &Service = QStringLiteral("org.freedesktop.NetworkManager");
    const QString &Path = QStringLiteral("/org/freedesktop/NetworkManager");
    const QString &Interface = QStringLiteral("org.freedesktop.NetworkManager");
    QDBusConnection Connection = QDBusConnection::systemBus();
    Connection.connect(Service, Path, Interface, "DeviceAdded", this, SIGNAL(DeviceAdded(const QDBusObjectPath &)));
    Connection.connect(Service, Path, Interface, "DeviceRemoved", this, SIGNAL(DeviceRemoved(const QDBusObjectPath &)));
    Connection.connect(Service, Path, Interface, "CheckPermissions", this, SIGNAL(CheckPermissions()));
    Connection.connect(Service, Path, Interface, "StateChanged", this, SIGNAL(StateChanged(quint32)));

#endif
    m_inter = new DDBusInterface(Service, Path, Interface, Connection, this);
    qRegisterMetaType<SettingDesc>("SettingDesc");
    qDBusRegisterMetaType<SettingDesc>();
    qDBusRegisterMetaType<QMap<QString, QString>>();
}

bool DNetworkManagerInterface::networkingEnabled() const
{
    return qdbus_cast<bool>(m_inter->property("NetworkingEnabled"));
}

bool DNetworkManagerInterface::wirelessEnabled() const
{
    return qdbus_cast<bool>(m_inter->property("WirelessEnabled"));
}

bool DNetworkManagerInterface::wirelessHardwareEnabled() const
{
    return qdbus_cast<bool>(m_inter->property("WirelessHardwareEnabled"));
}

void DNetworkManagerInterface::setWirelessEnabled(const bool enable) const
{
    m_inter->setProperty("WirelessEnabled", enable);
}

QList<QDBusObjectPath> DNetworkManagerInterface::activeConnections() const
{
    return qdbus_cast<QList<QDBusObjectPath>>(m_inter->property("ActiveConnections"));
}

QDBusObjectPath DNetworkManagerInterface::primaryConnection() const
{
    return qdbus_cast<QDBusObjectPath>(m_inter->property("PrimaryConnection"));
}

QString DNetworkManagerInterface::primaryConnectionType() const
{
    return qdbus_cast<QString>(m_inter->property("PrimaryConnectionType"));
}

quint32 DNetworkManagerInterface::state() const
{
    return qdbus_cast<quint32>(m_inter->property("State"));
}

quint32 DNetworkManagerInterface::connectivity() const
{
    return qdbus_cast<quint32>(m_inter->property("Connectivity"));
}

QDBusPendingReply<QList<QDBusObjectPath>> DNetworkManagerInterface::getDevices()
{
    return m_inter->asyncCall("GetDevices");
}

QDBusPendingReply<QDBusObjectPath>
DNetworkManagerInterface::activateConnection(const QByteArray &connection, const QByteArray &device, const QByteArray &specObj)
{
    return m_inter->asyncCallWithArgumentList("ActivateConnection",
                                              {QVariant::fromValue(QDBusObjectPath(QString(connection))),
                                               QVariant::fromValue(QDBusObjectPath(QString(device))),
                                               QVariant::fromValue(QDBusObjectPath(QString(specObj)))});
}

QDBusPendingReply<QDBusObjectPath, QDBusObjectPath> DNetworkManagerInterface::addAndActivateConnection(
    const SettingDesc &connection, const QByteArray &device, const QByteArray &specObj)
{
    return m_inter->asyncCallWithArgumentList("AddAndActivateConnection",
                                              {QVariant::fromValue(connection),
                                               QVariant::fromValue(QDBusObjectPath(QString(device))),
                                               QVariant::fromValue(QDBusObjectPath(QString(specObj)))});
}

QDBusPendingReply<void> DNetworkManagerInterface::deactivateConnection(const QByteArray &activateConnection)
{
    return m_inter->asyncCallWithArgumentList("DeactivateConnection",
                                              {QVariant::fromValue(QDBusObjectPath(QString(activateConnection)))});
}

QDBusPendingReply<void> DNetworkManagerInterface::enable(const bool enable)
{
    return m_inter->asyncCallWithArgumentList("Enable", {QVariant::fromValue(enable)});
}

QDBusPendingReply<QMap<QString, QString>> DNetworkManagerInterface::getPermissions()
{
    return m_inter->asyncCall("GetPermissions");
}

QDBusPendingReply<quint32> DNetworkManagerInterface::checkConnectivity()
{
    return m_inter->asyncCall("CheckConnectivity");
}

DNETWORKMANAGER_END_NAMESPACE
