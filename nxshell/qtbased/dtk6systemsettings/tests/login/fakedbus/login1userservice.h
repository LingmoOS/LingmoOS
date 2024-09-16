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
struct DBusSessionPath;

/*
 * Adaptor class for interface org.freedesktop.login1.User
 */
class Login1UserService : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.freedesktop.login1.User")
public:
    Login1UserService(const QString &service = QStringLiteral("org.freedesktop.fakelogin1"),
                      const QString &path = QStringLiteral("/org/freedesktop/login1/user"),
                      QObject *parent = nullptr);
    ~Login1UserService() override;

public: // PROPERTIES
    Q_PROPERTY(DBusSessionPath Display MEMBER m_display READ display);
    Q_PROPERTY(bool IdleHint MEMBER m_idleHint READ idleHint);
    Q_PROPERTY(bool Linger MEMBER m_linger READ linger);
    Q_PROPERTY(QList<DTK_LOGIN_NAMESPACE::DBusSessionPath> Sessions MEMBER m_sessions READ sessions);
    Q_PROPERTY(quint32 UID MEMBER m_UID READ UID);
    Q_PROPERTY(quint32 GID MEMBER m_GID READ GID);
    Q_PROPERTY(QString Name MEMBER m_name READ name);
    Q_PROPERTY(QString RuntimePath MEMBER m_runtimePath READ runtimePath);
    Q_PROPERTY(QString Service MEMBER m_applicationService READ service);
    Q_PROPERTY(QString Slice MEMBER m_slice READ slice);
    Q_PROPERTY(QString State MEMBER m_state READ state);
    Q_PROPERTY(quint64 IdleSinceHint MEMBER m_idleSinceHint READ idleSinceHint);
    Q_PROPERTY(quint64 IdleSinceHintMonotonic MEMBER m_idleSinceHintMonotonic READ idleSinceHintMonotonic);
    Q_PROPERTY(quint64 Timestamp MEMBER m_timestamp READ timestamp);
    Q_PROPERTY(quint64 TimestampMonotonic MEMBER m_timestampMonotonic READ timestampMonotonic);

    DBusSessionPath display() const;
    QList<DTK_LOGIN_NAMESPACE::DBusSessionPath> sessions() const;
    quint32 UID() const;
    quint32 GID() const;
    bool idleHint() const;
    bool linger() const;
    QString name() const;
    QString runtimePath() const;
    QString service() const;
    QString slice() const;
    QString state() const;
    quint64 idleSinceHint() const;
    quint64 idleSinceHintMonotonic() const;
    quint64 timestamp() const;
    quint64 timestampMonotonic() const;

public Q_SLOTS: // METHODS
    void Kill(const qint32 signalNumber);
    void Terminate();

public:
    bool m_terminated;
    qint32 m_signalNumber;
    DBusSessionPath m_display;
    QList<DBusSessionPath> m_sessions;
    bool m_idleHint;
    bool m_linger;
    quint32 m_UID;
    quint32 m_GID;
    QString m_name;
    QString m_runtimePath;
    QString m_applicationService;
    QString m_slice;
    QString m_state;
    quint64 m_idleSinceHint;
    quint64 m_idleSinceHintMonotonic;
    quint64 m_timestamp;
    quint64 m_timestampMonotonic;

private:
    bool registerService(const QString &service, const QString &path);
    void unRegisterService();
    QString m_service;
    QString m_path;
};

DLOGIN_END_NAMESPACE
