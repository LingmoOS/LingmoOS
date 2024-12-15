// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#pragma once

#include "dlogintypes_p.h"

#include <qobject.h>
#include <QStringList>

#include <QtDBus>
QT_BEGIN_NAMESPACE
class QByteArray;
template<class T>
class QList;
template<class Key, class Value>
class QMap;
class QString;
class QVariant;
QT_END_NAMESPACE

DLOGIN_BEGIN_NAMESPACE

/*
 * Adaptor class for interface org.freedesktop.login1.Seat
 */
class Login1SeatService : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.freedesktop.login1.Seat")
public:
    Login1SeatService(const QString &service = QStringLiteral("org.freedesktop.fakelogin1"),
                      const QString &path = QStringLiteral("/org/freedesktop/login1/seat"),
                      QObject *parent = nullptr);
    ~Login1SeatService() override;

public: // PROPERTIES
    Q_PROPERTY(DBusSessionPath ActiveSession MEMBER m_activeSession READ activeSession);
    Q_PROPERTY(bool CanGraphical MEMBER m_canGraphical READ canGraphical);
    Q_PROPERTY(bool CanTTY MEMBER m_canTTY READ canTTY);
    Q_PROPERTY(QString Id MEMBER m_id READ id);
    Q_PROPERTY(bool IdleHint MEMBER m_idleHint READ idleHint);
    Q_PROPERTY(quint64 IdleSinceHint MEMBER m_idleSinceHint READ idleSinceHint);
    Q_PROPERTY(quint64 IdleSinceHintMonotonic MEMBER m_idleSinceHintMonotonic READ idleSinceHintMonotonic);
    Q_PROPERTY(QList<DTK_LOGIN_NAMESPACE::DBusSessionPath> Sessions MEMBER m_sessions READ sessions);

    DBusSessionPath activeSession() const;
    bool canGraphical() const;
    bool canTTY() const;
    bool idleHint() const;
    QString id() const;
    quint64 idleSinceHint() const;
    quint64 idleSinceHintMonotonic() const;
    QList<DTK_LOGIN_NAMESPACE::DBusSessionPath> sessions() const;

public Q_SLOTS: // METHODS
    void ActivateSession(const QString &sessionId);
    void SwitchTo(const quint32 VTNr);
    void SwitchToNext();
    void SwitchToPrevious();

public:
    DBusSessionPath m_activeSession;
    bool m_canGraphical;
    bool m_canTTY;
    bool m_idleHint;
    QString m_id;
    QString m_sessionId;
    quint32 m_VTNr;
    quint64 m_idleSinceHint;
    quint64 m_idleSinceHintMonotonic;
    QList<DBusSessionPath> m_sessions;

private:
    bool registerService(const QString &service, const QString &path);
    void unRegisterService();
    QString m_service;
    QString m_path;
};

DLOGIN_END_NAMESPACE
