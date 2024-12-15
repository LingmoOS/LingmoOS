// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "login1sessionservice.h"

#include "dlogintypes_p.h"

#include <qbytearray.h>
#include <qlist.h>
#include <qmap.h>
#include <qmetaobject.h>
#include <qstring.h>
#include <qvariant.h>

DLOGIN_BEGIN_NAMESPACE

/*
 * Implementation of adaptor class Login1SessionAdaptor
 */

Login1SessionService::Login1SessionService(const QString &service,
                                           const QString &path,
                                           QObject *parent)
    : QObject(parent)
    , m_service(service)
    , m_path(path)
{
    DBusUserPath::registerMetaType();
    DBusSeatPath::registerMetaType();
    registerService(m_service, m_path);
}

Login1SessionService::~Login1SessionService()
{
    unRegisterService();
}

bool Login1SessionService::registerService(const QString &service, const QString &path)
{
    QDBusConnection connection = QDBusConnection::sessionBus();
    if (!connection.registerService(service)) {
        QString errorMsg = connection.lastError().message();
        if (errorMsg.isEmpty())
            errorMsg = "maybe it's running";

        qWarning() << QString("Can't register the %1 service, %2.").arg(service).arg(errorMsg);
        return false;
    }
    if (!connection.registerObject(path, this, QDBusConnection::ExportAllContents)) {
        qWarning() << QString("Can't register %1 the D-Bus object.").arg(path);
        return false;
    }
    return true;
}

void Login1SessionService::unRegisterService()
{
    QDBusConnection connection = QDBusConnection::sessionBus();
    connection.unregisterObject(m_path);
    connection.unregisterService(m_service);
}

bool Login1SessionService::active() const
{
    return m_active;
}

uint Login1SessionService::audit() const
{
    return m_audit;
}

QString Login1SessionService::sessionClass() const
{
    return m_sessionClass;
}

QString Login1SessionService::desktop() const
{
    return m_desktop;
}

QString Login1SessionService::display() const
{
    return m_display;
}

QString Login1SessionService::id() const
{
    return m_id;
}

bool Login1SessionService::idleHint() const
{
    return m_idleHint;
}

quint64 Login1SessionService::idleSinceHint() const
{
    return m_idleSinceHint;
}

quint64 Login1SessionService::idleSinceHintMonotonic() const
{
    return m_idleSinceHintMonotonic;
}

uint Login1SessionService::leader() const
{
    return m_leader;
}

bool Login1SessionService::lockedHint() const
{
    return m_lockedHint;
}

QString Login1SessionService::name() const
{
    return m_name;
}

bool Login1SessionService::remote() const
{
    return m_remote;
}

QString Login1SessionService::remoteHost() const
{
    return m_remoteHost;
}

QString Login1SessionService::remoteUser() const
{
    return m_remoteUser;
}

QString Login1SessionService::scope() const
{
    return m_scope;
}

DBusSeatPath Login1SessionService::seat() const
{
    return m_seat;
}

QString Login1SessionService::service() const
{
    return m_applicationService;
}

QString Login1SessionService::state() const
{
    return m_state;
}

QString Login1SessionService::TTY() const
{
    return m_TTY;
}

quint64 Login1SessionService::timestamp() const
{
    return m_timestamp;
}

quint64 Login1SessionService::timestampMonotonic() const
{
    return m_timestampMonotonic;
}

QString Login1SessionService::type() const
{
    return m_type;
}

DBusUserPath Login1SessionService::user() const
{
    return m_user;
}

uint Login1SessionService::VTNr() const
{
    return m_VTNr;
}

void Login1SessionService::Activate()
{
    m_active = true;
}

void Login1SessionService::Kill(const QString &who, qint32 signalNumber)
{
    m_who = who;
    m_signalNumber = signalNumber;
}

void Login1SessionService::Lock()
{
    m_lockedHint = true;
}

void Login1SessionService::SetIdleHint(bool idle)
{
    m_idleHint = idle;
}

void Login1SessionService::SetType(const QString &type)
{
    m_type = type;
}

void Login1SessionService::Terminate()
{
    m_terminated = true;
}

DLOGIN_END_NAMESPACE
