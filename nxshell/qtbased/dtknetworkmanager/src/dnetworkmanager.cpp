// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dnetworkmanager_p.h"
#include "dnmutils.h"
#include "dadsldevice.h"
#include "dgenericdevice.h"
#include "dwireddevice.h"
#include "dwirelessdevice.h"
#include "dtundevice.h"
#include "dactivevpnconnection.h"
#include <QDBusMessage>
#include <QDebug>

DNETWORKMANAGER_BEGIN_NAMESPACE

using DCORE_NAMESPACE::DUnexpected;
using DCORE_NAMESPACE::emplace_tag;

DNetworkManagerPrivate::DNetworkManagerPrivate(DNetworkManager *parent)
    : q_ptr(parent)
    , m_manager(new DNetworkManagerInterface(this))
{
}

DNetworkManager::DNetworkManager(QObject *parent)
    : QObject(parent)
    , d_ptr(new DNetworkManagerPrivate(this))
{
    Q_D(const DNetworkManager);

    connect(d->m_manager, &DNetworkManagerInterface::NetworkingEnabledChanged, this, &DNetworkManager::networkingEnabledChanged);

    connect(d->m_manager, &DNetworkManagerInterface::WirelessEnabledChanged, this, &DNetworkManager::wirelessEnabledChanged);

    connect(d->m_manager,
            &DNetworkManagerInterface::WirelessHardwareEnabledChanged,
            this,
            &DNetworkManager::wirelessHardwareEnabledChanged);

    connect(d->m_manager, &DNetworkManagerInterface::ActiveConnectionsChanged, this, [this](const QList<QDBusObjectPath> &conn) {
        QList<quint64> ret;
        for (const auto &it : conn)
            ret.append(getIdFromObjectPath(it));
        Q_EMIT this->activeConnectionsChanged(ret);
    });

    connect(d->m_manager, &DNetworkManagerInterface::PrimaryConnectionChanged, this, [this](const QDBusObjectPath &conn) {
        Q_EMIT this->primaryConnectionChanged(getIdFromObjectPath(conn));
    });

    connect(d->m_manager, &DNetworkManagerInterface::PrimaryConnectionTypeChanged, this, [this](const QString &type) {
        Q_EMIT this->primaryConnectionTypeChanged(DNMSetting::stringToType(type));
    });

    connect(d->m_manager, &DNetworkManagerInterface::ConnectivityChanged, this, [this](const quint32 con) {
        Q_EMIT this->connectivityChanged(static_cast<NMConnectivityState>(con));
    });

    connect(d->m_manager, &DNetworkManagerInterface::DeviceAdded, this, [this](const QDBusObjectPath &device) {
        qDebug() << "dnetworkmanager.cpp";
        Q_EMIT this->DeviceAdded(getIdFromObjectPath(device));
    });

    connect(d->m_manager, &DNetworkManagerInterface::DeviceRemoved, this, [this](const QDBusObjectPath &device) {
        Q_EMIT this->DeviceRemoved(getIdFromObjectPath(device));
    });

    connect(d->m_manager, &DNetworkManagerInterface::CheckPermissions, this, &DNetworkManager::CheckPermissions);

    connect(d->m_manager, &DNetworkManagerInterface::StateChanged, this, [this](const quint32 state) {
        Q_EMIT this->StateChanged(static_cast<NMState>(state));
    });
}

DNetworkManager::~DNetworkManager() = default;

bool DNetworkManager::networkingEnabled() const
{
    Q_D(const DNetworkManager);
    return d->m_manager->networkingEnabled();
}

bool DNetworkManager::wirelessEnabled() const
{
    Q_D(const DNetworkManager);
    return d->m_manager->wirelessEnabled();
}

void DNetworkManager::setWirelessEnabled(const bool enable) const
{
    Q_D(const DNetworkManager);
    d->m_manager->setWirelessEnabled(enable);
}

bool DNetworkManager::wirelessHardwareEnabled() const
{
    Q_D(const DNetworkManager);
    return d->m_manager->wirelessHardwareEnabled();
}

QList<quint64> DNetworkManager::getActiveConnectionsIdList() const
{
    Q_D(const DNetworkManager);
    QList<quint64> ret;
    for (const auto &conn : d->m_manager->activeConnections())
        ret.append(getIdFromObjectPath(conn));
    return ret;
}

quint64 DNetworkManager::primaryConnection() const
{
    Q_D(const DNetworkManager);
    return getIdFromObjectPath(d->m_manager->primaryConnection());
}

DNMSetting::SettingType DNetworkManager::primaryConnectionType() const
{
    Q_D(const DNetworkManager);

    return DNMSetting::stringToType(d->m_manager->primaryConnectionType());
}

NMState DNetworkManager::state() const
{
    Q_D(const DNetworkManager);
    return static_cast<NMState>(d->m_manager->state());
}

NMConnectivityState DNetworkManager::connectivity() const
{
    Q_D(const DNetworkManager);
    return static_cast<NMConnectivityState>(d->m_manager->connectivity());
}

DExpected<QList<quint64>> DNetworkManager::getDeviceIdList() const
{
    Q_D(const DNetworkManager);
    auto reply = d->m_manager->getDevices();
    reply.waitForFinished();
    if (!reply.isValid())
        return DUnexpected{emplace_tag::USE_EMPLACE, reply.error().type(), reply.error().message()};
    QList<quint64> ret;
    for (const auto &it : reply.value())
        ret.append(getIdFromObjectPath(it));
    return ret;
}

DExpected<quint64> DNetworkManager::activateConnection(const quint64 connId, const quint64 deviceId, const qint64 objId) const
{
    Q_D(const DNetworkManager);
    QByteArray objPath{"/"};

    if (objId != 0) {
        auto type = DNetworkManagerPrivate::getSettingTypeFromConnId(connId);
        if (!type)
            return DUnexpected{std::move(type).error()};
        objPath = DNetworkManagerPrivate::getObjPath(type.value()) + QByteArray::number(objId);
    }

    auto reply = d->m_manager->activateConnection("/org/freedesktop/NetworkManager/Settings/" + QByteArray::number(connId),
                                                  "/org/freedesktop/NetworkManager/Devices/" + QByteArray::number(deviceId),
                                                  objPath);
    reply.waitForFinished();
    if (!reply.isValid())
        return DUnexpected{emplace_tag::USE_EMPLACE, reply.error().type(), reply.error().message()};
    return getIdFromObjectPath(reply.value());
}

DExpected<NewConn>
DNetworkManager::addAndActivateConnection(const SettingDesc &conn, const quint64 deviceId, const qint64 objId) const
{
    Q_D(const DNetworkManager);
    QByteArray objPath{"/"};

    if (objId != 0) {
        auto type = DNetworkManagerPrivate::getSettingTypeFromMap(conn);
        if (!type)
            return DUnexpected{std::move(type).error()};
        objPath = DNetworkManagerPrivate::getObjPath(type.value()) + QByteArray::number(objId);
    }

    auto reply = d->m_manager->addAndActivateConnection(
        conn, "/org/freedesktop/NetworkManager/Devices/" + QByteArray::number(deviceId), objPath);
    reply.waitForFinished();
    if (!reply.isValid())
        return DUnexpected{emplace_tag::USE_EMPLACE, reply.error().type(), reply.error().message()};
    return NewConn{getIdFromObjectPath(reply.argumentAt<0>()), getIdFromObjectPath(reply.argumentAt<1>())};
}

DExpected<void> DNetworkManager::deactivateConnection(const quint64 activeConnId) const
{
    Q_D(const DNetworkManager);
    auto reply = d->m_manager->deactivateConnection("/org/freedesktop/NetworkManager/ActiveConnection/" +
                                                    QByteArray::number(activeConnId));
    reply.waitForFinished();
    if (!reply.isValid())
        return DUnexpected{emplace_tag::USE_EMPLACE, reply.error().type(), reply.error().message()};
    return {};
}

DExpected<void> DNetworkManager::enable(const bool enabled) const
{
    Q_D(const DNetworkManager);
    auto reply = d->m_manager->enable(enabled);
    reply.waitForFinished();
    if (!reply.isValid())
        return DUnexpected{emplace_tag::USE_EMPLACE, reply.error().type(), reply.error().message()};
    return {};
}

DExpected<QMap<QString, QString>> DNetworkManager::permissions() const
{
    Q_D(const DNetworkManager);
    auto reply = d->m_manager->getPermissions();
    reply.waitForFinished();
    if (!reply.isValid())
        return DUnexpected{emplace_tag::USE_EMPLACE, reply.error().type(), reply.error().message()};
    return reply.value();
}

DExpected<NMConnectivityState> DNetworkManager::checkConnectivity() const
{
    Q_D(const DNetworkManager);
    auto reply = d->m_manager->checkConnectivity();
    reply.waitForFinished();
    if (!reply.isValid())
        return DUnexpected{emplace_tag::USE_EMPLACE, reply.error().type(), reply.error().message()};
    return static_cast<NMConnectivityState>(reply.value());
}

DExpected<QSharedPointer<DDevice>> DNetworkManager::getDeviceObject(const quint64 id) const
{
    const QString &Service = QStringLiteral("org.freedesktop.NetworkManager");
    const QString &Path = QStringLiteral("/org/freedesktop/NetworkManager/Devices/");
    const QString &Interface = QStringLiteral("org.freedesktop.DBus.Properties");
    auto msg = QDBusMessage::createMethodCall(Service, Path + QString::number(id), Interface, "Get");
    msg << "org.freedesktop.NetworkManager.Device"
        << "DeviceType";
    QDBusPendingReply<QVariant> type = QDBusConnection::systemBus().asyncCall(msg);
    NMDeviceType realType;
    if (type.value().isValid())
        realType = static_cast<NMDeviceType>(qdbus_cast<quint32>(type));
    else
        return DUnexpected{emplace_tag::USE_EMPLACE, type.error().type(), type.error().message()};
    switch (realType) {
        case NMDeviceType::ADSL:
            return QSharedPointer<DAdslDevice>(new DAdslDevice(id));
        case NMDeviceType::Generic:
            return QSharedPointer<DGenericDevice>(new DGenericDevice(id));
        case NMDeviceType::Tun:
            return QSharedPointer<DTunDevice>(new DTunDevice(id));
        case NMDeviceType::Ethernet:
            return QSharedPointer<DWiredDevice>(new DWiredDevice(id));
        case NMDeviceType::WiFi:
            return QSharedPointer<DWirelessDevice>(new DWirelessDevice(id));
        default:
            return DUnexpected{emplace_tag::USE_EMPLACE, -1, "unsupported device"};
    }
    Q_UNREACHABLE();
}

DExpected<QSharedPointer<DActiveConnection>> DNetworkManager::getActiveConnectionObject(const quint64 id) const
{
    const QString &Service = QStringLiteral("org.freedesktop.NetworkManager");
    const QString &Path = QStringLiteral("/org/freedesktop/NetworkManager/ActiveConnection/");
    const QString &Interface = QStringLiteral("org.freedesktop.DBus.Properties");
    auto msg = QDBusMessage::createMethodCall(Service, Path + QString::number(id), Interface, "Get");
    msg << "org.freedesktop.NetworkManager.Connection.Active"
        << "Vpn";
    QDBusPendingReply<QVariant> type = QDBusConnection::systemBus().asyncCall(msg);
    bool isVpn{false};
    if (type.value().isValid())
        isVpn = qdbus_cast<bool>(type);
    else
        return DUnexpected{emplace_tag::USE_EMPLACE, type.error().type(), type.error().message()};
    if (isVpn)
        return QSharedPointer<DActiveConnection>(new DActiveVpnConnection(id));
    return QSharedPointer<DActiveConnection>(new DActiveConnection(id));
}

DNETWORKMANAGER_END_NAMESPACE
