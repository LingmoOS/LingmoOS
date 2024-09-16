// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "login1sessioninterface.h"

#include "ddbusinterface.h"

#include <qdbusconnection.h>

DLOGIN_BEGIN_NAMESPACE
Login1SessionInterface::Login1SessionInterface(const QString &service,
                                               const QString &path,
                                               QDBusConnection connection,
                                               QObject *parent)
    : QObject(parent)
    , m_interface(new DDBusInterface(service, path, staticInterfaceName(), connection, this))
    , m_path(path)
{
    DBusSeatPath::registerMetaType();
    DBusUserPath::registerMetaType();

    // Relay signals
    connection.connect(service, path, staticInterfaceName(), "Lock", this, SLOT(locked()));
    connection.connect(service,
                       path,
                       staticInterfaceName(),
                       "PauseDevice",
                       this,
                       SLOT(pauseDevice(quint32, quint32, const QString &)));
    connection.connect(service,
                       path,
                       staticInterfaceName(),
                       "ResumeDevice",
                       this,
                       SLOT(resumeDevice(const quint32, const quint32, const int)));
    connection.connect(service, path, staticInterfaceName(), "Unlock", this, SLOT(unlocked()));
}

Login1SessionInterface::~Login1SessionInterface() = default;

bool Login1SessionInterface::active() const
{
    return qvariant_cast<bool>(m_interface->property("Active"));
}

bool Login1SessionInterface::idleHint() const
{
    return qvariant_cast<bool>(m_interface->property("IdleHint"));
}

bool Login1SessionInterface::lockedHint() const
{
    return qvariant_cast<bool>(m_interface->property("LockedHint"));
}

bool Login1SessionInterface::remote() const
{
    return qvariant_cast<bool>(m_interface->property("Remote"));
}

QString Login1SessionInterface::sessionClass() const
{
    return qvariant_cast<QString>(m_interface->property("Class"));
}

QString Login1SessionInterface::desktop() const
{
    return qvariant_cast<QString>(m_interface->property("Desktop"));
}

QString Login1SessionInterface::display() const
{
    return qvariant_cast<QString>(m_interface->property("Display"));
}

QString Login1SessionInterface::id() const
{
    return qvariant_cast<QString>(m_interface->property("Id"));
}

QString Login1SessionInterface::name() const
{
    return qvariant_cast<QString>(m_interface->property("Name"));
}

QString Login1SessionInterface::remoteHost() const
{
    return qvariant_cast<QString>(m_interface->property("RemoteHost"));
}

QString Login1SessionInterface::remoteUser() const
{
    return qvariant_cast<QString>(m_interface->property("RemoteUser"));
}

QString Login1SessionInterface::scope() const
{
    return qvariant_cast<QString>(m_interface->property("Scope"));
}

QString Login1SessionInterface::service() const
{
    return qvariant_cast<QString>(m_interface->property("Service"));
}

QString Login1SessionInterface::state() const
{
    return qvariant_cast<QString>(m_interface->property("State"));
}

QString Login1SessionInterface::TTY() const
{
    return qvariant_cast<QString>(m_interface->property("TTY"));
}

QString Login1SessionInterface::type() const
{
    return qvariant_cast<QString>(m_interface->property("Type"));
}

DBusSeatPath Login1SessionInterface::seat() const
{
    return qdbus_cast<DBusSeatPath>(m_interface->property("Seat"));
}

DBusUserPath Login1SessionInterface::user() const
{
    return qdbus_cast<DBusUserPath>(m_interface->property("User"));
}

quint32 Login1SessionInterface::audit() const
{
    return qvariant_cast<quint32>(m_interface->property("Audit"));
}

quint32 Login1SessionInterface::leader() const
{
    return qvariant_cast<quint32>(m_interface->property("Leader"));
}

quint32 Login1SessionInterface::VTNr() const
{
    return qvariant_cast<quint32>(m_interface->property("VTNr"));
}

quint64 Login1SessionInterface::idleSinceHint() const
{
    return qvariant_cast<quint64>(m_interface->property("IdleSinceHint"));
}

quint64 Login1SessionInterface::idleSinceHintMonotonic() const
{
    return qvariant_cast<quint64>(m_interface->property("IdleSinceHintMonotonic"));
}

quint64 Login1SessionInterface::timestamp() const
{
    return qvariant_cast<quint64>(m_interface->property("Timestamp"));
}

quint64 Login1SessionInterface::timestampMonotonic() const
{
    return qvariant_cast<quint64>(m_interface->property("TimestampMonotonic"));
}

QDBusPendingReply<> Login1SessionInterface::activate()
{
    QDBusPendingReply<> reply = m_interface->asyncCall(QLatin1String("Activate"));
    return reply;
}

QDBusPendingReply<> Login1SessionInterface::kill(const QString &who, qint32 signalNumber)
{
    QDBusPendingReply<> reply =
            m_interface->asyncCallWithArgumentList(QLatin1String("Kill"),
                                                   { QVariant::fromValue(who), signalNumber });
    return reply;
}

QDBusPendingReply<> Login1SessionInterface::lock()
{
    QDBusPendingReply<> reply = m_interface->asyncCall(QLatin1String("Lock"));
    return reply;
}

QDBusPendingReply<> Login1SessionInterface::pauseDeviceComplete(quint32 major, quint32 minor)
{
    QDBusPendingReply<> reply =
            m_interface->asyncCallWithArgumentList(QLatin1String("PauseDeviceComplete"),
                                                   { major, minor });
    return reply;
}

QDBusPendingReply<> Login1SessionInterface::releaseControl()
{
    QDBusPendingReply<> reply = m_interface->asyncCall(QLatin1String("ReleaseControl"));
    return reply;
}

QDBusPendingReply<> Login1SessionInterface::releaseDevice(const quint32 major, const quint32 minor)
{
    QDBusPendingReply<> reply =
            m_interface->asyncCallWithArgumentList(QLatin1String("ReleaseDevice"),
                                                   { major, minor });
    return reply;
}

QDBusPendingReply<> Login1SessionInterface::setBrightness(const QString &subsystem,
                                                          const QString &name,
                                                          const quint32 brightness)
{
    QDBusPendingReply reply = m_interface->asyncCallWithArgumentList(
            QLatin1String("SetBrightness"),
            { QVariant::fromValue(subsystem), QVariant::fromValue(name), brightness });
    return reply;
}

QDBusPendingReply<> Login1SessionInterface::setIdleHint(const bool idle)
{
    QDBusPendingReply<> reply =
            m_interface->asyncCallWithArgumentList(QLatin1String("SetIdleHint"), { idle });
    return reply;
}

QDBusPendingReply<> Login1SessionInterface::setLockedHint(const bool locked)
{
    QDBusPendingReply<> reply =
            m_interface->asyncCallWithArgumentList(QLatin1String("SetLockedHint"), { locked });
    return reply;
}

QDBusPendingReply<> Login1SessionInterface::setType(const QString &type)
{
    QDBusPendingReply reply = m_interface->asyncCallWithArgumentList(QLatin1String("SetType"),
                                                                     { QVariant::fromValue(type) });
    return reply;
}

QDBusPendingReply<> Login1SessionInterface::takeControl(const bool force)
{
    QDBusPendingReply reply =
            m_interface->asyncCallWithArgumentList(QLatin1String("TakeControl"), { force });
    return reply;
}

QDBusPendingReply<int, bool> Login1SessionInterface::takeDevice(quint32 major, quint32 minor)
{
    QDBusPendingReply<int, bool> reply =
            m_interface->asyncCallWithArgumentList(QLatin1String("TakeDevice"), { major, minor });
    return reply;
}

QDBusPendingReply<> Login1SessionInterface::terminate()
{
    QDBusPendingReply<> reply = m_interface->asyncCall(QLatin1String("Terminate"));
    return reply;
}

QDBusPendingReply<> Login1SessionInterface::unlock()
{
    QDBusPendingReply<> reply = m_interface->asyncCall(QLatin1String("Unlock"));
    return reply;
}

DLOGIN_END_NAMESPACE
