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
 * Adaptor class for interface org.freedesktop.login1.Session
 */
class Login1SessionService : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.freedesktop.login1.Session")
public:
    explicit Login1SessionService(
            const QString &service = QStringLiteral("org.freedesktop.fakelogin1"),
            const QString &path = QStringLiteral("/org/freedesktop/login1/session"),
            QObject *parent = nullptr);
    ~Login1SessionService() override;

public: // PROPERTIES
    Q_PROPERTY(bool Active MEMBER m_active READ active);
    Q_PROPERTY(quint32 Audit MEMBER m_audit READ audit);
    Q_PROPERTY(QString Class MEMBER m_sessionClass READ sessionClass);
    Q_PROPERTY(QString Desktop MEMBER m_desktop READ desktop);
    Q_PROPERTY(QString Display MEMBER m_display READ display);
    Q_PROPERTY(QString Id MEMBER m_id READ id);
    Q_PROPERTY(bool IdleHint MEMBER m_idleHint READ idleHint);
    Q_PROPERTY(quint64 IdleSinceHint MEMBER m_idleSinceHint READ idleSinceHint);
    Q_PROPERTY(quint64 IdleSinceHintMonotonic MEMBER m_idleSinceHintMonotonic READ idleSinceHintMonotonic);
    Q_PROPERTY(quint32 Leader MEMBER m_leader READ leader);
    Q_PROPERTY(bool LockedHint MEMBER m_lockedHint READ lockedHint);
    Q_PROPERTY(QString Name MEMBER m_name READ name);
    Q_PROPERTY(bool Remote MEMBER m_remote READ remote);
    Q_PROPERTY(QString RemoteHost MEMBER m_remoteHost READ remoteHost);
    Q_PROPERTY(QString RemoteUser MEMBER m_remoteUser READ remoteUser);
    Q_PROPERTY(QString Scope MEMBER m_scope READ scope);
    Q_PROPERTY(DTK_LOGIN_NAMESPACE::DBusSeatPath Seat MEMBER m_seat READ seat);
    Q_PROPERTY(QString Service MEMBER m_applicationService READ service);
    Q_PROPERTY(QString State MEMBER m_state READ state);
    Q_PROPERTY(QString TTY MEMBER m_TTY READ TTY);
    Q_PROPERTY(quint64 Timestamp MEMBER m_timestamp READ timestamp);
    Q_PROPERTY(quint64 TimestampMonotonic MEMBER m_timestampMonotonic READ timestampMonotonic);
    Q_PROPERTY(QString Type MEMBER m_type READ type);
    Q_PROPERTY(DTK_LOGIN_NAMESPACE::DBusUserPath User MEMBER m_user READ user);
    Q_PROPERTY(quint32 VTNr MEMBER m_VTNr READ VTNr);

    bool active() const;
    quint32 audit() const;
    QString sessionClass() const;
    QString desktop() const;
    QString display() const;
    QString id() const;
    bool idleHint() const;
    quint64 idleSinceHint() const;
    quint64 idleSinceHintMonotonic() const;
    quint32 leader() const;
    bool lockedHint() const;
    QString name() const;
    bool remote() const;
    QString remoteHost() const;
    QString remoteUser() const;
    QString scope() const;
    DTK_LOGIN_NAMESPACE::DBusSeatPath seat() const;
    QString service() const;
    QString state() const;
    QString TTY() const;
    quint64 timestamp() const;
    quint64 timestampMonotonic() const;
    QString type() const;
    DTK_LOGIN_NAMESPACE::DBusUserPath user() const;
    quint32 VTNr() const;

public Q_SLOTS: // METHODS
    void Activate();
    void Kill(const QString &who, qint32 signal_number);
    void Lock();
    void SetIdleHint(bool idle);
    void SetType(const QString &type);
    void Terminate();

public:
    QString m_who;
    qint32 m_signalNumber;
    bool m_terminated;
    bool m_active;
    quint32 m_audit;
    QString m_sessionClass;
    QString m_desktop;
    QString m_display;
    QString m_id;
    bool m_idleHint;
    quint64 m_idleSinceHint;
    quint64 m_idleSinceHintMonotonic;
    quint32 m_leader;
    bool m_lockedHint;
    QString m_name;
    bool m_remote;
    QString m_remoteHost;
    QString m_remoteUser;
    QString m_scope;
    DBusSeatPath m_seat;
    QString m_applicationService;
    QString m_state;
    QString m_TTY;
    quint64 m_timestamp;
    quint64 m_timestampMonotonic;
    QString m_type;
    DBusUserPath m_user;
    quint32 m_VTNr;

private:
    bool registerService(const QString &service, const QString &path);
    void unRegisterService();
    QString m_service;
    QString m_path;
};

DLOGIN_END_NAMESPACE
