// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#pragma once
#include "dlogintypes.h"
#include "dlogintypes_p.h"

#include <ddbusinterface.h>
#include <qdbuspendingreply.h>
#include <qdbusunixfiledescriptor.h>
#include <qobject.h>

using DTK_CORE_NAMESPACE::DDBusInterface;
class QDBusObjectPath;

DLOGIN_BEGIN_NAMESPACE class Login1ManagerInterface : public QObject
{
    Q_OBJECT
public:
    static inline const char *staticInterfaceName() { return "org.freedesktop.login1.Manager"; }

    Login1ManagerInterface(const QString &service,
                           const QString &path,
                           QDBusConnection connection,
                           QObject *parent = nullptr);
    ~Login1ManagerInterface() override;

    Q_PROPERTY(QStringList bootLoaderEntries READ bootLoaderEntries);
    Q_PROPERTY(QStringList killExcludeUsers READ killExcludeUsers);
    Q_PROPERTY(QStringList killOnlyUsers READ killOnlyUsers);
    Q_PROPERTY(bool docked READ docked);
    Q_PROPERTY(bool enableWallMessages READ enableWallMessages WRITE setEnableWallMessages);
    Q_PROPERTY(bool idleHint READ idleHint);
    Q_PROPERTY(bool killUserProcesses READ killUserProcesses);
    Q_PROPERTY(bool lidClosed READ lidClosed);
    Q_PROPERTY(bool onExternalPower READ onExternalPower);
    Q_PROPERTY(bool preparingForShutdown READ preparingForShutdown);
    Q_PROPERTY(bool preparingForSleep READ preparingForSleep);
    Q_PROPERTY(bool rebootToFirmwareSetup READ rebootToFirmwareSetup);
    Q_PROPERTY(bool removeIPC READ removeIPC);
    Q_PROPERTY(QString blockInhibited READ blockInhibited);
    Q_PROPERTY(QString delayInhibited READ delayInhibited);
    Q_PROPERTY(QString handleHibernateKey READ handleHibernateKey);
    Q_PROPERTY(QString handleLidSwitch READ handleLidSwitch);
    Q_PROPERTY(QString handleLidSwitchDocked READ handleLidSwitchDocked);
    Q_PROPERTY(QString handleLidSwitchExternalPower READ handleLidSwitchExternalPower);
    Q_PROPERTY(QString handlePowerKey READ handlePowerKey);
    Q_PROPERTY(QString handleSuspendKey READ handleSuspendKey);
    Q_PROPERTY(QString idleAction READ idleAction);
    Q_PROPERTY(QString rebootParameter READ rebootParameter);
    Q_PROPERTY(QString rebootToBootLoaderEntry READ rebootToBootLoaderEntry);
    Q_PROPERTY(QString wallMessage READ wallMessage WRITE setWallMessage);
    Q_PROPERTY(DBusScheduledShutdownValue scheduledShutdown READ scheduledShutdown);
    Q_PROPERTY(quint32 nAutoVTs READ nAutoVTs);
    Q_PROPERTY(quint64 holdoffTimeoutUSec READ holdoffTimeoutUSec);
    Q_PROPERTY(quint64 idleActionUSec READ idleActionUSec);
    Q_PROPERTY(quint64 idleSinceHint READ idleSinceHint);
    Q_PROPERTY(quint64 idleSinceHintMonotonic READ idleSinceHintMonotonic);
    Q_PROPERTY(quint64 inhibitDelayMaxUSec READ inhibitDelayMaxUSec);
    Q_PROPERTY(quint64 inhibitorsMax READ inhibitorsMax);
    Q_PROPERTY(quint64 nCurrentInhibitors READ nCurrentInhibitors);
    Q_PROPERTY(quint64 nCurrentSessions READ nCurrentSessions);
    Q_PROPERTY(quint64 rebootToBootLoaderMenu READ rebootToBootLoaderMenu);
    Q_PROPERTY(quint64 runtimeDirectoryInodesMax READ runtimeDirectoryInodesMax);
    Q_PROPERTY(quint64 runtimeDirectorySize READ runtimeDirectorySize);
    Q_PROPERTY(quint64 sessionsMax READ sessionsMax);
    Q_PROPERTY(quint64 userStopDelayUSec READ userStopDelayUSec);

    QStringList bootLoaderEntries() const;
    QStringList killExcludeUsers() const;
    QStringList killOnlyUsers() const;
    bool docked() const;
    bool enableWallMessages() const;
    void setEnableWallMessages(bool enable);
    bool idleHint() const;
    bool killUserProcesses() const;
    bool lidClosed() const;
    bool onExternalPower() const;
    bool preparingForShutdown() const;
    bool preparingForSleep() const;
    bool rebootToFirmwareSetup() const;
    bool removeIPC() const;
    QString blockInhibited() const;
    QString delayInhibited() const;
    QString handleHibernateKey() const;
    QString handleLidSwitch() const;
    QString handleLidSwitchDocked() const;
    QString handleLidSwitchExternalPower() const;
    QString handlePowerKey() const;
    QString handleSuspendKey() const;
    QString idleAction() const;
    QString rebootParameter() const;
    QString rebootToBootLoaderEntry() const;
    QString wallMessage() const;
    void setWallMessage(const QString &message);
    DBusScheduledShutdownValue scheduledShutdown() const;
    quint32 nAutoVTs() const;
    quint64 holdoffTimeoutUSec() const;
    quint64 idleActionUSec() const;
    quint64 idleSinceHint() const;
    quint64 idleSinceHintMonotonic() const;
    quint64 inhibitDelayMaxUSec() const;
    quint64 inhibitorsMax() const;
    quint64 nCurrentInhibitors() const;
    quint64 nCurrentSessions() const;
    quint64 rebootToBootLoaderMenu() const;
    quint64 runtimeDirectoryInodesMax() const;
    quint64 runtimeDirectorySize() const;
    quint64 sessionsMax() const;
    quint64 userStopDelayUSec() const;

public slots:
    QDBusPendingReply<> activateSession(const QString &sessionId);
    QDBusPendingReply<> activateSessionOnSeat(const QString &sessionId, const QString &seatId);
    QDBusPendingReply<> attachDevice(const QString &seatId,
                                     const QString &sysfsPath,
                                     bool interactive);
    QDBusPendingReply<QString> canHalt();
    QDBusPendingReply<QString> canHibernate();
    QDBusPendingReply<QString> canHybridSleep();
    QDBusPendingReply<QString> canPowerOff();
    QDBusPendingReply<QString> canReboot();
    QDBusPendingReply<QString> canRebootParameter();
    QDBusPendingReply<QString> canRebootToBootLoaderEntry();
    QDBusPendingReply<QString> canRebootToBootLoaderMenu();
    QDBusPendingReply<QString> canRebootToFirmwareSetup();
    QDBusPendingReply<QString> canSuspend();
    QDBusPendingReply<QString> canSuspendThenHibernate();
    QDBusPendingReply<bool> cancelScheduledShutdown();
    QDBusPendingReply<> flushDevices(bool interactive);
    QDBusPendingReply<QDBusObjectPath> getSeat(const QString &seatId);
    QDBusPendingReply<QDBusObjectPath> getSession(const QString &sessionId);
    QDBusPendingReply<QDBusObjectPath> getSessionByPID(quint32 pid);
    QDBusPendingReply<QDBusObjectPath> getUser(quint32 UID);
    QDBusPendingReply<QDBusObjectPath> getUserByPID(quint32 pid);
    QDBusPendingReply<> halt(bool interactive);
    QDBusPendingReply<> haltWithFlags(quint64 flags);
    QDBusPendingReply<> hibernate(bool interactive);
    QDBusPendingReply<> hibernateWithFlags(quint64 flags);
    QDBusPendingReply<> hybridSleep(bool interactive);
    QDBusPendingReply<> hybridSleepWithFlags(quint64 flags);
    QDBusPendingReply<QDBusUnixFileDescriptor>
    inhibit(const QString &what, const QString &who, const QString &why, const QString &mode);
    QDBusPendingReply<> killSession(const QString &sessionId,
                                    const QString &who,
                                    qint32 signalNumber);
    QDBusPendingReply<> killUser(quint32 UID, qint32 signalName);
    QDBusPendingReply<QList<DBusInhibitor>> listInhibitors();
    QDBusPendingReply<QList<DTK_LOGIN_NAMESPACE::DBusSeat>> listSeats();
    QDBusPendingReply<QList<DTK_LOGIN_NAMESPACE::DBusSession>> listSessions();
    QDBusPendingReply<QList<DTK_LOGIN_NAMESPACE::DBusUser>> listUsers();
    QDBusPendingReply<> lockSession(const QString &sessionId);
    QDBusPendingReply<> lockSessions();
    QDBusPendingReply<> powerOff(bool interactive);
    QDBusPendingReply<> powerOffWithFlags(quint64 flags);
    QDBusPendingReply<> reboot(bool interactive);
    QDBusPendingReply<> rebootWithFlags(quint64 flags);
    QDBusPendingReply<> releaseSession(const QString &sessionId);
    QDBusPendingReply<> scheduleShutdown(const QString &type, quint64 usec);
    QDBusPendingReply<> setRebootParameter(const QString &parameter);
    QDBusPendingReply<> setRebootToBootLoaderEntry(const QString &bootLoaderEntry);
    QDBusPendingReply<> setRebootToBootLoaderMenu(quint64 timeout);
    QDBusPendingReply<> setRebootToFirmwareSetup(bool enable);
    QDBusPendingReply<> setUserLinger(quint32 UID, bool enable, bool interactive);
    QDBusPendingReply<> suspend(bool interactive);
    QDBusPendingReply<> suspendWithFlags(quint64 flags);
    QDBusPendingReply<> suspendThenHibernate(bool interactive);
    QDBusPendingReply<> suspendThenHibernateWithFlags(quint64 flags);
    QDBusPendingReply<> terminateSeat(const QString &seatId);
    QDBusPendingReply<> terminateSession(const QString &sessionId);
    QDBusPendingReply<> terminateUser(const quint32 UID);
    QDBusPendingReply<> unlockSession(const QString &sessionId);
    QDBusPendingReply<> unlockSessions();

Q_SIGNALS:
    void prepareForShutdown(bool value);
    void prepareForSleep(bool value);
    void seatNew(const QString &seatId, const QDBusObjectPath &seatPath);
    void seatRemoved(const QString &seatId, const QDBusObjectPath &seatPath);
    void sessionNew(const QString &sessionId, const QDBusObjectPath &sessionPath);
    void sessionRemoved(const QString &sessionId, const QDBusObjectPath &sessionPath);
    void userNew(quint32 UID, const QDBusObjectPath &seatPath);
    void userRemoved(quint32 UID, const QDBusObjectPath &seatPath);

private:
    DDBusInterface *m_interface;
};

DLOGIN_END_NAMESPACE
