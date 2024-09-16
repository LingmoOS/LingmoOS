// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#pragma once
#include "dlogintypes_p.h"
#include "dtklogin_global.h"

#include <ddbusinterface.h>
#include <qdbuspendingreply.h>
#include <qobject.h>

using DTK_CORE_NAMESPACE::DDBusInterface;
DLOGIN_BEGIN_NAMESPACE

class Login1SeatInterface : public QObject
{
    Q_OBJECT
public:
    static inline const char *staticInterfaceName() { return "org.freedesktop.login1.Seat"; }

    Login1SeatInterface(const QString &service,
                        const QString &path,
                        const QDBusConnection &connection,
                        QObject *parent = nullptr);
    ~Login1SeatInterface() override;

    Q_PROPERTY(QList<DBusSessionPath> sessions READ sessions);
    Q_PROPERTY(bool canGraphical READ canGraphical);
    Q_PROPERTY(bool canTTY READ canTTY);
    Q_PROPERTY(bool idleHint READ idleHint);
    Q_PROPERTY(QString id READ id);
    Q_PROPERTY(DBusSessionPath activeSession READ activeSession);
    Q_PROPERTY(quint64 idleSinceHint READ idleSinceHint);
    Q_PROPERTY(quint64 idleSinceHintMonotonic READ idleSinceHintMonotonic);

    QList<DBusSessionPath> sessions() const;
    bool canGraphical() const;
    bool canTTY() const;
    bool idleHint() const;
    QString id() const;
    DBusSessionPath activeSession() const;
    quint64 idleSinceHint() const;
    quint64 idleSinceHintMonotonic() const;

public slots:
    QDBusPendingReply<> activateSession(const QString &sessionId);
    QDBusPendingReply<> switchTo(quint32 VTNr);
    QDBusPendingReply<> switchToNext();
    QDBusPendingReply<> switchToPrevious();
    QDBusPendingReply<> terminate();

private:
    DDBusInterface *m_interface;
    QString m_path;
};

DLOGIN_END_NAMESPACE
