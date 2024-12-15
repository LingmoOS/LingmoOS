// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "login1managerservice.h"

#include "dlogintypes_p.h"

#include <qbytearray.h>
#include <qdbusconnection.h>
#include <qdbuserror.h>
#include <qdbusunixfiledescriptor.h>
#include <qdebug.h>
#include <qlist.h>
#include <qmap.h>
#include <qmetaobject.h>
#include <qstring.h>
#include <qstringlist.h>
#include <qvariant.h>

#include <QDBusObjectPath>
DLOGIN_BEGIN_NAMESPACE

/*
 * Implementation of adaptor class  Login1ManagerService
 */

Login1ManagerService::Login1ManagerService(const QString &service,
                                           const QString &path,
                                           QObject *parent)
    : QObject(parent)
    , m_service(service)
    , m_path(path)
{
    DBusScheduledShutdownValue::registerMetaType();
    DBusInhibitor::registerMetaType();
    DBusSeat::registerMetaType();
    DBusSession::registerMetaType();
    DBusUser::registerMetaType();
    registerService(m_service, m_path);
}

Login1ManagerService::~Login1ManagerService()
{
    unRegisterService();
}

bool Login1ManagerService::registerService(const QString &service, const QString &path)
{
    QDBusConnection connection = QDBusConnection::sessionBus();
    if (!connection.registerService(service)) {
        QString errorMsg = connection.lastError().message();
        if (errorMsg.isEmpty())
            errorMsg = "maybe it's running";

        qWarning() << QString("Can't register the %1 service, %2.").arg(service).arg(errorMsg);
        return false;
    }
    if (!connection.registerObject(path, this, QDBusConnection::ExportAllContents)) {
        qWarning() << QString("Can't register %1 the D-Bus object.").arg(path);
        return false;
    }
    return true;
}

void Login1ManagerService::unRegisterService()
{
    QDBusConnection connection = QDBusConnection::sessionBus();
    connection.unregisterObject(m_path);
    connection.unregisterService(m_service);
}

QString Login1ManagerService::blockInhibited() const
{
    return m_blockInhibited;
}

QString Login1ManagerService::delayInhibited() const
{
    return m_delayInhibited;
}

bool Login1ManagerService::docked() const
{
    return m_docked;
}

QString Login1ManagerService::handleHibernateKey() const
{
    return m_handleHibernateKey;
}

QString Login1ManagerService::handleLidSwitch() const
{
    return m_handleLidSwitch;
}

QString Login1ManagerService::handleLidSwitchDocked() const
{
    return m_handleLidSwitchDocked;
}

QString Login1ManagerService::handleLidSwitchExternalPower() const
{
    return m_handleLidSwitchExternalPower;
}

QString Login1ManagerService::handlePowerKey() const
{
    return m_handlePowerKey;
}

QString Login1ManagerService::handleSuspendKey() const
{
    return m_handleSuspendKey;
}

quint64 Login1ManagerService::holdoffTimeoutUSec() const
{
    return m_holdoffTimeoutUSec;
}

QString Login1ManagerService::idleAction() const
{
    return m_idleAction;
}

quint64 Login1ManagerService::idleActionUSec() const
{
    return m_idleActionUSec;
}

bool Login1ManagerService::idleHint() const
{
    return m_idleHint;
}

quint64 Login1ManagerService::idleSinceHint() const
{
    return m_idleSinceHint;
}

quint64 Login1ManagerService::idleSinceHintMonotonic() const
{
    return m_idleSinceHintMonotonic;
}

quint64 Login1ManagerService::inhibitDelayMaxUSec() const
{
    return m_inhibitDelayMaxUSec;
}

quint64 Login1ManagerService::inhibitorsMax() const
{
    return m_inhibitorsMax;
}

QStringList Login1ManagerService::killExcludeUsers() const
{
    return m_killExcludeUsers;
}

QStringList Login1ManagerService::killOnlyUsers() const
{
    return m_killOnlyUsers;
}

bool Login1ManagerService::killUserProcesses() const
{
    return m_killUserProcesses;
}

bool Login1ManagerService::lidClosed() const
{
    return m_lidClosed;
}

quint32 Login1ManagerService::nAutoVTs() const
{
    return m_nAutoVTs;
}

quint64 Login1ManagerService::nCurrentInhibitors() const
{
    return m_nCurrentInhibitors;
}

quint64 Login1ManagerService::nCurrentSessions() const
{
    return m_nCurrentSessions;
}

bool Login1ManagerService::preparingForShutdown() const
{
    return m_preparingForShutdown;
}

bool Login1ManagerService::preparingForSleep() const
{
    return m_preparingForSleep;
}

bool Login1ManagerService::removeIPC() const
{
    return m_removeIPC;
}

quint64 Login1ManagerService::runtimeDirectoryInodesMax() const
{
    return m_runtimeDirectoryInodesMax;
}

quint64 Login1ManagerService::runtimeDirectorySize() const
{
    return m_runtimeDirectorySize;
}

DBusScheduledShutdownValue Login1ManagerService::scheduledShutdown() const
{
    return m_scheduledShutdown;
}

quint64 Login1ManagerService::sessionsMax() const
{
    return m_sessionsMax;
}

quint64 Login1ManagerService::userStopDelayUSec() const
{
    return m_userStopDelayUSec;
}

void Login1ManagerService::ActivateSession(const QString &sessionId)
{
    m_sessionId = sessionId;
}

void Login1ManagerService::ActivateSessionOnSeat(const QString &sessionId, const QString &seatId)
{
    m_sessionId = sessionId;
    m_seatId = seatId;
}

QString Login1ManagerService::CanHalt()
{
    return m_canHalt;
}

QString Login1ManagerService::CanHibernate()
{
    return m_canHibernate;
}

QString Login1ManagerService::CanHybridSleep()
{
    return m_canHybridSleep;
}

QString Login1ManagerService::CanPowerOff()
{
    return m_canPowerOff;
}

QString Login1ManagerService::CanReboot()
{
    return m_canReboot;
}

QString Login1ManagerService::CanSuspend()
{
    return m_canSuspend;
}

QString Login1ManagerService::CanSuspendThenHibernate()
{
    return m_canSuspendThenHibernate;
}

bool Login1ManagerService::CancelScheduledShutdown()
{
    return m_cancelScheduledShutdown;
}

QDBusObjectPath Login1ManagerService::GetSeat(const QString &seatId)
{
    m_seatId = seatId;
    return m_seatPath;
}

QDBusObjectPath Login1ManagerService::GetSession(const QString &sessionId)
{
    m_sessionId = sessionId;
    return m_sessionPath;
}

QDBusObjectPath Login1ManagerService::GetSessionByPID(quint32 PID)
{
    m_PID = PID;
    return m_sessionPath;
}

QDBusObjectPath Login1ManagerService::GetUser(quint32 UID)
{
    m_UID = UID;
    return m_userPath;
}

QDBusObjectPath Login1ManagerService::GetUserByPID(quint32 PID)
{
    m_PID = PID;
    return m_userPath;
}

void Login1ManagerService::Halt(bool interactive)
{
    m_haltInteractive = interactive;
}

void Login1ManagerService::Hibernate(bool interactive)
{
    m_hibernateInteractive = interactive;
}

void Login1ManagerService::HybridSleep(bool interactive)
{
    m_hybridSleepInteractive = interactive;
}

QDBusUnixFileDescriptor Login1ManagerService::Inhibit(const QString &what,
                                                      const QString &who,
                                                      const QString &why,
                                                      const QString &mode)
{
    m_inhibitor.what = what;
    m_inhibitor.who = who;
    m_inhibitor.why = why;
    m_inhibitor.mode = mode;
    return m_inhibitFileDescriptor;
}

void Login1ManagerService::KillSession(const QString &sessionId,
                                       const QString &who,
                                       qint32 signalNumber)
{
    m_sessionId = sessionId;
    m_sessionRole = who;
    m_signalNumber = signalNumber;
}

void Login1ManagerService::KillUser(quint32 UID, qint32 signalNumber)
{
    m_UID = UID;
    m_signalNumber = signalNumber;
}

QList<DBusInhibitor> Login1ManagerService::ListInhibitors()
{
    return m_inhibitors;
}

QList<DBusSeat> Login1ManagerService::ListSeats()
{
    return m_seats;
}

QList<DBusSession> Login1ManagerService::ListSessions()
{
    return m_sessions;
}

QList<DBusUser> Login1ManagerService::ListUsers()
{
    return m_users;
}

void Login1ManagerService::LockSession(const QString &session_id)
{
    m_sessionId = session_id;
}

void Login1ManagerService::LockSessions()
{
    m_lockSessions = true;
}

void Login1ManagerService::PowerOff(bool interactive)
{
    m_powerOffInteractive = interactive;
}

void Login1ManagerService::Reboot(bool interactive)
{
    m_rebootInteractive = interactive;
}

void Login1ManagerService::ScheduleShutdown(const QString &type, quint64 usec)
{
    m_scheduledShutdown.type = type;
    m_scheduledShutdown.usec = usec;
}

void Login1ManagerService::SetUserLinger(quint32 uid, bool enable, bool interactive)
{
    m_UID = uid;
    m_userLinger = enable;
    m_userLingerInteractive = interactive;
}

void Login1ManagerService::Suspend(bool interactive)
{
    m_suspendInteractive = interactive;
}

void Login1ManagerService::SuspendThenHibernate(bool interactive)
{
    m_suspendThenHibernateInteractive = interactive;
}

void Login1ManagerService::TerminateSeat(const QString &seatId)
{
    m_seatId = seatId;
}

void Login1ManagerService::TerminateSession(const QString &sessionId)
{
    m_sessionId = sessionId;
}

void Login1ManagerService::TerminateUser(quint32 UID)
{
    m_UID = UID;
}

DLOGIN_END_NAMESPACE
