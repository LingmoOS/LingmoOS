// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dactiveconnection_p.h"
#include "dnmutils.h"

DNETWORKMANAGER_BEGIN_NAMESPACE

DActiveConnectionPrivate::DActiveConnectionPrivate(const quint64 activeConnId, DActiveConnection *parent)
    : q_ptr(parent)
#ifdef USE_FAKE_INTERFACE
    , m_activeConn(new DActiveConnectionInterface(
          "/com/deepin/FakeNetworkManager/ActiveConnection/" + QByteArray::number(activeConnId), this))
#else
    , m_activeConn(new DActiveConnectionInterface(
          "/org/freedesktop/NetworkManager/ActiveConnection/" + QByteArray::number(activeConnId), this))
#endif
{
}

DActiveConnectionPrivate::~DActiveConnectionPrivate() = default;

DActiveConnection::DActiveConnection(const quint64 activeConnId, QObject *parent)
    : QObject(parent)
    , d_ptr(new DActiveConnectionPrivate(activeConnId, this))
{
    Q_D(const DActiveConnection);
    connect(d->m_activeConn, &DActiveConnectionInterface::DevicesChanged, this, [this](const QList<QDBusObjectPath> &devices) {
        QList<quint64> ret;
        for (const auto &it : devices)
            ret.append(getIdFromObjectPath(it));
        Q_EMIT this->devicesChanged(ret);
    });
    connect(d->m_activeConn, &DActiveConnectionInterface::VpnChanged, this, &DActiveConnection::vpnChanged);
    connect(d->m_activeConn, &DActiveConnectionInterface::ConnectionChanged, this, [this](const QDBusObjectPath &connection) {
        Q_EMIT this->connectionChanged(getIdFromObjectPath(connection));
    });
    connect(d->m_activeConn, &DActiveConnectionInterface::Dhcp4ConfigChanged, this, [this](const QDBusObjectPath &config) {
        Q_EMIT this->DHCP4ConfigChanged(getIdFromObjectPath(config));
    });
    connect(d->m_activeConn, &DActiveConnectionInterface::Dhcp6ConfigChanged, this, [this](const QDBusObjectPath &config) {
        Q_EMIT this->DHCP6ConfigChanged(getIdFromObjectPath(config));
    });
    connect(d->m_activeConn, &DActiveConnectionInterface::IP4ConfigChanged, this, [this](const QDBusObjectPath &config) {
        Q_EMIT this->IP4ConfigChanged(getIdFromObjectPath(config));
    });
    connect(d->m_activeConn, &DActiveConnectionInterface::IP6ConfigChanged, this, [this](const QDBusObjectPath &config) {
        Q_EMIT this->IP6ConfigChanged(getIdFromObjectPath(config));
    });
    connect(d->m_activeConn, &DActiveConnectionInterface::SpecificObjectChanged, this, [this](const QDBusObjectPath &specObj) {
        Q_EMIT this->specificObjectChanged(getIdFromObjectPath(specObj));
    });
    connect(d->m_activeConn, &DActiveConnectionInterface::IdChanged, this, &DActiveConnection::connectionIdChanged);
    connect(d->m_activeConn, &DActiveConnectionInterface::UuidChanged, this, [this](const QString &uuid) {
        Q_EMIT this->UUIDChanged(QUuid{uuid.toUtf8()});
    });
    connect(d->m_activeConn, &DActiveConnectionInterface::TypeChanged, this, [this](const QString &type) {
        Q_EMIT this->connectionTypeChanged(DNMSetting::stringToType(type));
    });
    connect(d->m_activeConn, &DActiveConnectionInterface::StateChanged, this, [this](const quint32 state, const quint32 reason) {
        Q_EMIT this->connectionStateChanged(static_cast<NMActiveConnectionState>(state),
                                            static_cast<NMActiveConnectionStateReason>(reason));
    });
}

DActiveConnection::DActiveConnection(DActiveConnectionPrivate &other, QObject *parent)
    : QObject(parent)
    , d_ptr(&other)
{
    Q_D(DActiveConnection);
    connect(d->m_activeConn, &DActiveConnectionInterface::DevicesChanged, this, [this](const QList<QDBusObjectPath> &devices) {
        QList<quint64> ret;
        for (const auto &it : devices)
            ret.append(getIdFromObjectPath(it));
        Q_EMIT this->devicesChanged(ret);
    });
    connect(d->m_activeConn, &DActiveConnectionInterface::VpnChanged, this, &DActiveConnection::vpnChanged);
    connect(d->m_activeConn, &DActiveConnectionInterface::ConnectionChanged, this, [this](const QDBusObjectPath &connection) {
        Q_EMIT this->connectionChanged(getIdFromObjectPath(connection));
    });
    connect(d->m_activeConn, &DActiveConnectionInterface::Dhcp4ConfigChanged, this, [this](const QDBusObjectPath &config) {
        Q_EMIT this->DHCP4ConfigChanged(getIdFromObjectPath(config));
    });
    connect(d->m_activeConn, &DActiveConnectionInterface::Dhcp6ConfigChanged, this, [this](const QDBusObjectPath &config) {
        Q_EMIT this->DHCP6ConfigChanged(getIdFromObjectPath(config));
    });
    connect(d->m_activeConn, &DActiveConnectionInterface::IP4ConfigChanged, this, [this](const QDBusObjectPath &config) {
        Q_EMIT this->IP4ConfigChanged(getIdFromObjectPath(config));
    });
    connect(d->m_activeConn, &DActiveConnectionInterface::IP6ConfigChanged, this, [this](const QDBusObjectPath &config) {
        Q_EMIT this->IP6ConfigChanged(getIdFromObjectPath(config));
    });
    connect(d->m_activeConn, &DActiveConnectionInterface::SpecificObjectChanged, this, [this](const QDBusObjectPath &specObj) {
        Q_EMIT this->specificObjectChanged(getIdFromObjectPath(specObj));
    });
    connect(d->m_activeConn, &DActiveConnectionInterface::IdChanged, this, &DActiveConnection::connectionIdChanged);
    connect(d->m_activeConn, &DActiveConnectionInterface::UuidChanged, this, [this](const QString &uuid) {
        Q_EMIT this->UUIDChanged(QUuid{uuid});
    });
    connect(d->m_activeConn, &DActiveConnectionInterface::TypeChanged, this, [this](const QString &type) {
        Q_EMIT this->connectionTypeChanged(DNMSetting::stringToType(type));
    });
    connect(d->m_activeConn, &DActiveConnectionInterface::StateChanged, this, [this](const quint32 state, const quint32 reason) {
        Q_EMIT this->connectionStateChanged(static_cast<NMActiveConnectionState>(state),
                                            static_cast<NMActiveConnectionStateReason>(reason));
    });
}

DActiveConnection::~DActiveConnection() = default;

QList<quint64> DActiveConnection::devices() const
{
    Q_D(const DActiveConnection);
    QList<quint64> ret;
    for (const auto &it : d->m_activeConn->devices())
        ret.append(getIdFromObjectPath(it));
    return ret;
}

bool DActiveConnection::vpn() const
{
    Q_D(const DActiveConnection);
    return d->m_activeConn->vpn();
}

quint64 DActiveConnection::connection() const
{
    Q_D(const DActiveConnection);
    return getIdFromObjectPath(d->m_activeConn->connection());
}

quint64 DActiveConnection::DHCP4Config() const
{
    Q_D(const DActiveConnection);
    return getIdFromObjectPath(d->m_activeConn->DHCP4Config());
}

quint64 DActiveConnection::DHCP6Config() const
{
    Q_D(const DActiveConnection);
    return getIdFromObjectPath(d->m_activeConn->DHCP6Config());
}

quint64 DActiveConnection::IP4Config() const
{
    Q_D(const DActiveConnection);
    return getIdFromObjectPath(d->m_activeConn->IP4Config());
}

quint64 DActiveConnection::IP6Config() const
{
    Q_D(const DActiveConnection);
    return getIdFromObjectPath(d->m_activeConn->IP6Config());
}

quint64 DActiveConnection::specificObject() const
{
    Q_D(const DActiveConnection);
    return getIdFromObjectPath(d->m_activeConn->specificObject());
}

QString DActiveConnection::connectionId() const
{
    Q_D(const DActiveConnection);
    return d->m_activeConn->id();
}

DNMSetting::SettingType DActiveConnection::connectionType() const
{
    Q_D(const DActiveConnection);
    return DNMSetting::stringToType(d->m_activeConn->type());
}

QUuid DActiveConnection::UUID() const
{
    Q_D(const DActiveConnection);
    return QUuid{d->m_activeConn->uuid()};
}

NMActiveConnectionState DActiveConnection::connectionState() const
{
    Q_D(const DActiveConnection);
    return static_cast<NMActiveConnectionState>(d->m_activeConn->state());
}

DNETWORKMANAGER_END_NAMESPACE
