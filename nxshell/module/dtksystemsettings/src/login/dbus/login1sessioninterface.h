// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#pragma once
#include "dlogintypes_p.h"

#include <ddbusinterface.h>
#include <qdbuspendingreply.h>

using DTK_CORE_NAMESPACE::DDBusInterface;
DLOGIN_BEGIN_NAMESPACE

class Login1SessionInterface : public QObject
{
    Q_OBJECT
public:
    static inline const char *staticInterfaceName() { return "org.freedesktop.login1.Session"; }

    explicit Login1SessionInterface(const QString &service,
                                    const QString &path,
                                    QDBusConnection connection,
                                    QObject *parent = nullptr);
    ~Login1SessionInterface() override;

    Q_PROPERTY(bool active READ active);
    Q_PROPERTY(bool idleHint READ idleHint);
    Q_PROPERTY(bool lockedHint READ lockedHint);
    Q_PROPERTY(bool remote READ remote);
    Q_PROPERTY(QString sessionClass READ sessionClass);
    Q_PROPERTY(QString desktop READ desktop);
    Q_PROPERTY(QString display READ display);
    Q_PROPERTY(QString id READ id);
    Q_PROPERTY(QString name READ name);
    Q_PROPERTY(QString remoteHost READ remoteHost);
    Q_PROPERTY(QString remoteUser READ remoteUser);
    Q_PROPERTY(QString scope READ scope);
    Q_PROPERTY(QString service READ service);
    Q_PROPERTY(QString state READ state);
    // Why TTY?
    Q_PROPERTY(QString TTY READ TTY);
    Q_PROPERTY(QString type READ type);

    Q_PROPERTY(DBusSeatPath seat READ seat);
    Q_PROPERTY(DBusUserPath user READ user);

    Q_PROPERTY(quint32 audit READ audit);
    Q_PROPERTY(quint32 leader READ leader);
    Q_PROPERTY(quint32 VTNr READ VTNr);

    Q_PROPERTY(quint64 idleSinceHint READ idleSinceHint);
    Q_PROPERTY(quint64 idleSinceHintMonotonic READ idleSinceHintMonotonic);
    Q_PROPERTY(quint64 timestamp READ timestamp);
    Q_PROPERTY(quint64 timestampMonotonic READ timestampMonotonic);

    bool active() const;
    bool idleHint() const;
    bool lockedHint() const;
    bool remote() const;
    QString sessionClass() const;
    QString desktop() const;
    QString display() const;
    QString id() const;
    QString name() const;
    QString remoteHost() const;
    QString remoteUser() const;
    QString scope() const;
    QString service() const;
    QString state() const;
    QString TTY() const;
    QString type() const;
    DBusSeatPath seat() const;
    DBusUserPath user() const;
    quint32 audit() const;
    quint32 leader() const;
    quint32 VTNr() const;

    quint64 idleSinceHint() const;
    quint64 idleSinceHintMonotonic() const;
    quint64 timestamp() const;
    quint64 timestampMonotonic() const;
signals:
    void locked();
    void pauseDevice(const quint32 value, const quint32 device, const QString &location);
    void resumeDevice(const quint32 value, const quint32 device, const int descriptor);
    void unlocked();

public slots:
    QDBusPendingReply<> activate();
    QDBusPendingReply<> kill(const QString &who, qint32 signalNumber);
    QDBusPendingReply<> lock();
    QDBusPendingReply<> pauseDeviceComplete(quint32 major, quint32 minor);
    QDBusPendingReply<> releaseControl();
    QDBusPendingReply<> releaseDevice(quint32 major, quint32 minor);
    QDBusPendingReply<> setBrightness(const QString &subsystem,
                                      const QString &name,
                                      quint32 brightness);
    QDBusPendingReply<> setIdleHint(bool idle);
    QDBusPendingReply<> setLockedHint(bool locked);
    QDBusPendingReply<> setType(const QString &type);
    QDBusPendingReply<> takeControl(bool force);
    QDBusPendingReply<int, bool> takeDevice(quint32 major, quint32 minor);
    QDBusPendingReply<> terminate();
    QDBusPendingReply<> unlock();

private:
    DDBusInterface *m_interface;
    QString m_path;
};

DLOGIN_END_NAMESPACE
