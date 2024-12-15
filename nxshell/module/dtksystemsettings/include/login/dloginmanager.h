// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#pragma once

#include "dlogintypes.h"

#include <dexpected.h>
#include <qobject.h>
#include <qscopedpointer.h>
#include <qsharedpointer.h>

DLOGIN_BEGIN_NAMESPACE
class DLoginSeat;
class DLoginUser;
class DLoginSession;
DLOGIN_END_NAMESPACE

using LoginExecStatus = DTK_LOGIN_NAMESPACE::ExecuteStatus;
using LoginSeatPtr = QSharedPointer<DTK_LOGIN_NAMESPACE::DLoginSeat>;
using LoginSessionPtr = QSharedPointer<DTK_LOGIN_NAMESPACE::DLoginSession>;
using LoginUserPtr = QSharedPointer<DTK_LOGIN_NAMESPACE::DLoginUser>;
using LoginInhibitorList = QList<DTK_LOGIN_NAMESPACE::Inhibitor>;
using LoginShutdownType = DTK_LOGIN_NAMESPACE::ShutdownType;
using LoginInhibitMode = DTK_LOGIN_NAMESPACE::InhibitMode;
using LoginSessionRole = DTK_LOGIN_NAMESPACE::SessionRole;

DLOGIN_BEGIN_NAMESPACE
using DCORE_NAMESPACE::DExpected;

class DLoginManagerPrivate;

class DLoginManager : public QObject
{
    Q_OBJECT
public:
    explicit DLoginManager(QObject *parent = nullptr);
    ~DLoginManager() override;

    Q_PROPERTY(QStringList killExcludeUsers READ killExcludeUsers);
    Q_PROPERTY(QStringList killOnlyUsers READ killOnlyUsers);
    Q_PROPERTY(bool docked READ docked);
    Q_PROPERTY(bool idleHint READ idleHint);
    Q_PROPERTY(bool killUserProcesses READ killUserProcesses);
    Q_PROPERTY(bool lidClosed READ lidClosed);
    Q_PROPERTY(bool preparingForShutdown READ preparingForShutdown);
    Q_PROPERTY(bool preparingForSleep READ preparingForSleep);
    Q_PROPERTY(bool removeIPC READ removeIPC);
    Q_PROPERTY(QString blockInhibited READ blockInhibited);
    Q_PROPERTY(QString delayInhibited READ delayInhibited);
    Q_PROPERTY(PowerAction handleHibernateKey READ handleHibernateKey);
    Q_PROPERTY(PowerAction handleLidSwitch READ handleLidSwitch);
    Q_PROPERTY(PowerAction handleLidSwitchDocked READ handleLidSwitchDocked);
    Q_PROPERTY(PowerAction handleLidSwitchExternalPower READ handleLidSwitchExternalPower);
    Q_PROPERTY(PowerAction handlePowerKey READ handlePowerKey);
    Q_PROPERTY(PowerAction handleSuspendKey READ handleSuspendKey);
    Q_PROPERTY(PowerAction idleAction READ idleAction);
    Q_PROPERTY(ScheduledShutdownValue scheduledShutdown READ scheduledShutdown);
    Q_PROPERTY(quint32 nAutoVTs READ nAutoVTs);
    Q_PROPERTY(quint64 holdoffTimeoutUSec READ holdoffTimeoutUSec);
    Q_PROPERTY(quint64 idleActionUSec READ idleActionUSec);
    Q_PROPERTY(QDateTime idleSinceHint READ idleSinceHint);
    Q_PROPERTY(quint64 idleSinceHintMonotonic READ idleSinceHintMonotonic);
    Q_PROPERTY(quint64 inhibitDelayMaxUSec READ inhibitDelayMaxUSec);
    Q_PROPERTY(quint64 inhibitorsMax READ inhibitorsMax);
    Q_PROPERTY(quint64 nCurrentInhibitors READ nCurrentInhibitors);
    Q_PROPERTY(quint64 nCurrentSessions READ nCurrentSessions);
    Q_PROPERTY(quint64 runtimeDirectoryInodesMax READ runtimeDirectoryInodesMax);
    Q_PROPERTY(quint64 runtimeDirectorySize READ runtimeDirectorySize);
    Q_PROPERTY(quint64 sessionsMax READ sessionsMax);
    Q_PROPERTY(quint64 userStopDelayUSec READ userStopDelayUSec);

    QStringList killExcludeUsers() const;
    QStringList killOnlyUsers() const;
    bool docked() const;
    bool idleHint() const;
    bool killUserProcesses() const;
    bool lidClosed() const;
    bool preparingForShutdown() const;
    bool preparingForSleep() const;
    bool removeIPC() const;
    QString blockInhibited() const;
    QString delayInhibited() const;
    PowerAction handleHibernateKey() const;
    PowerAction handleLidSwitch() const;
    PowerAction handleLidSwitchDocked() const;
    PowerAction handleLidSwitchExternalPower() const;
    PowerAction handlePowerKey() const;
    PowerAction handleSuspendKey() const;
    PowerAction idleAction() const;
    ScheduledShutdownValue scheduledShutdown() const;
    quint32 nAutoVTs() const;
    quint64 holdoffTimeoutUSec() const;
    quint64 idleActionUSec() const;
    QDateTime idleSinceHint() const;
    quint64 idleSinceHintMonotonic() const;
    quint64 inhibitDelayMaxUSec() const;
    quint64 inhibitorsMax() const;
    quint64 nCurrentInhibitors() const;
    quint64 nCurrentSessions() const;
    quint64 runtimeDirectoryInodesMax() const;
    quint64 runtimeDirectorySize() const;
    quint64 sessionsMax() const;
    quint64 userStopDelayUSec() const;

Q_SIGNALS:
    void prepareForShutdown(bool value);
    void prepareForSleep(bool value);
    void seatNew(const QString &seatId);
    void seatRemoved(const QString &seatId);
    void sessionNew(const QString &sessionId);
    void sessionRemoved(const QString &sessionId);
    void userNew(quint32 UID);
    void userRemoved(quint32 UID);

public Q_SLOTS:
    DExpected<void> activateSession(const QString &sessionId);
    DExpected<void> activateSessionOnSeat(const QString &sessionId, const QString &seatId);
    DExpected<LoginExecStatus> canHalt();
    DExpected<LoginExecStatus> canHibernate();
    DExpected<LoginExecStatus> canHybridSleep();
    DExpected<LoginExecStatus> canPowerOff();
    DExpected<LoginExecStatus> canReboot();
    DExpected<LoginExecStatus> canSuspend();
    DExpected<LoginExecStatus> canSuspendThenHibernate();
    DExpected<bool> cancelScheduledShutdown();
    DExpected<LoginSeatPtr> findSeatById(const QString &seatId);
    DExpected<LoginSessionPtr> findSessionById(const QString &sessionId);
    DExpected<LoginSessionPtr> findSessionByPID(quint32 PID);
    DExpected<LoginUserPtr> findUserById(quint32 UID);
    DExpected<LoginUserPtr> findUserByPID(quint32 PID);
    DExpected<void> halt(bool interactive = false);
    DExpected<void> hibernate(bool interactive = false);
    DExpected<void> hybridSleep(bool interactive = false);
    DExpected<int> inhibit(int what, const QString &who, const QString &why, LoginInhibitMode mode);
    DExpected<void> killSession(const QString &sessionId,
                                LoginSessionRole who,
                                qint32 signalNumber);
    DExpected<void> killUser(quint32 UID, qint32 signalNumber);
    DExpected<LoginInhibitorList> listInhibitors();
    DExpected<QStringList> listSeats();
    DExpected<QStringList> listSessions();
    DExpected<QList<quint32>> listUsers();
    DExpected<void> lockSession(const QString &sessionId);
    DExpected<void> powerOff(bool interactive = false);
    DExpected<void> reboot(bool interactive = false);
    DExpected<void> scheduleShutdown(LoginShutdownType type, const QDateTime &usec);
    DExpected<void> setUserLinger(quint32 UID, bool enable, bool interactive);
    DExpected<void> suspend(bool interactive = false);
    DExpected<void> suspendThenHibernate(bool interactive = false);
    DExpected<void> terminateSession(const QString &sessionId);
    DExpected<void> terminateUser(quint32 uid);
    DExpected<void> logout();
    DExpected<LoginSeatPtr> currentSeat();
    DExpected<LoginSessionPtr> currentSession();
    DExpected<LoginUserPtr> currentUser();

private:
    QScopedPointer<DLoginManagerPrivate> d_ptr;
    Q_DECLARE_PRIVATE(DLoginManager)
};

DLOGIN_END_NAMESPACE
