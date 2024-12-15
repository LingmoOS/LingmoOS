// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "login1userservice.h"

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
 * Implementation of adaptor class Login1UserAdaptor
 */

Login1UserService::Login1UserService(const QString &service, const QString &path, QObject *parent)
    : QObject(parent)
    , m_service(service)
    , m_path(path)
{
    registerService(m_service, m_path);
}

Login1UserService::~Login1UserService()
{
    unRegisterService();
}

bool Login1UserService::registerService(const QString &service, const QString &path)
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

void Login1UserService::unRegisterService()
{
    QDBusConnection connection = QDBusConnection::sessionBus();
    connection.unregisterObject(m_path);
    connection.unregisterService(m_service);
}

DBusSessionPath Login1UserService::display() const
{
    return m_display;
}

quint32 Login1UserService::GID() const
{
    return m_GID;
}

QString Login1UserService::name() const
{
    return m_name;
}

QString Login1UserService::runtimePath() const
{
    return m_runtimePath;
}

QString Login1UserService::service() const
{
    return m_applicationService;
}

QString Login1UserService::slice() const
{
    return m_slice;
}

QString Login1UserService::state() const
{
    return m_state;
}

quint64 Login1UserService::timestamp() const
{
    return m_timestamp;
}

quint64 Login1UserService::timestampMonotonic() const
{
    return m_timestampMonotonic;
}

quint32 Login1UserService::UID() const
{
    return m_UID;
}

void Login1UserService::Kill(const qint32 signalNumber)
{
    m_signalNumber = signalNumber;
}

void Login1UserService::Terminate()
{
    m_terminated = true;
}

QList<DTK_LOGIN_NAMESPACE::DBusSessionPath> Login1UserService::sessions() const
{
    return m_sessions;
}

bool Login1UserService::idleHint() const
{
    return m_idleHint;
}

bool Login1UserService::linger() const
{
    return m_linger;
}

quint64 Login1UserService::idleSinceHint() const
{
    return m_idleSinceHint;
}

quint64 Login1UserService::idleSinceHintMonotonic() const
{
    return m_idleSinceHintMonotonic;
}

DLOGIN_END_NAMESPACE
