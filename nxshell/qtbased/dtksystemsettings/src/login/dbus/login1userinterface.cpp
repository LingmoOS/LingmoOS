// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "login1userinterface.h"

#include "ddbusinterface.h"

DLOGIN_BEGIN_NAMESPACE
Login1UserInterface::Login1UserInterface(const QString &service,
                                         const QString &path,
                                         const QDBusConnection &connection,
                                         QObject *parent)
    : QObject(parent)
    , m_interface(new DDBusInterface(service, path, staticInterfaceName(), connection, this))
    , m_path(path)
{
    DBusSessionPath::registerMetaType();
}

Login1UserInterface::~Login1UserInterface() = default;

QList<DBusSessionPath> Login1UserInterface::sessions() const
{
    return qdbus_cast<QList<DBusSessionPath>>(m_interface->property("Sessions"));
}

bool Login1UserInterface::idleHint() const
{
    return qdbus_cast<bool>(m_interface->property("IdleHint"));
}

bool Login1UserInterface::linger() const
{
    return qdbus_cast<bool>(m_interface->property("Linger"));
}

QString Login1UserInterface::name() const
{
    return qdbus_cast<QString>(m_interface->property("Name"));
}

QString Login1UserInterface::runtimePath() const
{
    return qdbus_cast<QString>(m_interface->property("RuntimePath"));
}

QString Login1UserInterface::service() const
{
    return qdbus_cast<QString>(m_interface->property("Service"));
}

QString Login1UserInterface::slice() const
{
    return qdbus_cast<QString>(m_interface->property("Slice"));
}

QString Login1UserInterface::state() const
{
    return qdbus_cast<QString>(m_interface->property("State"));
}

DBusSessionPath Login1UserInterface::display() const
{
    return qdbus_cast<DBusSessionPath>(m_interface->property("Display"));
}

uint Login1UserInterface::GID() const
{
    return qdbus_cast<uint>(m_interface->property("GID"));
}

uint Login1UserInterface::UID() const
{
    return qdbus_cast<uint>(m_interface->property("UID"));
}

quint64 Login1UserInterface::idleSinceHint() const
{
    return qdbus_cast<quint64>(m_interface->property("IdleSinceHint"));
}

quint64 Login1UserInterface::idleSinceHintMonotonic() const
{
    return qdbus_cast<quint64>(m_interface->property("IdleSinceHintMonotonic"));
}

quint64 Login1UserInterface::timestamp() const
{
    return qdbus_cast<quint64>(m_interface->property("Timestamp"));
}

quint64 Login1UserInterface::timestampMonotonic() const
{
    return qdbus_cast<quint64>(m_interface->property("TimestampMonotonic"));
}

QDBusPendingReply<> Login1UserInterface::kill(const int signalNumber)
{
    QDBusPendingReply<> reply =
            m_interface->asyncCallWithArgumentList("Kill", { QVariant::fromValue(signalNumber) });
    return reply;
}

QDBusPendingReply<> Login1UserInterface::terminate()
{
    QDBusPendingReply<> reply = m_interface->asyncCall("Terminate");
    return reply;
}

DLOGIN_END_NAMESPACE