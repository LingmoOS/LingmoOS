// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "login1seatinterface.h"

#include "ddbusinterface.h"
#include "dlogintypes_p.h"

#include <qdbusargument.h>
#include <qdbuspendingreply.h>

DLOGIN_BEGIN_NAMESPACE
Login1SeatInterface::Login1SeatInterface(const QString &service,
                                         const QString &path,
                                         const QDBusConnection &connection,
                                         QObject *parent)
    : QObject(parent)
    , m_interface(new DDBusInterface(service, path, staticInterfaceName(), connection, this))
    , m_path(path)
{
    DBusSessionPath::registerMetaType();
}

Login1SeatInterface::~Login1SeatInterface() = default;

bool Login1SeatInterface::canGraphical() const
{
    return qdbus_cast<bool>(m_interface->property("CanGraphical"));
}

bool Login1SeatInterface::canTTY() const
{
    return qdbus_cast<bool>(m_interface->property("CanTTY"));
}

bool Login1SeatInterface::idleHint() const
{
    return qdbus_cast<bool>(m_interface->property("IdleHint"));
}

QList<DBusSessionPath> Login1SeatInterface::sessions() const
{
    return qdbus_cast<QList<DBusSessionPath>>(m_interface->property("Sessions"));
}

QString Login1SeatInterface::id() const
{
    return qdbus_cast<QString>(m_interface->property("Id"));
}

DBusSessionPath Login1SeatInterface::activeSession() const
{
    return qdbus_cast<DBusSessionPath>(m_interface->property("ActiveSession"));
}

quint64 Login1SeatInterface::idleSinceHint() const
{
    return qdbus_cast<quint64>(m_interface->property("IdleSinceHint"));
}

quint64 Login1SeatInterface::idleSinceHintMonotonic() const
{
    return qdbus_cast<quint64>(m_interface->property("IdleSinceHintMonotonic"));
}

QDBusPendingReply<> Login1SeatInterface::activateSession(const QString &sessionId)
{
    QDBusPendingReply<> reply =
            m_interface->asyncCallWithArgumentList("ActivateSession",
                                                   { QVariant::fromValue(sessionId) });
    return reply;
}

QDBusPendingReply<> Login1SeatInterface::switchTo(const quint32 VTNr)
{
    QDBusPendingReply<> reply =
            m_interface->asyncCallWithArgumentList(QStringLiteral("SwitchTo"),
                                                   { QVariant::fromValue(VTNr) });
    return reply;
}

QDBusPendingReply<> Login1SeatInterface::switchToNext()
{
    QDBusPendingReply<> reply = m_interface->asyncCall(QStringLiteral("SwitchToNext"));
    return reply;
}

QDBusPendingReply<> Login1SeatInterface::switchToPrevious()
{
    QDBusPendingReply<> reply = m_interface->asyncCall(QStringLiteral("SwitchToPrevious"));
    return reply;
}

QDBusPendingReply<> Login1SeatInterface::terminate()
{
    QDBusPendingReply<> reply = m_interface->asyncCall(QStringLiteral("Terminate"));
    return reply;
}

DLOGIN_END_NAMESPACE