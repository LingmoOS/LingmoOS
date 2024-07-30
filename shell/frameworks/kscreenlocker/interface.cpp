/*
SPDX-FileCopyrightText: 1999 Martin R. Jones <mjones@kde.org>
SPDX-FileCopyrightText: 2011 Martin Gräßlin <mgraesslin@kde.org>

SPDX-License-Identifier: GPL-2.0-or-later
*/
#include "interface.h"
#include "kscreensaveradaptor.h"
#include "ksldapp.h"
#include "powerdevilpolicyagent.h"
#include "screensaveradaptor.h"
// KDE
#include <KAuthorized>
#include <KIdleTime>
#include <KWindowSystem>
// Qt
#include <QDBusConnection>
#include <QDBusContext>
#include <QDBusReply>
#include <QDBusServiceWatcher>
#include <QRandomGenerator>

namespace ScreenLocker
{
const uint ChangeScreenSettings = 4;

Interface::Interface(KSldApp *parent)
    : QObject(parent)
    , m_daemon(parent)
    , m_serviceWatcher(new QDBusServiceWatcher(this))
    , m_next_cookie(0)
{
    (void)new ScreenSaverAdaptor(this);
    QDBusConnection::sessionBus().registerService(QStringLiteral("org.freedesktop.ScreenSaver"));
    (void)new KScreenSaverAdaptor(this);
    QDBusConnection::sessionBus().registerService(QStringLiteral("org.kde.screensaver"));
    QDBusConnection::sessionBus().registerObject(QStringLiteral("/ScreenSaver"), this);
    QDBusConnection::sessionBus().registerObject(QStringLiteral("/org/freedesktop/ScreenSaver"), this);
    connect(m_daemon, &KSldApp::locked, this, &Interface::slotLocked);
    connect(m_daemon, &KSldApp::unlocked, this, &Interface::slotUnlocked);
    connect(m_daemon, &KSldApp::aboutToLock, this, &Interface::AboutToLock);

    m_serviceWatcher->setConnection(QDBusConnection::sessionBus());
    m_serviceWatcher->setWatchMode(QDBusServiceWatcher::WatchForUnregistration);
    connect(m_serviceWatcher, &QDBusServiceWatcher::serviceUnregistered, this, &Interface::serviceUnregistered);

    // I make it a really random number to avoid
    // some assumptions in clients, but just increase
    // while gnome-ss creates a random number every time
    m_next_cookie = QRandomGenerator::global()->bounded(19999);
}

Interface::~Interface()
{
}

bool Interface::GetActive()
{
    return m_daemon->lockState() == KSldApp::Locked;
}

uint Interface::GetActiveTime()
{
    return m_daemon->activeTime();
}

uint Interface::GetSessionIdleTime()
{
    if (KWindowSystem::isPlatformWayland()) {
        QDBusContext::sendErrorReply(QDBusError::NotSupported, QStringLiteral("GetSessionIdleTime is not supported on this platform"));
        return 0;
    }
    return KIdleTime::instance()->idleTime();
}

void Interface::Lock()
{
    if (!KAuthorized::authorizeAction(QStringLiteral("lock_screen"))) {
        return;
    }
    m_daemon->lock(calledFromDBus() ? EstablishLock::Immediate : EstablishLock::Delayed);

    if (calledFromDBus() && m_daemon->lockState() == KSldApp::AcquiringLock) {
        m_lockReplies << message().createReply();
        setDelayedReply(true);
    }
}

void Interface::SwitchUser()
{
    if (!KAuthorized::authorizeAction(QStringLiteral("lock_screen"))) {
        return;
    }
    m_daemon->lock(EstablishLock::DefaultToSwitchUser);

    if (calledFromDBus() && m_daemon->lockState() == KSldApp::AcquiringLock) {
        m_lockReplies << message().createReply();
        setDelayedReply(true);
    }
}

bool Interface::SetActive(bool state)
{
    // TODO: what should the return value be?
    if (state) {
        Lock();
        return true;
    }
    // set inactive is ignored
    return false;
}

uint Interface::Inhibit(const QString &application_name, const QString &reason_for_inhibit)
{
    OrgKdeSolidPowerManagementPolicyAgentInterface policyAgent(QStringLiteral("org.kde.Solid.PowerManagement.PolicyAgent"),
                                                               QStringLiteral("/org/kde/Solid/PowerManagement/PolicyAgent"),
                                                               QDBusConnection::sessionBus());
    QDBusReply<uint> reply = policyAgent.AddInhibition(ChangeScreenSettings, application_name, reason_for_inhibit);

    InhibitRequest sr;
    sr.cookie = m_next_cookie++;
    sr.dbusid = message().service();
    sr.powerdevilcookie = reply.isValid() ? reply : 0;
    m_requests.append(sr);
    m_serviceWatcher->addWatchedService(sr.dbusid);
    KSldApp::self()->inhibit();
    return sr.cookie;
}

void Interface::UnInhibit(uint cookie)
{
    QMutableListIterator<InhibitRequest> it(m_requests);
    while (it.hasNext()) {
        if (it.next().cookie == cookie) {
            if (uint powerdevilcookie = it.value().powerdevilcookie) {
                OrgKdeSolidPowerManagementPolicyAgentInterface policyAgent(QStringLiteral("org.kde.Solid.PowerManagement.PolicyAgent"),
                                                                           QStringLiteral("/org/kde/Solid/PowerManagement/PolicyAgent"),
                                                                           QDBusConnection::sessionBus());
                policyAgent.ReleaseInhibition(powerdevilcookie);
            }
            it.remove();
            KSldApp::self()->uninhibit();
            break;
        }
    }
}

void Interface::serviceUnregistered(const QString &name)
{
    m_serviceWatcher->removeWatchedService(name);
    QListIterator<InhibitRequest> it(m_requests);
    while (it.hasNext()) {
        const InhibitRequest &r = it.next();
        if (r.dbusid == name) {
            UnInhibit(r.cookie);
        }
    }
}

void Interface::SimulateUserActivity()
{
    KIdleTime::instance()->simulateUserActivity();
}

uint Interface::Throttle(const QString &application_name, const QString &reason_for_inhibit)
{
    Q_UNUSED(application_name)
    Q_UNUSED(reason_for_inhibit)
    // TODO: implement me
    return 0;
}

void Interface::UnThrottle(uint cookie)
{
    Q_UNUSED(cookie)
    // TODO: implement me
}

void Interface::slotLocked()
{
    sendLockReplies();
    Q_EMIT ActiveChanged(true);
}

void Interface::slotUnlocked()
{
    sendLockReplies();
    Q_EMIT ActiveChanged(false);
}

void Interface::configure()
{
    m_daemon->configure();
}

void Interface::sendLockReplies()
{
    for (const QDBusMessage &reply : std::as_const(m_lockReplies)) {
        QDBusConnection::sessionBus().send(reply);
    }

    m_lockReplies.clear();
}

} // namespace

#include "moc_interface.cpp"
