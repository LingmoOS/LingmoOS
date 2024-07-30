/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2016 Kai Uwe Broulik <kde@privat.broulik.de>
    SPDX-FileCopyrightText: 2021 Harald Sitter <sitter@kde.org>
*/

#include "statusnotifier_activationclosetimer.h"

#include <QDBusConnectionInterface>

#include <KIdleTime>

#include "drkonqi_debug.h"

void ActivationCloseTimer::refresh()
{
    qCDebug(DRKONQI_LOG) << m_idle << m_pendingActivations << m_closeTimer.isActive();
    if (m_idle) {
        m_closeTimer.stop();
    } else { // not idle
        if (m_pendingActivations > 0) { // going for auto activation instead of closing
            m_closeTimer.stop();
        } else if (!m_closeTimer.isActive()) { // otherwise we may auto close if not already running
            m_closeTimer.start();
        }
    }
}

void ActivationCloseTimer::watchIdle(IdleWatcher *idleWatcher)
{
    // if nobody bothered to look at the crash after 1 minute, just close
    m_closeTimer.setSingleShot(true);
    m_closeTimer.setInterval(m_closeTimeout);
    connect(&m_closeTimer, &QTimer::timeout, this, &ActivationCloseTimer::autoClose);
    m_closeTimer.start();

    // make sure the user doesn't miss the SNI by stopping the auto hide timer when the session becomes idle
    connect(idleWatcher, &IdleWatcher::idle, this, [this] {
        m_idle = true;
        refresh();
    });
    connect(idleWatcher, &IdleWatcher::notIdle, this, [this] {
        m_idle = false;
        refresh();
    });

    idleWatcher->start();
}

void ActivationCloseTimer::watchDBus(DBusServiceWatcher *watcher)
{
    // Should the SNI host implode and not return within 10s, automatically
    // open the dialog.
    // We are tracking the related Notifications service here, because actually
    // tracking the Host interface is fairly involved with no tangible advantage.

    auto activationTimer = new QTimer(this);
    activationTimer->setInterval(m_activationTimeout);
    connect(activationTimer, &QTimer::timeout, this, &ActivationCloseTimer::autoActivate);

    connect(watcher, &DBusServiceWatcher::serviceUnregistered, activationTimer, [this, activationTimer] {
        ++m_pendingActivations;
        activationTimer->start();
        refresh();
    });
    connect(watcher, &DBusServiceWatcher::serviceRegistered, activationTimer, [this, activationTimer] {
        if (m_pendingActivations > 0) {
            --m_pendingActivations;
        }
        activationTimer->stop();
        refresh();
    });

    watcher->start();
}

void ActivationCloseTimer::start(DBusServiceWatcher *dbusWatcher, IdleWatcher *idleWatcher)
{
    connect(this, &ActivationCloseTimer::autoActivate, this, [this] {
        m_closeTimer.stop(); // make double sure we don't close once activated!
    });
    watchDBus(dbusWatcher);
    watchIdle(idleWatcher);
}

void IdleWatcher::start()
{
    // make sure the user doesn't miss the SNI by stopping the auto hide timer when the session becomes idle
    const auto idleTime = 30s;
    const int idleId = KIdleTime::instance()->addIdleTimeout(int(std::chrono::milliseconds(idleTime).count()));
    connect(KIdleTime::instance(), static_cast<void (KIdleTime::*)(int, int)>(&KIdleTime::timeoutReached), this, [this, idleId](int id) {
        if (idleId == id) {
            Q_EMIT idle();
        }
        // this is apparently needed or else resumingFromIdle is never called
        KIdleTime::instance()->catchNextResumeEvent();
    });
    connect(KIdleTime::instance(), &KIdleTime::resumingFromIdle, this, &IdleWatcher::notIdle);

    if (std::chrono::milliseconds(KIdleTime::instance()->idleTime()) >= idleTime) {
        Q_EMIT idle();
    }
}

QList<QString> DBusServiceWatcher::serviceNames() const
{
    return m_serviceNames;
}

void DBusServiceWatcher::start()
{
    const QDBusConnection sessionBus = QDBusConnection::sessionBus();
    const QDBusConnectionInterface *sessionInterface = sessionBus.interface();
    Q_ASSERT(sessionInterface);

    m_watcher = new QDBusServiceWatcher(this);
    m_watcher->setConnection(sessionBus);
    m_watcher->setWatchMode(QDBusServiceWatcher::WatchForOwnerChange);
    for (const auto &serviceName : m_serviceNames) {
        m_watcher->addWatchedService(serviceName);

        // if not currently available queue the activation - this is in case the service isn't available *right now*
        // in which case we'll not get any registration events
        if (!sessionInterface->isServiceRegistered(serviceName)) {
            Q_EMIT serviceUnregistered();
        }
    }

    connect(m_watcher, &QDBusServiceWatcher::serviceUnregistered, this, &DBusServiceWatcher::serviceUnregistered);
    connect(m_watcher, &QDBusServiceWatcher::serviceRegistered, this, &DBusServiceWatcher::serviceRegistered);
}

void ActivationCloseTimer::setActivationTimeout(std::chrono::milliseconds timeout)
{
    m_activationTimeout = timeout;
}

void ActivationCloseTimer::setCloseTimeout(std::chrono::milliseconds timeout)
{
    m_closeTimeout = timeout;
}

#include "moc_statusnotifier_activationclosetimer.cpp"
