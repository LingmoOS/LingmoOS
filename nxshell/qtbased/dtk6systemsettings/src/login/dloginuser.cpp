// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dloginuser.h"

#include "dlogintypes_p.h"
#include "dloginuser_p.h"
#include "dloginutils.h"
#include "login1userinterface.h"

#include <qdbusconnection.h>
#include <qdbuspendingreply.h>
#include <qdebug.h>
#include <qlist.h>
#include <qobject.h>

DLOGIN_BEGIN_NAMESPACE
using DCORE_NAMESPACE::DError;
using DCORE_NAMESPACE::DExpected;
using DCORE_NAMESPACE::DUnexpected;

DLoginUser::DLoginUser(const QString &path, QObject *parent)
    : QObject(parent)
    , d_ptr(new DLoginUserPrivate(this))
{
#if defined(USE_FAKE_INTERFACE) // for unit test
    const QString &Service = QStringLiteral("org.freedesktop.fakelogin1");
    QDBusConnection connection = QDBusConnection::sessionBus();
#else
    const QString &Service = QStringLiteral("org.freedesktop.login1");
    QDBusConnection connection = QDBusConnection::systemBus();
#endif

    DBusSessionPath::registerMetaType();
    qRegisterMetaType<UserState>("UserState");
    Q_D(DLoginUser);
    d->m_inter = new Login1UserInterface(Service, path, connection, this);
}

DLoginUser::~DLoginUser() { }

QStringList DLoginUser::sessions() const
{
    Q_D(const DLoginUser);
    const auto &result = d->m_inter->sessions();
    QStringList sessionIds;
    for (auto &&sessionPath : result) {
        sessionIds.append(sessionPath.sessionId);
    }
    return sessionIds;
}

bool DLoginUser::idleHint() const
{
    Q_D(const DLoginUser);
    return d->m_inter->idleHint();
}

bool DLoginUser::linger() const
{
    Q_D(const DLoginUser);
    return d->m_inter->linger();
}

QString DLoginUser::name() const
{
    Q_D(const DLoginUser);
    return d->m_inter->name();
}

QString DLoginUser::runtimePath() const
{
    Q_D(const DLoginUser);
    return d->m_inter->runtimePath();
}

QString DLoginUser::service() const
{
    Q_D(const DLoginUser);
    return d->m_inter->service();
}

QString DLoginUser::slice() const
{
    Q_D(const DLoginUser);
    return d->m_inter->slice();
}

UserState DLoginUser::state() const
{
    Q_D(const DLoginUser);
    return Utils::stringToUserState(d->m_inter->state());
}

QString DLoginUser::display() const
{
    Q_D(const DLoginUser);
    const auto &result = d->m_inter->display();
    return result.sessionId;
}

quint32 DLoginUser::GID() const
{
    Q_D(const DLoginUser);
    return d->m_inter->GID();
}

quint32 DLoginUser::UID() const
{
    Q_D(const DLoginUser);
    return d->m_inter->UID();
}

QDateTime DLoginUser::idleSinceHint() const
{
    Q_D(const DLoginUser);
    return QDateTime::fromMSecsSinceEpoch(d->m_inter->idleSinceHint() / 1000);
}

quint64 DLoginUser::idleSinceHintMonotonic() const
{
    Q_D(const DLoginUser);
    return d->m_inter->idleSinceHintMonotonic();
}

QDateTime DLoginUser::loginTime() const
{
    Q_D(const DLoginUser);
    return QDateTime::fromMSecsSinceEpoch(d->m_inter->timestamp() / 1000);
}

quint64 DLoginUser::loginTimeMonotonic() const
{
    Q_D(const DLoginUser);
    return d->m_inter->timestampMonotonic();
}

// public slots

DExpected<void> DLoginUser::kill(qint32 signalNumber)
{
    Q_D(DLoginUser);
    QDBusPendingReply<> reply = d->m_inter->kill(signalNumber);
    reply.waitForFinished();
    if (!reply.isValid()) {
        return DUnexpected{ DError{ reply.error().type(), reply.error().message() } };
    }
    return {};
}

DExpected<void> DLoginUser::terminate()
{
    Q_D(DLoginUser);
    QDBusPendingReply<> reply = d->m_inter->terminate();
    reply.waitForFinished();
    if (!reply.isValid()) {
        return DUnexpected{ DError{ reply.error().type(), reply.error().message() } };
    }
    return {};
}

DLOGIN_END_NAMESPACE
