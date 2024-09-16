// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dloginmanager.h"

#include "dloginmanager_p.h"
#include "dloginseat.h"
#include "dloginsession.h"
#include "dloginuser.h"
#include "dloginutils.h"
#include "login1managerinterface.h"

#include <qdatetime.h>
#include <qdbusconnection.h>
#include <qdbusextratypes.h>
#include <qdbuspendingreply.h>
#include <qdbusunixfiledescriptor.h>
#include <qdebug.h>
#include <qglobal.h>
#include <qlist.h>
#include <qstringliteral.h>

using DTK_CORE_NAMESPACE::DError;
using DTK_CORE_NAMESPACE::DUnexpected;

DLOGIN_BEGIN_NAMESPACE
DLoginManager::DLoginManager(QObject *parent)
    : QObject(parent)
    , d_ptr(new DLoginManagerPrivate(this))
{
#if defined(USE_FAKE_INTERFACE) // for unit test
    const QString &Service = QStringLiteral("org.freedesktop.fakelogin1");
    const QString &Path = QStringLiteral("/org/freedesktop/login1/manager");
    QDBusConnection connection = QDBusConnection::sessionBus();
#else
    const QString &Service = QStringLiteral("org.freedesktop.login1");
    const QString &Path = QStringLiteral("/org/freedesktop/login1");
    QDBusConnection connection = QDBusConnection::systemBus();
#endif
    Q_D(DLoginManager);
    DBusScheduledShutdownValue::registerMetaType();
    DBusInhibitor::registerMetaType();
    DBusSeat::registerMetaType();
    DBusSession::registerMetaType();
    DBusUser::registerMetaType();
    qRegisterMetaType<ShutdownType>("ShutdownType");
    qRegisterMetaType<ScheduledShutdownValue>("ScheduledShutdownValue");
    qRegisterMetaType<PowerAction>("PowerAction");
    qRegisterMetaType<ExecuteStatus>("ExecuteStatus");
    qRegisterMetaType<SessionRole>("SessionRole");
    qRegisterMetaType<InhibitMode>("InhibitMode");
    qRegisterMetaType<Inhibitor>("Inhibitor");
    Utils::registerAllStringConverter();
    d->m_inter = new Login1ManagerInterface(Service, Path, connection, d);
    connect(d->m_inter,
            &Login1ManagerInterface::prepareForShutdown,
            this,
            &DLoginManager::prepareForShutdown);
    connect(d->m_inter,
            &Login1ManagerInterface::prepareForSleep,
            this,
            &DLoginManager::prepareForSleep);
    connect(d->m_inter, &Login1ManagerInterface::seatNew, this, &DLoginManager::seatNew);
    connect(d->m_inter, &Login1ManagerInterface::seatRemoved, this, &DLoginManager::seatRemoved);
    connect(d->m_inter, &Login1ManagerInterface::sessionNew, this, &DLoginManager::sessionNew);
    connect(d->m_inter,
            &Login1ManagerInterface::sessionRemoved,
            this,
            &DLoginManager::sessionRemoved);
    connect(d->m_inter, &Login1ManagerInterface::userNew, this, &DLoginManager::userNew);
    connect(d->m_inter, &Login1ManagerInterface::userRemoved, this, &DLoginManager::userRemoved);
}

DLoginManager::~DLoginManager() = default;

// properties

QStringList DLoginManager::killExcludeUsers() const
{
    Q_D(const DLoginManager);
    return d->m_inter->killExcludeUsers();
}

QStringList DLoginManager::killOnlyUsers() const
{
    Q_D(const DLoginManager);
    return d->m_inter->killOnlyUsers();
}

bool DLoginManager::docked() const
{
    Q_D(const DLoginManager);
    return d->m_inter->docked();
}

bool DLoginManager::idleHint() const
{
    Q_D(const DLoginManager);
    return d->m_inter->idleHint();
}

bool DLoginManager::killUserProcesses() const
{
    Q_D(const DLoginManager);
    return d->m_inter->killUserProcesses();
}

bool DLoginManager::lidClosed() const
{
    Q_D(const DLoginManager);
    return d->m_inter->lidClosed();
}

bool DLoginManager::preparingForShutdown() const
{
    Q_D(const DLoginManager);
    return d->m_inter->preparingForShutdown();
}

bool DLoginManager::preparingForSleep() const
{
    Q_D(const DLoginManager);
    return d->m_inter->preparingForSleep();
}

bool DLoginManager::removeIPC() const
{
    Q_D(const DLoginManager);
    return d->m_inter->removeIPC();
}

QString DLoginManager::blockInhibited() const
{
    Q_D(const DLoginManager);
    return d->m_inter->blockInhibited();
}

QString DLoginManager::delayInhibited() const
{
    Q_D(const DLoginManager);
    return d->m_inter->delayInhibited();
}

PowerAction DLoginManager::handleHibernateKey() const
{
    Q_D(const DLoginManager);
    return Utils::stringToAction(d->m_inter->handleHibernateKey());
}

PowerAction DLoginManager::handleLidSwitch() const
{
    Q_D(const DLoginManager);
    return Utils::stringToAction(d->m_inter->handleLidSwitch());
}

PowerAction DLoginManager::handleLidSwitchDocked() const
{
    Q_D(const DLoginManager);
    return Utils::stringToAction(d->m_inter->handleLidSwitchDocked());
}

PowerAction DLoginManager::handleLidSwitchExternalPower() const
{
    Q_D(const DLoginManager);
    return Utils::stringToAction(d->m_inter->handleLidSwitchExternalPower());
}

PowerAction DLoginManager::handlePowerKey() const
{
    Q_D(const DLoginManager);
    return Utils::stringToAction(d->m_inter->handlePowerKey());
}

PowerAction DLoginManager::handleSuspendKey() const
{
    Q_D(const DLoginManager);
    return Utils::stringToAction(d->m_inter->handleSuspendKey());
}

PowerAction DLoginManager::idleAction() const
{
    Q_D(const DLoginManager);
    return Utils::stringToAction(d->m_inter->idleAction());
}

ScheduledShutdownValue DLoginManager::scheduledShutdown() const
{
    Q_D(const DLoginManager);
    const auto &result = d->m_inter->scheduledShutdown();
    ScheduledShutdownValue value;
    value.type = Utils::stringToShutdownType(result.type);
    value.time = QDateTime::fromMSecsSinceEpoch(result.usec / 1000);
    return value;
}

quint32 DLoginManager::nAutoVTs() const
{
    Q_D(const DLoginManager);
    return d->m_inter->nAutoVTs();
}

quint64 DLoginManager::holdoffTimeoutUSec() const
{
    Q_D(const DLoginManager);
    return d->m_inter->holdoffTimeoutUSec();
}

quint64 DLoginManager::idleActionUSec() const
{
    Q_D(const DLoginManager);
    return d->m_inter->idleActionUSec();
}

QDateTime DLoginManager::idleSinceHint() const
{
    Q_D(const DLoginManager);
    return QDateTime::fromMSecsSinceEpoch(d->m_inter->idleSinceHint() / 1000);
}

quint64 DLoginManager::idleSinceHintMonotonic() const
{
    Q_D(const DLoginManager);
    return d->m_inter->idleSinceHintMonotonic();
}

quint64 DLoginManager::inhibitDelayMaxUSec() const
{
    Q_D(const DLoginManager);
    return d->m_inter->inhibitDelayMaxUSec();
}

quint64 DLoginManager::inhibitorsMax() const
{
    Q_D(const DLoginManager);
    return d->m_inter->inhibitorsMax();
}

quint64 DLoginManager::nCurrentInhibitors() const
{
    Q_D(const DLoginManager);
    return d->m_inter->nCurrentInhibitors();
}

quint64 DLoginManager::nCurrentSessions() const
{
    Q_D(const DLoginManager);
    return d->m_inter->nCurrentSessions();
}

quint64 DLoginManager::runtimeDirectoryInodesMax() const
{
    Q_D(const DLoginManager);
    return d->m_inter->runtimeDirectoryInodesMax();
}

quint64 DLoginManager::runtimeDirectorySize() const
{
    Q_D(const DLoginManager);
    return d->m_inter->runtimeDirectorySize();
}

quint64 DLoginManager::sessionsMax() const
{
    Q_D(const DLoginManager);
    return d->m_inter->sessionsMax();
}

quint64 DLoginManager::userStopDelayUSec() const
{
    Q_D(const DLoginManager);
    return d->m_inter->userStopDelayUSec();
}

// public slots

DExpected<void> DLoginManager::activateSession(const QString &sessionId)
{
    Q_D(DLoginManager);
    QDBusPendingReply<> reply = d->m_inter->activateSession(sessionId);
    reply.waitForFinished();
    if (!reply.isValid()) {
        return DUnexpected<>{ DError{ reply.error().type(), reply.error().message() } };
    }
    return {};
}

DExpected<void> DLoginManager::activateSessionOnSeat(const QString &sessionId,
                                                     const QString &seatId)
{
    Q_D(DLoginManager);
    QDBusPendingReply<> reply = d->m_inter->activateSessionOnSeat(sessionId, seatId);
    reply.waitForFinished();
    if (!reply.isValid()) {
        return DUnexpected<>{ DError{ reply.error().type(), reply.error().message() } };
    }
    return {};
}

DExpected<ExecuteStatus> DLoginManager::canHalt()
{
    Q_D(DLoginManager);
    QDBusPendingReply<QString> reply = d->m_inter->canHalt();
    reply.waitForFinished();
    if (!reply.isValid()) {
        return DUnexpected<>{ DError{ reply.error().type(), reply.error().message() } };
    }
    return Utils::stringToStatus(reply.value());
}

DExpected<DTK_LOGIN_NAMESPACE::ExecuteStatus> DLoginManager::canHibernate()
{
    Q_D(DLoginManager);
    QDBusPendingReply<QString> reply = d->m_inter->canHibernate();
    reply.waitForFinished();
    if (!reply.isValid()) {
        return DUnexpected<>{ DError{ reply.error().type(), reply.error().message() } };
    }
    return Utils::stringToStatus(reply.value());
}

DExpected<DTK_LOGIN_NAMESPACE::ExecuteStatus> DLoginManager::canHybridSleep()
{
    Q_D(DLoginManager);
    QDBusPendingReply<QString> reply = d->m_inter->canHybridSleep();
    reply.waitForFinished();
    if (!reply.isValid()) {
        return DUnexpected<>{ DError{ reply.error().type(), reply.error().message() } };
    }
    return Utils::stringToStatus(reply.value());
}

DExpected<DTK_LOGIN_NAMESPACE::ExecuteStatus> DLoginManager::canPowerOff()
{
    Q_D(DLoginManager);
    QDBusPendingReply<QString> reply = d->m_inter->canPowerOff();
    reply.waitForFinished();
    if (!reply.isValid()) {
        return DUnexpected<>{ DError{ reply.error().type(), reply.error().message() } };
    }
    return Utils::stringToStatus(reply.value());
}

DExpected<DTK_LOGIN_NAMESPACE::ExecuteStatus> DLoginManager::canReboot()
{
    Q_D(DLoginManager);
    QDBusPendingReply<QString> reply = d->m_inter->canReboot();
    reply.waitForFinished();
    if (!reply.isValid()) {
        return DUnexpected<>{ DError{ reply.error().type(), reply.error().message() } };
    }
    return Utils::stringToStatus(reply.value());
}

DExpected<DTK_LOGIN_NAMESPACE::ExecuteStatus> DLoginManager::canSuspend()
{
    Q_D(DLoginManager);
    QDBusPendingReply<QString> reply = d->m_inter->canSuspend();
    reply.waitForFinished();
    if (!reply.isValid()) {
        return DUnexpected<>{ DError{ reply.error().type(), reply.error().message() } };
    }
    return Utils::stringToStatus(reply.value());
}

DExpected<DTK_LOGIN_NAMESPACE::ExecuteStatus> DLoginManager::canSuspendThenHibernate()
{
    Q_D(DLoginManager);
    QDBusPendingReply<QString> reply = d->m_inter->canSuspendThenHibernate();
    reply.waitForFinished();
    if (!reply.isValid()) {
        return DUnexpected<>{ DError{ reply.error().type(), reply.error().message() } };
    }
    return Utils::stringToStatus(reply.value());
}

DExpected<bool> DLoginManager::cancelScheduledShutdown()
{
    Q_D(DLoginManager);
    QDBusPendingReply<bool> reply = d->m_inter->cancelScheduledShutdown();
    reply.waitForFinished();
    if (!reply.isValid()) {
        return DUnexpected<>{ DError{ reply.error().type(), reply.error().message() } };
    }
    return reply.value();
}

DExpected<LoginSeatPtr> DLoginManager::findSeatById(const QString &seatId)
{
    Q_D(DLoginManager);
    QDBusPendingReply<QDBusObjectPath> reply = d->m_inter->getSeat(seatId);
    reply.waitForFinished();
    if (!reply.isValid()) {
        return DUnexpected<>{ DError{ reply.error().type(), reply.error().message() } };
    }
    return QSharedPointer<DLoginSeat>(new DLoginSeat(reply.value().path()));
}

DExpected<LoginSessionPtr> DLoginManager::findSessionById(const QString &sessionId)
{
    Q_D(DLoginManager);
    QDBusPendingReply<QDBusObjectPath> reply = d->m_inter->getSession(sessionId);
    reply.waitForFinished();
    if (!reply.isValid()) {
        return DUnexpected<>{ DError{ reply.error().type(), reply.error().message() } };
    }
    return QSharedPointer<DLoginSession>(new DLoginSession(reply.value().path()));
}

DExpected<LoginSessionPtr> DLoginManager::findSessionByPID(quint32 PID)
{
    Q_D(DLoginManager);
    QDBusPendingReply<QDBusObjectPath> reply = d->m_inter->getSessionByPID(PID);
    reply.waitForFinished();
    if (!reply.isValid()) {
        return DUnexpected<>{ DError{ reply.error().type(), reply.error().message() } };
    }
    return QSharedPointer<DLoginSession>(new DLoginSession(reply.value().path()));
}

DExpected<LoginUserPtr> DLoginManager::findUserById(quint32 UID)
{
    Q_D(DLoginManager);
    QDBusPendingReply<QDBusObjectPath> reply = d->m_inter->getUser(UID);
    reply.waitForFinished();
    if (!reply.isValid()) {
        return DUnexpected<>{ DError{ reply.error().type(), reply.error().message() } };
    }
    return QSharedPointer<DLoginUser>(new DLoginUser(reply.value().path()));
}

DExpected<LoginUserPtr> DLoginManager::findUserByPID(quint32 PID)
{
    Q_D(DLoginManager);
    QDBusPendingReply<QDBusObjectPath> reply = d->m_inter->getUserByPID(PID);
    reply.waitForFinished();
    if (!reply.isValid()) {
        return DUnexpected<>{ DError{ reply.error().type(), reply.error().message() } };
    }
    return QSharedPointer<DLoginUser>(new DLoginUser(reply.value().path()));
}

DExpected<void> DLoginManager::halt(bool interactive)
{
    Q_D(DLoginManager);
    QDBusPendingReply<> reply = d->m_inter->halt(interactive);
    reply.waitForFinished();
    if (!reply.isValid()) {
        return DUnexpected<>{ DError{ reply.error().type(), reply.error().message() } };
    }
    return {};
}

DExpected<void> DLoginManager::hibernate(bool interactive)
{
    Q_D(DLoginManager);
    QDBusPendingReply<> reply = d->m_inter->hibernate(interactive);
    reply.waitForFinished();
    if (!reply.isValid()) {
        return DUnexpected<>{ DError{ reply.error().type(), reply.error().message() } };
    }
    return {};
}

DExpected<void> DLoginManager::hybridSleep(bool interactive)
{
    Q_D(DLoginManager);
    QDBusPendingReply<> reply = d->m_inter->hybridSleep(interactive);
    reply.waitForFinished();
    if (!reply.isValid()) {
        return DUnexpected<>{ DError{ reply.error().type(), reply.error().message() } };
    }
    return {};
}

DExpected<int>
DLoginManager::inhibit(int what, const QString &who, const QString &why, InhibitMode mode)
{
    Q_D(DLoginManager);
    QDBusPendingReply<QDBusUnixFileDescriptor> reply =
            d->m_inter->inhibit(Utils::decodeBehavior(what), who, why, Utils::modeToString(mode));
    reply.waitForFinished();
    if (!reply.isValid()) {
        return DUnexpected<>{ DError{ reply.error().type(), reply.error().message() } };
    }
    return reply.value().takeFileDescriptor();
}

DExpected<void> DLoginManager::killSession(const QString &sessionId,
                                           SessionRole who,
                                           qint32 signalNumber)
{
    Q_D(DLoginManager);
    QDBusPendingReply<> reply =
            d->m_inter->killSession(sessionId, Utils::sessionRoleToString(who), signalNumber);
    reply.waitForFinished();
    if (!reply.isValid()) {
        return DUnexpected<>{ DError{ reply.error().type(), reply.error().message() } };
    }
    return {};
}

DExpected<void> DLoginManager::killUser(quint32 UID, qint32 signalNumber)
{
    Q_D(DLoginManager);
    QDBusPendingReply<> reply = d->m_inter->killUser(UID, signalNumber);
    reply.waitForFinished();
    if (!reply.isValid()) {
        return DUnexpected<>{ DError{ reply.error().type(), reply.error().message() } };
    }
    return {};
}

DExpected<LoginInhibitorList> DLoginManager::listInhibitors()
{
    Q_D(DLoginManager);
    QDBusPendingReply<QList<DBusInhibitor>> reply = d->m_inter->listInhibitors();
    reply.waitForFinished();
    QList<Inhibitor> inhibitors;
    if (!reply.isValid()) {
        return DUnexpected<>{ DError{ reply.error().type(), reply.error().message() } };
    }
    for (auto &&inhibitorDBus : reply.value()) {
        inhibitors.append(Utils::inhibitorFromDBus(inhibitorDBus));
    }
    return inhibitors;
}

DExpected<QStringList> DLoginManager::listSeats()
{
    Q_D(DLoginManager);
    QDBusPendingReply<QList<DBusSeat>> reply = d->m_inter->listSeats();
    reply.waitForFinished();
    QStringList seats;
    if (!reply.isValid()) {
        return DUnexpected<>{ DError{ reply.error().type(), reply.error().message() } };
    }
    for (auto &&seatDBus : reply.value()) {
        seats.append(seatDBus.seatId);
    }
    return seats;
}

DExpected<QStringList> DLoginManager::listSessions()
{
    Q_D(DLoginManager);
    QDBusPendingReply<QList<DBusSession>> reply = d->m_inter->listSessions();
    reply.waitForFinished();
    QStringList sessions;
    if (!reply.isValid()) {
        return DUnexpected<>{ DError{ reply.error().type(), reply.error().message() } };
    }
    for (auto &&sessionDBus : reply.value()) {
        sessions.append(sessionDBus.sessionId);
    }
    return sessions;
}

DExpected<QList<quint32>> DLoginManager::listUsers()
{
    Q_D(DLoginManager);
    QDBusPendingReply<QList<DBusUser>> reply = d->m_inter->listUsers();
    reply.waitForFinished();
    QList<quint32> users;
    if (!reply.isValid()) {
        return DUnexpected<>{ DError{ reply.error().type(), reply.error().message() } };
    }
    for (auto &&userDBus : reply.value()) {
        users.append(userDBus.userId);
    }
    return users;
}

DExpected<void> DLoginManager::lockSession(const QString &sessionId)
{
    Q_D(DLoginManager);
    QDBusPendingReply<> reply = d->m_inter->lockSession(sessionId);
    reply.waitForFinished();
    if (!reply.isValid()) {
        return DUnexpected<>{ DError{ reply.error().type(), reply.error().message() } };
    }
    return {};
}

DExpected<void> DLoginManager::powerOff(bool interactive)
{
    Q_D(DLoginManager);
    QDBusPendingReply<> reply = d->m_inter->powerOff(interactive);
    reply.waitForFinished();
    if (!reply.isValid()) {
        return DUnexpected<>{ DError{ reply.error().type(), reply.error().message() } };
    }
    return {};
}

DExpected<void> DLoginManager::reboot(bool interactive)
{
    Q_D(DLoginManager);
    QDBusPendingReply<> reply = d->m_inter->reboot(interactive);
    reply.waitForFinished();
    if (!reply.isValid()) {
        return DUnexpected<>{ DError{ reply.error().type(), reply.error().message() } };
    }
    return {};
}

DExpected<void> DLoginManager::scheduleShutdown(ShutdownType type, const QDateTime &usec)
{
    Q_D(DLoginManager);
    QDBusPendingReply<> reply = d->m_inter->scheduleShutdown(Utils::shutdownTypeToString(type),
                                                             usec.toMSecsSinceEpoch() * 1000);
    reply.waitForFinished();
    if (!reply.isValid()) {
        return DUnexpected<>{ DError{ reply.error().type(), reply.error().message() } };
    }
    return {};
}

DExpected<void> DLoginManager::setUserLinger(quint32 UID, bool enable, bool interactive)
{
    Q_D(DLoginManager);
    QDBusPendingReply<> reply = d->m_inter->setUserLinger(UID, enable, interactive);
    reply.waitForFinished();
    if (!reply.isValid()) {
        return DUnexpected<>{ DError{ reply.error().type(), reply.error().message() } };
    }
    return {};
}

DExpected<void> DLoginManager::suspend(bool interactive)
{
    Q_D(DLoginManager);
    QDBusPendingReply<> reply = d->m_inter->suspend(interactive);
    reply.waitForFinished();
    if (!reply.isValid()) {
        return DUnexpected<>{ DError{ reply.error().type(), reply.error().message() } };
    }
    return {};
}

DExpected<void> DLoginManager::suspendThenHibernate(bool interactive)
{
    Q_D(DLoginManager);
    QDBusPendingReply<> reply = d->m_inter->suspendThenHibernate(interactive);
    reply.waitForFinished();
    if (!reply.isValid()) {
        return DUnexpected<>{ DError{ reply.error().type(), reply.error().message() } };
    }
    return {};
}

DExpected<void> DLoginManager::terminateSession(const QString &sessionId)
{
    Q_D(DLoginManager);
    QDBusPendingReply<> reply = d->m_inter->terminateSession(sessionId);
    reply.waitForFinished();
    if (!reply.isValid()) {
        return DUnexpected<>{ DError{ reply.error().type(), reply.error().message() } };
    }
    return {};
}

DExpected<void> DLoginManager::terminateUser(quint32 uid)
{
    Q_D(DLoginManager);
    QDBusPendingReply<> reply = d->m_inter->terminateUser(uid);
    reply.waitForFinished();
    if (!reply.isValid()) {
        return DUnexpected<>{ DError{ reply.error().type(), reply.error().message() } };
    }
    return {};
}

DExpected<void> DLoginManager::logout()
{
    auto eSession = currentSession();
    if (eSession) {
        eSession.value()->terminate();
        return {};
    } else {
        return DUnexpected{ eSession.error() };
    }
}

DExpected<LoginSeatPtr> DLoginManager::currentSeat()
{
    DLoginSeat current(QStringLiteral("/org/freedesktop/login1/seat/self"));
    auto eSeat = findSeatById(current.id());
    if (eSeat) {
        return eSeat.value();
    } else {
        return DUnexpected{ eSeat.error() };
    }
}

DExpected<LoginSessionPtr> DLoginManager::currentSession()
{
    DLoginSession current(QStringLiteral("/org/freedesktop/login1/session/self"));
    auto eSession = findSessionById(current.id());
    if (eSession) {
        return eSession.value();
    } else {
        return DUnexpected{ eSession.error() };
    }
}

DExpected<LoginUserPtr> DLoginManager::currentUser()
{
    DLoginUser current(QStringLiteral("/org/freedesktop/login1/user/self"));
    auto eUser = findUserById(current.UID());
    if (eUser) {
        return eUser.value();
    } else {
        return DUnexpected{ eUser.error() };
    }
}

DLOGIN_END_NAMESPACE
