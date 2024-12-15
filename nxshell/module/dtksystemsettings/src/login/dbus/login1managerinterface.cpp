// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "login1managerinterface.h"

#include "ddbusinterface.h"

#include <qdbusconnection.h>
#include <qdbusextratypes.h>
#include <qdbusunixfiledescriptor.h>
#include <qdebug.h>
#include <qmetatype.h>
DLOGIN_BEGIN_NAMESPACE

Login1ManagerInterface::Login1ManagerInterface(const QString &service,
                                               const QString &path,
                                               QDBusConnection connection,
                                               QObject *parent)
    : QObject(parent)
    , m_interface(new DDBusInterface(service, path, staticInterfaceName(), connection, this))
{
    DBusScheduledShutdownValue::registerMetaType();
    DBusInhibitor::registerMetaType();
    DBusSeat::registerMetaType();
    DBusSession::registerMetaType();
    DBusUser::registerMetaType();
    // Relay signals
    connection.connect(service,
                       path,
                       staticInterfaceName(),
                       "PrepareForShutdown",
                       this,
                       SIGNAL(prepareForShutdown(bool)));
    connection.connect(service,
                       path,
                       staticInterfaceName(),
                       "PrepareForSleep",
                       this,
                       SIGNAL(prepareForSleep(bool)));
    connection.connect(service,
                       path,
                       staticInterfaceName(),
                       "SeatNew",
                       this,
                       SIGNAL(seatNew(const QString &, const QDBusObjectPath &)));
    connection.connect(service,
                       path,
                       staticInterfaceName(),
                       "SeatRemoved",
                       this,
                       SIGNAL(seatRemoved(const QString &, const QDBusObjectPath &)));
    connection.connect(service,
                       path,
                       staticInterfaceName(),
                       "SessionNew",
                       this,
                       SIGNAL(sessionNew(const QString &, const QDBusObjectPath &)));
    connection.connect(service,
                       path,
                       staticInterfaceName(),
                       "SessionRemoved",
                       this,
                       SIGNAL(sessionRemoved(const QString &, const QDBusObjectPath &)));
    connection.connect(service,
                       path,
                       staticInterfaceName(),
                       "UserNew",
                       this,
                       SIGNAL(userNew(quint32, const QDBusObjectPath &)));
    connection.connect(service,
                       path,
                       staticInterfaceName(),
                       "UserRemoved",
                       this,
                       SIGNAL(userRemoved(quint32, const QDBusObjectPath &)));
}

Login1ManagerInterface::~Login1ManagerInterface() = default;

// properties

QStringList Login1ManagerInterface::bootLoaderEntries() const
{
    return qdbus_cast<QStringList>(m_interface->property("BootLoaderEntries"));
}

QStringList Login1ManagerInterface::killExcludeUsers() const
{
    return qdbus_cast<QStringList>(m_interface->property("KillExcludeUsers"));
}

QStringList Login1ManagerInterface::killOnlyUsers() const
{
    return qdbus_cast<QStringList>(m_interface->property("KillOnlyUsers"));
}

bool Login1ManagerInterface::docked() const
{
    return qdbus_cast<bool>(m_interface->property("Docked"));
}

bool Login1ManagerInterface::enableWallMessages() const
{
    return qdbus_cast<bool>(m_interface->property("EnableWallMessages"));
}

void Login1ManagerInterface::setEnableWallMessages(const bool enable)
{
    m_interface->setProperty("EnableWallMessages", QVariant::fromValue(enable));
}

bool Login1ManagerInterface::idleHint() const
{
    return qdbus_cast<bool>(m_interface->property("IdleHint"));
}

bool Login1ManagerInterface::killUserProcesses() const
{
    return qdbus_cast<bool>(m_interface->property("KillUserProcesses"));
}

bool Login1ManagerInterface::lidClosed() const
{
    return qdbus_cast<bool>(m_interface->property("LidClosed"));
}

bool Login1ManagerInterface::onExternalPower() const
{
    return qdbus_cast<bool>(m_interface->property("OnExternalPower"));
}

bool Login1ManagerInterface::preparingForShutdown() const
{
    return qdbus_cast<bool>(m_interface->property("PreparingForShutdown"));
}

bool Login1ManagerInterface::preparingForSleep() const
{
    return qdbus_cast<bool>(m_interface->property("PreparingForSleep"));
}

bool Login1ManagerInterface::rebootToFirmwareSetup() const
{
    return qdbus_cast<bool>(m_interface->property("RebootToFirmwareSetup"));
}

bool Login1ManagerInterface::removeIPC() const
{
    return qdbus_cast<bool>(m_interface->property("RemoveIPC"));
}

QString Login1ManagerInterface::blockInhibited() const
{
    return qdbus_cast<QString>(m_interface->property("BlockInhibited"));
}

QString Login1ManagerInterface::delayInhibited() const
{
    return qdbus_cast<QString>(m_interface->property("DelayInhibited"));
}

QString Login1ManagerInterface::handleHibernateKey() const
{
    return qdbus_cast<QString>(m_interface->property("HandleHibernateKey"));
}

QString Login1ManagerInterface::handleLidSwitch() const
{
    return qdbus_cast<QString>(m_interface->property("HandleLidSwitch"));
}

QString Login1ManagerInterface::handleLidSwitchDocked() const
{
    return qdbus_cast<QString>(m_interface->property("HandleLidSwitchDocked"));
}

QString Login1ManagerInterface::handleLidSwitchExternalPower() const
{
    return qdbus_cast<QString>(m_interface->property("HandleLidSwitchExternalPower"));
}

QString Login1ManagerInterface::handlePowerKey() const
{
    return qdbus_cast<QString>(m_interface->property("HandlePowerKey"));
}

QString Login1ManagerInterface::handleSuspendKey() const
{
    return qdbus_cast<QString>(m_interface->property("HandleSuspendKey"));
}

QString Login1ManagerInterface::idleAction() const
{
    return qdbus_cast<QString>(m_interface->property("IdleAction"));
}

QString Login1ManagerInterface::rebootParameter() const
{
    return qdbus_cast<QString>(m_interface->property("RebootParameter"));
}

QString Login1ManagerInterface::rebootToBootLoaderEntry() const
{
    return qdbus_cast<QString>(m_interface->property("RebootToBootLoaderEntry"));
}

QString Login1ManagerInterface::wallMessage() const
{
    return qdbus_cast<QString>(m_interface->property("WallMessage"));
}

void Login1ManagerInterface::setWallMessage(const QString &wallMessage)
{
    m_interface->setProperty("WallMessage", QVariant::fromValue(wallMessage));
}

DBusScheduledShutdownValue Login1ManagerInterface::scheduledShutdown() const
{
    return qdbus_cast<DBusScheduledShutdownValue>(m_interface->property("ScheduledShutdown"));
}

quint32 Login1ManagerInterface::nAutoVTs() const
{
    return qdbus_cast<quint32>(m_interface->property("NAutoVTs"));
}

quint64 Login1ManagerInterface::holdoffTimeoutUSec() const
{
    return qdbus_cast<quint64>(m_interface->property("HoldoffTimeoutUSec"));
}

quint64 Login1ManagerInterface::idleActionUSec() const
{
    return qdbus_cast<quint64>(m_interface->property("IdleActionUSec"));
}

quint64 Login1ManagerInterface::idleSinceHint() const
{
    return qdbus_cast<quint64>(m_interface->property("IdleSinceHint"));
}

quint64 Login1ManagerInterface::idleSinceHintMonotonic() const
{
    return qdbus_cast<quint64>(m_interface->property("IdleSinceHintMonotonic"));
}

quint64 Login1ManagerInterface::inhibitDelayMaxUSec() const
{
    return qdbus_cast<quint64>(m_interface->property("InhibitDelayMaxUSec"));
}

quint64 Login1ManagerInterface::inhibitorsMax() const
{
    return qdbus_cast<quint64>(m_interface->property("InhibitorsMax"));
}

quint64 Login1ManagerInterface::nCurrentInhibitors() const
{
    return qdbus_cast<quint64>(m_interface->property("NCurrentInhibitors"));
}

quint64 Login1ManagerInterface::nCurrentSessions() const
{
    return qdbus_cast<quint64>(m_interface->property("NCurrentSessions"));
}

quint64 Login1ManagerInterface::rebootToBootLoaderMenu() const
{
    return qdbus_cast<quint64>(m_interface->property("RebootToBootLoaderMenu"));
}

quint64 Login1ManagerInterface::runtimeDirectoryInodesMax() const
{
    return qdbus_cast<quint64>(m_interface->property("RuntimeDirectoryInodesMax"));
}

quint64 Login1ManagerInterface::runtimeDirectorySize() const
{
    return qdbus_cast<quint64>(m_interface->property("RuntimeDirectorySize"));
}

quint64 Login1ManagerInterface::sessionsMax() const
{
    return qdbus_cast<quint64>(m_interface->property("SessionsMax"));
}

quint64 Login1ManagerInterface::userStopDelayUSec() const
{
    return qdbus_cast<quint64>(m_interface->property("UserStopDelayUSec"));
}

QDBusPendingReply<> Login1ManagerInterface::activateSession(const QString &sessionId)
{
    QDBusPendingReply<> reply =
            m_interface->asyncCallWithArgumentList("ActivateSession",
                                                   { QVariant::fromValue(sessionId) });
    return reply;
}

QDBusPendingReply<> Login1ManagerInterface::activateSessionOnSeat(const QString &sessionId,
                                                                  const QString &seatId)
{
    QDBusPendingReply<> reply = m_interface->asyncCallWithArgumentList(
            "ActivateSessionOnSeat",
            { QVariant::fromValue(sessionId), QVariant::fromValue(seatId) });
    return reply;
}

QDBusPendingReply<> Login1ManagerInterface::attachDevice(const QString &seatId,
                                                         const QString &sysfsPath,
                                                         const bool interactive)
{
    QDBusPendingReply<> reply =
            m_interface->asyncCallWithArgumentList(QLatin1String("AttachDevice"),
                                                   { QVariant::fromValue(seatId),
                                                     QVariant::fromValue(sysfsPath),
                                                     QVariant::fromValue(interactive) });
    return reply;
}

QDBusPendingReply<QString> Login1ManagerInterface::canHalt()
{
    QDBusPendingReply<QString> reply = m_interface->asyncCall(QLatin1String("CanHalt"));
    return reply;
}

QDBusPendingReply<QString> Login1ManagerInterface::canHibernate()
{
    QDBusPendingReply<QString> reply = m_interface->asyncCall(QLatin1String("CanHibernate"));
    return reply;
}

QDBusPendingReply<QString> Login1ManagerInterface::canHybridSleep()
{
    QDBusPendingReply<QString> reply = m_interface->asyncCall(QLatin1String("CanHybridSleep"));
    return reply;
}

QDBusPendingReply<QString> Login1ManagerInterface::canPowerOff()
{
    QDBusPendingReply<QString> reply = m_interface->asyncCall(QLatin1String("CanPowerOff"));
    return reply;
}

QDBusPendingReply<QString> Login1ManagerInterface::canReboot()
{
    QDBusPendingReply<QString> reply = m_interface->asyncCall(QLatin1String("CanReboot"));
    return reply;
}

QDBusPendingReply<QString> Login1ManagerInterface::canRebootParameter()
{
    QDBusPendingReply<QString> reply = m_interface->asyncCall(QLatin1String("CanRebootParameter"));
    return reply;
}

QDBusPendingReply<QString> Login1ManagerInterface::canRebootToBootLoaderEntry()
{
    QDBusPendingReply<QString> reply =
            m_interface->asyncCall(QLatin1String("CanRebootToBootLoaderEntry"));
    return reply;
}

QDBusPendingReply<QString> Login1ManagerInterface::canRebootToBootLoaderMenu()
{
    QDBusPendingReply<QString> reply =
            m_interface->asyncCall(QLatin1String("CanRebootToBootLoaderMenu"));
    return reply;
}

QDBusPendingReply<QString> Login1ManagerInterface::canRebootToFirmwareSetup()
{
    QDBusPendingReply<QString> reply =
            m_interface->asyncCall(QLatin1String("CanRebootToFirmwareSetup"));
    return reply;
}

QDBusPendingReply<QString> Login1ManagerInterface::canSuspend()
{
    QDBusPendingReply<QString> reply = m_interface->asyncCall(QLatin1String("CanSuspend"));
    return reply;
}

QDBusPendingReply<QString> Login1ManagerInterface::canSuspendThenHibernate()
{
    QDBusPendingReply<QString> reply =
            m_interface->asyncCall(QLatin1String("CanSuspendThenHibernate"));
    return reply;
}

QDBusPendingReply<bool> Login1ManagerInterface::cancelScheduledShutdown()
{
    QDBusPendingReply<bool> reply =
            m_interface->asyncCall(QLatin1String("CancelScheduledShutdown"));
    return reply;
}

QDBusPendingReply<> Login1ManagerInterface::flushDevices(const bool interactive)
{
    QDBusPendingReply<> reply =
            m_interface->asyncCallWithArgumentList(QLatin1String("FlushDevices"), { interactive });
    return reply;
}

QDBusPendingReply<QDBusObjectPath> Login1ManagerInterface::getSeat(const QString &seatId)
{
    QDBusPendingReply<QDBusObjectPath> reply =
            m_interface->asyncCallWithArgumentList(QLatin1String("GetSeat"),
                                                   { QVariant::fromValue(seatId) });
    return reply;
}

QDBusPendingReply<QDBusObjectPath> Login1ManagerInterface::getSession(const QString &sessionId)
{
    QDBusPendingReply<QDBusObjectPath> reply =
            m_interface->asyncCallWithArgumentList(QLatin1String("GetSession"),
                                                   { QVariant::fromValue(sessionId) });
    return reply;
}

QDBusPendingReply<QDBusObjectPath> Login1ManagerInterface::getSessionByPID(const quint32 pid)
{
    QDBusPendingReply<QDBusObjectPath> reply =
            m_interface->asyncCallWithArgumentList(QLatin1String("GetSessionByPID"), { pid });
    return reply;
}

QDBusPendingReply<QDBusObjectPath> Login1ManagerInterface::getUser(const quint32 UID)
{
    QDBusPendingReply<QDBusObjectPath> reply =
            m_interface->asyncCallWithArgumentList(QLatin1String("GetUser"), { UID });
    return reply;
}

QDBusPendingReply<QDBusObjectPath> Login1ManagerInterface::getUserByPID(const quint32 pid)
{
    QDBusPendingReply<QDBusObjectPath> reply =
            m_interface->asyncCallWithArgumentList(QLatin1String("GetUserByPID"), { pid });
    return reply;
}

QDBusPendingReply<> Login1ManagerInterface::halt(const bool interactive)
{
    QDBusPendingReply<> reply =
            m_interface->asyncCallWithArgumentList(QLatin1String("Halt"),
                                                   { QVariant::fromValue(interactive) });
    return reply;
}

QDBusPendingReply<> Login1ManagerInterface::haltWithFlags(const quint64 flags)
{
    QDBusPendingReply<> reply =
            m_interface->asyncCallWithArgumentList(QLatin1String("HaltWithFlags"), { flags });
    return reply;
}

QDBusPendingReply<> Login1ManagerInterface::hibernate(const bool interactive)
{
    QDBusPendingReply<> reply =
            m_interface->asyncCallWithArgumentList(QLatin1String("Hibernate"), { interactive });
    return reply;
}

QDBusPendingReply<> Login1ManagerInterface::hibernateWithFlags(const quint64 flags)
{
    QDBusPendingReply<> reply =
            m_interface->asyncCallWithArgumentList(QLatin1String("HibernateWithFlags"), { flags });
    return reply;
}

QDBusPendingReply<> Login1ManagerInterface::hybridSleep(const bool interactive)
{
    QDBusPendingReply<> reply =
            m_interface->asyncCallWithArgumentList(QLatin1String("HybridSleep"), { interactive });
    return reply;
}

QDBusPendingReply<> Login1ManagerInterface::hybridSleepWithFlags(const quint64 flags)
{
    QDBusPendingReply<> reply =
            m_interface->asyncCallWithArgumentList(QLatin1String("HybridSleepWithFlags"),
                                                   { flags });
    return reply;
}

QDBusPendingReply<QDBusUnixFileDescriptor> Login1ManagerInterface::inhibit(const QString &what,
                                                                           const QString &who,
                                                                           const QString &why,
                                                                           const QString &mode)
{
    QDBusPendingReply<QDBusUnixFileDescriptor> reply =
            m_interface->asyncCallWithArgumentList(QLatin1String("Inhibit"),
                                                   { QVariant::fromValue(what),
                                                     QVariant::fromValue(who),
                                                     QVariant::fromValue(why),
                                                     QVariant::fromValue(mode) });
    return reply;
}

QDBusPendingReply<> Login1ManagerInterface::killSession(const QString &sessionId,
                                                        const QString &who,
                                                        qint32 signalNumber)
{
    QDBusPendingReply<> reply = m_interface->asyncCallWithArgumentList(
            QLatin1String("KillSession"),
            { QVariant::fromValue(sessionId), QVariant::fromValue(who), signalNumber });
    return reply;
}

QDBusPendingReply<> Login1ManagerInterface::killUser(const quint32 UID, qint32 signalName)
{
    QDBusPendingReply<> reply =
            m_interface->asyncCallWithArgumentList(QLatin1String("KillUser"), { UID, signalName });
    return reply;
}

QDBusPendingReply<QList<DBusInhibitor>> Login1ManagerInterface::listInhibitors()
{
    QDBusPendingReply<QList<DBusInhibitor>> reply =
            m_interface->asyncCall(QLatin1String("ListInhibitors"));
    return reply;
}

QDBusPendingReply<QList<DBusSeat>> Login1ManagerInterface::listSeats()
{
    QDBusPendingReply<QList<DBusSeat>> reply = m_interface->asyncCall(QLatin1String("ListSeats"));
    return reply;
}

QDBusPendingReply<QList<DBusSession>> Login1ManagerInterface::listSessions()
{
    QDBusPendingReply<QList<DBusSession>> reply =
            m_interface->asyncCall(QLatin1String("ListSessions"));
    return reply;
}

QDBusPendingReply<QList<DBusUser>> Login1ManagerInterface::listUsers()
{
    QDBusPendingReply<QList<DBusUser>> reply = m_interface->asyncCall(QLatin1String("ListUsers"));
    return reply;
}

QDBusPendingReply<> Login1ManagerInterface::lockSession(const QString &sessionId)
{
    QDBusPendingReply<> reply =
            m_interface->asyncCallWithArgumentList(QLatin1String("LockSession"),
                                                   { QVariant::fromValue(sessionId) });
    return reply;
}

QDBusPendingReply<> Login1ManagerInterface::lockSessions()
{
    QDBusPendingReply<> reply = m_interface->asyncCall(QLatin1String("LockSessions"));
    return reply;
}

QDBusPendingReply<> Login1ManagerInterface::powerOff(const bool interactive)
{
    QDBusPendingReply<> reply =
            m_interface->asyncCallWithArgumentList(QLatin1String("PowerOff"), { interactive });
    return reply;
}

QDBusPendingReply<> Login1ManagerInterface::powerOffWithFlags(const quint64 flags)
{
    QDBusPendingReply<> reply =
            m_interface->asyncCallWithArgumentList(QLatin1String("PowerOffWithFlags"), { flags });
    return reply;
}

QDBusPendingReply<> Login1ManagerInterface::reboot(const bool interactive)
{
    QDBusPendingReply<> reply =
            m_interface->asyncCallWithArgumentList(QLatin1String("Reboot"), { interactive });
    return reply;
}

QDBusPendingReply<> Login1ManagerInterface::rebootWithFlags(const quint64 flags)
{
    QDBusPendingReply<> reply =
            m_interface->asyncCallWithArgumentList(QLatin1String("RebootWithFlags"), { flags });
    return reply;
}

QDBusPendingReply<> Login1ManagerInterface::releaseSession(const QString &sessionId)
{
    QDBusPendingReply<> reply =
            m_interface->asyncCallWithArgumentList(QLatin1String("ReleaseSession"),
                                                   { QVariant::fromValue(sessionId) });
    return reply;
}

QDBusPendingReply<> Login1ManagerInterface::scheduleShutdown(const QString &type,
                                                             const quint64 usec)
{
    QDBusPendingReply<> reply =
            m_interface->asyncCallWithArgumentList(QLatin1String("ScheduleShutdown"),
                                                   { QVariant::fromValue(type), usec });
    return reply;
}

QDBusPendingReply<> Login1ManagerInterface::setRebootParameter(const QString &parameter)
{
    QDBusPendingReply<> reply =
            m_interface->asyncCallWithArgumentList(QLatin1String("SetRebootParameter"),
                                                   { QVariant::fromValue(parameter) });
    return reply;
}

QDBusPendingReply<>
Login1ManagerInterface::setRebootToBootLoaderEntry(const QString &bootLoaderEntry)
{
    QDBusPendingReply<> reply =
            m_interface->asyncCallWithArgumentList(QLatin1String("SetRebootToBootLoaderEntry"),
                                                   { QVariant::fromValue(bootLoaderEntry) });
    return reply;
}

QDBusPendingReply<> Login1ManagerInterface::setRebootToBootLoaderMenu(const quint64 timeout)
{
    QDBusPendingReply<> reply =
            m_interface->asyncCallWithArgumentList(QLatin1String("SetRebootToBootLoaderMenu"),
                                                   { timeout });
    return reply;
}

QDBusPendingReply<> Login1ManagerInterface::setRebootToFirmwareSetup(const bool enable)
{
    QDBusPendingReply reply =
            m_interface->asyncCallWithArgumentList(QLatin1String("SetRebootToFirmwareSetup"),
                                                   { enable });
    return reply;
}

QDBusPendingReply<> Login1ManagerInterface::setUserLinger(const quint32 UID,
                                                          const bool enable,
                                                          const bool interactive)
{
    QDBusPendingReply<> reply =
            m_interface->asyncCallWithArgumentList(QLatin1String("SetUserLinger"),
                                                   { UID, enable, interactive });
    return reply;
}

QDBusPendingReply<> Login1ManagerInterface::suspend(const bool interactive)
{
    QDBusPendingReply<> reply =
            m_interface->asyncCallWithArgumentList(QLatin1String("Suspend"), { interactive });
    return reply;
}

QDBusPendingReply<> Login1ManagerInterface::suspendWithFlags(const quint64 flags)
{
    QDBusPendingReply<> reply =
            m_interface->asyncCallWithArgumentList(QLatin1String("SuspendWithFlags"), { flags });
    return reply;
}

QDBusPendingReply<> Login1ManagerInterface::suspendThenHibernate(const bool interactive)
{
    QDBusPendingReply<> reply =
            m_interface->asyncCallWithArgumentList(QLatin1String("SuspendThenHibernate"),
                                                   { interactive });
    return reply;
}

QDBusPendingReply<> Login1ManagerInterface::suspendThenHibernateWithFlags(const quint64 flags)
{
    QDBusPendingReply<> reply =
            m_interface->asyncCallWithArgumentList(QLatin1String("SuspendThenHibernateWithFlags"),
                                                   { flags });
    return reply;
}

QDBusPendingReply<> Login1ManagerInterface::terminateSeat(const QString &seatId)
{
    QDBusPendingReply<> reply =
            m_interface->asyncCallWithArgumentList(QLatin1String("TerminateSeat"),
                                                   { QVariant::fromValue(seatId) });
    return reply;
}

QDBusPendingReply<> Login1ManagerInterface::terminateSession(const QString &sessionId)
{
    QDBusPendingReply<> reply =
            m_interface->asyncCallWithArgumentList(QLatin1String("TerminateSession"),
                                                   { QVariant::fromValue(sessionId) });
    return reply;
}

QDBusPendingReply<> Login1ManagerInterface::terminateUser(const quint32 UID)
{
    QDBusPendingReply<> reply =
            m_interface->asyncCallWithArgumentList(QLatin1String("TerminateUser"),
                                                   { QVariant::fromValue(UID) });
    return reply;
}

QDBusPendingReply<> Login1ManagerInterface::unlockSession(const QString &sessionId)
{
    QDBusPendingReply<> reply =
            m_interface->asyncCallWithArgumentList(QLatin1String("UnlockSession"),
                                                   { QVariant::fromValue(sessionId) });
    return reply;
}

QDBusPendingReply<> Login1ManagerInterface::unlockSessions()
{
    QDBusPendingReply reply = m_interface->asyncCall(QLatin1String("UnlockSessions"));
    return reply;
}

DLOGIN_END_NAMESPACE
