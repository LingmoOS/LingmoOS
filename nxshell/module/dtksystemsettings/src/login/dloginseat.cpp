// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dloginseat.h"

#include "dloginseat_p.h"
#include "dlogintypes_p.h"
#include "login1seatinterface.h"

#include <qdatetime.h>
#include <qdbusconnection.h>
#include <qdbuspendingreply.h>
#include <qdebug.h>
#include <qlist.h>
#include <qobject.h>

DLOGIN_BEGIN_NAMESPACE
using DCORE_NAMESPACE::DError;
using DCORE_NAMESPACE::DExpected;
using DCORE_NAMESPACE::DUnexpected;

DLoginSeat::DLoginSeat(const QString &path, QObject *parent)
    : QObject(parent)
    , d_ptr(new DLoginSeatPrivate(this))
{
#if defined(USE_FAKE_INTERFACE) // for unit test
    const QString &Service = QStringLiteral("org.freedesktop.fakelogin1");
    QDBusConnection connection = QDBusConnection::sessionBus();
#else
    const QString &Service = QStringLiteral("org.freedesktop.login1");
    QDBusConnection connection = QDBusConnection::systemBus();
#endif

    DBusSessionPath::registerMetaType();
    Q_D(DLoginSeat);
    d->m_inter = new Login1SeatInterface(Service, path, connection, this);
}

QStringList DLoginSeat::sessions() const
{
    Q_D(const DLoginSeat);
    const auto &result = d->m_inter->sessions();
    QStringList sessionIds;
    for (const auto &sessionPath : result) {
        sessionIds.append(sessionPath.sessionId);
    }
    return sessionIds;
}

bool DLoginSeat::canGraphical() const
{
    Q_D(const DLoginSeat);
    return d->m_inter->canGraphical();
}

bool DLoginSeat::canTTY() const
{
    Q_D(const DLoginSeat);
    return d->m_inter->canTTY();
}

bool DLoginSeat::idleHint() const
{
    Q_D(const DLoginSeat);
    return d->m_inter->idleHint();
}

QString DLoginSeat::id() const
{
    Q_D(const DLoginSeat);
    return d->m_inter->id();
}

QString DLoginSeat::activeSession() const
{
    Q_D(const DLoginSeat);
    const auto &result = d->m_inter->activeSession();
    return result.sessionId;
}

QDateTime DLoginSeat::idleSinceHint() const
{
    Q_D(const DLoginSeat);
    return QDateTime::fromMSecsSinceEpoch(d->m_inter->idleSinceHint() / 1000);
}

quint64 DLoginSeat::idleSinceHintMonotonic() const
{
    Q_D(const DLoginSeat);
    return d->m_inter->idleSinceHintMonotonic();
}

DExpected<void> DLoginSeat::activateSession(const QString &sessionId)
{
    Q_D(DLoginSeat);
    QDBusPendingReply<> reply = d->m_inter->activateSession(sessionId);
    reply.waitForFinished();
    if (!reply.isValid()) {
        return DUnexpected{ DError{ reply.error().type(), reply.error().message() } };
    }
    return {};
}

DExpected<void> DLoginSeat::switchTo(quint32 VTNr)
{
    Q_D(DLoginSeat);
    QDBusPendingReply<> reply = d->m_inter->switchTo(VTNr);
    reply.waitForFinished();
    if (!reply.isValid()) {
        return DUnexpected{ DError{ reply.error().type(), reply.error().message() } };
    }
    return {};
}

DLoginSeat::~DLoginSeat() = default;
DLOGIN_END_NAMESPACE
