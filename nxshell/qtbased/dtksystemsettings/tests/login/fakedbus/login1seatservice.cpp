// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "login1seatservice.h"

#include "dlogintypes_p.h"

#include <qbytearray.h>
#include <qlist.h>
#include <qmap.h>
#include <qmetaobject.h>
#include <qstring.h>
#include <qstringlist.h>
#include <qvariant.h>

DLOGIN_BEGIN_NAMESPACE

/*
 * Implementation of adaptor class Login1SeatAdaptor
 */

Login1SeatService::Login1SeatService(const QString &service, const QString &path, QObject *parent)
    : QObject(parent)
    , m_service(service)
    , m_path(path)
{
    DBusSessionPath::registerMetaType();
    registerService(m_service, m_path);
}

Login1SeatService::~Login1SeatService()
{
    unRegisterService();
}

bool Login1SeatService::registerService(const QString &service, const QString &path)
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

void Login1SeatService::unRegisterService()
{
    QDBusConnection connection = QDBusConnection::sessionBus();
    connection.unregisterObject(m_path);
    connection.unregisterService(m_service);
}

DBusSessionPath Login1SeatService::activeSession() const
{
    return m_activeSession;
}

bool Login1SeatService::canGraphical() const
{
    return m_canGraphical;
}

bool Login1SeatService::canTTY() const
{
    return m_canTTY;
}

bool Login1SeatService::idleHint() const
{
    return m_idleHint;
}

QString Login1SeatService::id() const
{
    return m_id;
}

quint64 Login1SeatService::idleSinceHint() const
{
    return m_idleSinceHint;
}

quint64 Login1SeatService::idleSinceHintMonotonic() const
{
    return m_idleSinceHintMonotonic;
}

QList<DBusSessionPath> Login1SeatService::sessions() const
{
    return m_sessions;
}

void Login1SeatService::ActivateSession(const QString &sessionId)
{
    m_sessionId = sessionId;
}

void Login1SeatService::SwitchTo(const quint32 VTNr)
{
    m_VTNr = VTNr;
}

void Login1SeatService::SwitchToNext()
{
    m_VTNr++;
}

void Login1SeatService::SwitchToPrevious()
{
    m_VTNr--;
}

DLOGIN_END_NAMESPACE
