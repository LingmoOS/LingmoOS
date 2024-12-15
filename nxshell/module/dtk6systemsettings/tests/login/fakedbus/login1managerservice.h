// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#pragma once
#include "dlogintypes_p.h"

#include <qdbusabstractadaptor.h>
#include <qdbusunixfiledescriptor.h>
#include <qobject.h>
#include <QStringList>

QT_BEGIN_NAMESPACE
class QByteArray;
template<class T>
class QList;
template<class Key, class Value>
class QMap;
class QString;
class QVariant;
class QDBusObjectPath;
QT_END_NAMESPACE

DLOGIN_BEGIN_NAMESPACE

/*
 * Adaptor class for interface org.freedesktop.login1.Manager
 */
class Login1ManagerService : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.freedesktop.login1.Manager")
public:
    Login1ManagerService(const QString &service = QStringLiteral("org.freedesktop.fakelogin1"),
                         const QString &path = QStringLiteral("/org/freedesktop/login1/manager"),
                         QObject *parent = nullptr);
    ~Login1ManagerService() override;

public: // PROPERTIES
    Q_PROPERTY(QString BlockInhibited MEMBER m_blockInhibited READ blockInhibited);
    Q_PROPERTY(QString DelayInhibited MEMBER m_delayInhibited READ delayInhibited);
    Q_PROPERTY(bool Docked MEMBER m_docked READ docked);
    Q_PROPERTY(QString HandleHibernateKey MEMBER m_handleHibernateKey READ handleHibernateKey);
    Q_PROPERTY(QString HandleLidSwitch MEMBER m_handleLidSwitch READ handleLidSwitch);
    Q_PROPERTY(QString HandleLidSwitchDocked MEMBER m_handleLidSwitchDocked READ handleLidSwitchDocked);
    Q_PROPERTY(QString HandleLidSwitchExternalPower MEMBER m_handleLidSwitchExternalPower READ handleLidSwitchExternalPower);
    Q_PROPERTY(QString HandlePowerKey MEMBER m_handlePowerKey READ handlePowerKey);
    Q_PROPERTY(QString HandleSuspendKey MEMBER m_handleSuspendKey READ handleSuspendKey);
    Q_PROPERTY(quint64 HoldoffTimeoutUSec MEMBER m_holdoffTimeoutUSec READ holdoffTimeoutUSec);
    Q_PROPERTY(QString IdleAction MEMBER m_idleAction READ idleAction);
    Q_PROPERTY(quint64 IdleActionUSec MEMBER m_idleActionUSec READ idleActionUSec);
    Q_PROPERTY(bool IdleHint MEMBER m_idleHint READ idleHint);
    Q_PROPERTY(quint64 IdleSinceHint MEMBER m_idleSinceHint READ idleSinceHint);
    Q_PROPERTY(quint64 IdleSinceHintMonotonic MEMBER m_idleSinceHintMonotonic READ idleSinceHintMonotonic);
    Q_PROPERTY(quint64 InhibitDelayMaxUSec MEMBER m_inhibitDelayMaxUSec READ inhibitDelayMaxUSec);
    Q_PROPERTY(quint64 InhibitorsMax MEMBER m_inhibitorsMax READ inhibitorsMax);
    Q_PROPERTY(QStringList KillExcludeUsers MEMBER m_killExcludeUsers READ killExcludeUsers);
    Q_PROPERTY(QStringList KillOnlyUsers MEMBER m_killOnlyUsers READ killOnlyUsers);
    Q_PROPERTY(bool KillUserProcesses MEMBER m_killUserProcesses READ killUserProcesses);
    Q_PROPERTY(bool LidClosed MEMBER m_lidClosed READ lidClosed);
    Q_PROPERTY(quint32 NAutoVTs MEMBER m_nAutoVTs READ nAutoVTs);
    Q_PROPERTY(quint64 NCurrentInhibitors MEMBER m_nCurrentInhibitors READ nCurrentInhibitors);
    Q_PROPERTY(quint64 NCurrentSessions MEMBER m_nCurrentSessions READ nCurrentSessions);
    Q_PROPERTY(bool PreparingForShutdown MEMBER m_preparingForShutdown READ preparingForShutdown);
    Q_PROPERTY(bool PreparingForSleep MEMBER m_preparingForSleep READ preparingForSleep);
    Q_PROPERTY(bool RemoveIPC MEMBER m_removeIPC READ removeIPC);
    Q_PROPERTY(quint64 RuntimeDirectoryInodesMax MEMBER m_runtimeDirectoryInodesMax READ runtimeDirectoryInodesMax);
    Q_PROPERTY(quint64 RuntimeDirectorySize MEMBER m_runtimeDirectorySize READ runtimeDirectorySize);
    Q_PROPERTY(DBusScheduledShutdownValue ScheduledShutdown MEMBER m_scheduledShutdown READ scheduledShutdown);
    Q_PROPERTY(quint64 SessionsMax MEMBER m_sessionsMax READ sessionsMax);
    Q_PROPERTY(quint64 UserStopDelayUSec MEMBER m_userStopDelayUSec READ userStopDelayUSec);

    QString blockInhibited() const;
    QString delayInhibited() const;
    bool docked() const;
    QString handleHibernateKey() const;
    QString handleLidSwitch() const;
    QString handleLidSwitchDocked() const;
    QString handleLidSwitchExternalPower() const;
    QString handlePowerKey() const;
    QString handleSuspendKey() const;
    quint64 holdoffTimeoutUSec() const;
    QString idleAction() const;
    quint64 idleActionUSec() const;
    bool idleHint() const;
    quint64 idleSinceHint() const;
    quint64 idleSinceHintMonotonic() const;
    quint64 inhibitDelayMaxUSec() const;
    quint64 inhibitorsMax() const;
    QStringList killExcludeUsers() const;
    QStringList killOnlyUsers() const;
    bool killUserProcesses() const;
    bool lidClosed() const;
    quint32 nAutoVTs() const;
    quint64 nCurrentInhibitors() const;
    quint64 nCurrentSessions() const;
    bool preparingForShutdown() const;
    bool preparingForSleep() const;
    bool removeIPC() const;
    quint64 runtimeDirectoryInodesMax() const;
    quint64 runtimeDirectorySize() const;
    DBusScheduledShutdownValue scheduledShutdown() const;
    quint64 sessionsMax() const;
    quint64 userStopDelayUSec() const;

public Q_SLOTS: // METHODS
    void ActivateSession(const QString &session_id);
    void ActivateSessionOnSeat(const QString &session_id, const QString &seat_id);
    QString CanHalt();
    QString CanHibernate();
    QString CanHybridSleep();
    QString CanPowerOff();
    QString CanReboot();
    QString CanSuspend();
    QString CanSuspendThenHibernate();
    bool CancelScheduledShutdown();
    QDBusObjectPath GetSeat(const QString &seat_id);
    QDBusObjectPath GetSession(const QString &session_id);
    QDBusObjectPath GetSessionByPID(quint32 pid);
    QDBusObjectPath GetUser(quint32 uid);
    QDBusObjectPath GetUserByPID(quint32 pid);
    void Halt(bool interactive);
    void Hibernate(bool interactive);
    void HybridSleep(bool interactive);
    QDBusUnixFileDescriptor
    Inhibit(const QString &what, const QString &who, const QString &why, const QString &mode);
    void KillSession(const QString &session_id, const QString &who, qint32 signal_number);
    void KillUser(quint32 uid, qint32 signal_number);
    QList<DTK_LOGIN_NAMESPACE::DBusInhibitor> ListInhibitors();
    QList<DTK_LOGIN_NAMESPACE::DBusSeat> ListSeats();
    QList<DTK_LOGIN_NAMESPACE::DBusSession> ListSessions();
    QList<DTK_LOGIN_NAMESPACE::DBusUser> ListUsers();
    void LockSession(const QString &session_id);
    void LockSessions();
    void PowerOff(bool interactive);
    void Reboot(bool interactive);
    void ScheduleShutdown(const QString &type, quint64 usec);
    void SetUserLinger(quint32 uid, bool enable, bool interactive);
    void Suspend(bool interactive);
    void SuspendThenHibernate(bool interactive);
    void TerminateSeat(const QString &seat_id);
    void TerminateSession(const QString &session_id);
    void TerminateUser(quint32 uid);

Q_SIGNALS: // SIGNALS
    void PrepareForShutdown(bool start);
    void PrepareForSleep(bool start);
    void SeatNew(const QString &seat_id, const QDBusObjectPath &object_path);
    void SeatRemoved(const QString &seat_id, const QDBusObjectPath &object_path);
    void SessionNew(const QString &session_id, const QDBusObjectPath &object_path);
    void SessionRemoved(const QString &session_id, const QDBusObjectPath &object_path);
    void UserNew(quint32 uid, const QDBusObjectPath &object_path);
    void UserRemoved(quint32 uid, const QDBusObjectPath &object_path);

public:
    // properties
    QString m_blockInhibited;
    QString m_delayInhibited;
    bool m_docked;
    QString m_handleHibernateKey;
    QString m_handleLidSwitch;
    QString m_handleLidSwitchDocked;
    QString m_handleLidSwitchExternalPower;
    QString m_handlePowerKey;
    QString m_handleSuspendKey;
    quint64 m_holdoffTimeoutUSec;
    QString m_idleAction;
    quint64 m_idleActionUSec;
    bool m_idleHint;
    quint64 m_idleSinceHint;
    quint64 m_idleSinceHintMonotonic;
    quint64 m_inhibitDelayMaxUSec;
    quint64 m_inhibitorsMax;
    QStringList m_killExcludeUsers;
    QStringList m_killOnlyUsers;
    bool m_killUserProcesses;
    bool m_lidClosed;
    quint32 m_nAutoVTs;
    quint64 m_nCurrentInhibitors;
    quint64 m_nCurrentSessions;
    bool m_preparingForShutdown;
    bool m_preparingForSleep;
    bool m_removeIPC;
    quint64 m_runtimeDirectoryInodesMax;
    quint64 m_runtimeDirectorySize;
    DBusScheduledShutdownValue m_scheduledShutdown;
    quint64 m_sessionsMax;
    quint64 m_userStopDelayUSec;

    // for function tests
    QString m_sessionId;
    QString m_sessionRole;
    QString m_seatId;
    quint32 m_UID;
    quint32 m_PID;

    QString m_canHalt;
    QString m_canHibernate;
    QString m_canHybridSleep;
    QString m_canPowerOff;
    QString m_canReboot;
    QString m_canSuspend;
    QString m_canSuspendThenHibernate;

    bool m_cancelScheduledShutdown;

    QDBusObjectPath m_seatPath;
    QDBusObjectPath m_sessionPath;
    QDBusObjectPath m_userPath;

    bool m_haltInteractive;
    bool m_hibernateInteractive;
    bool m_hybridSleepInteractive;
    bool m_powerOffInteractive;
    bool m_rebootInteractive;
    bool m_suspendInteractive;
    bool m_suspendThenHibernateInteractive;

    QDBusUnixFileDescriptor m_inhibitFileDescriptor;
    QString m_who;
    qint32 m_signalNumber;

    DBusInhibitor m_inhibitor;
    QList<DBusInhibitor> m_inhibitors;
    QList<DBusSeat> m_seats;
    QList<DBusSession> m_sessions;
    QList<DBusUser> m_users;

    bool m_lockSessions;
    bool m_userLinger;
    bool m_userLingerInteractive;

private:
    bool registerService(const QString &service, const QString &path);
    void unRegisterService();
    QString m_service;
    QString m_path;
};

DLOGIN_END_NAMESPACE
