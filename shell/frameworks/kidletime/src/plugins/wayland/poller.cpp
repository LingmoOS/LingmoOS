/*
    SPDX-FileCopyrightText: 2021 David Edmundson <davidedmundson@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/
#include "poller.h"

#include <QDebug>
#include <QGuiApplication>
#include <QLoggingCategory>
#include <QWaylandClientExtensionTemplate>
#include <QtWaylandClientVersion>

#include "qwayland-ext-idle-notify-v1.h"
#include "qwayland-idle.h"

Q_DECLARE_LOGGING_CATEGORY(POLLER)
Q_LOGGING_CATEGORY(POLLER, "kf5idletime_wayland")

/*
 * Porting notes:
 * org_kde_kwin_idle refers to an early specific idle timeout protocol
 * the version ext_idle refers to an upstream stable protocol
 *
 * Pragmattically they're both the same, but we have to have two implementations for a while
 *
 * When a suitable amount of time passes (Plasma 5.24 being EOL) drop IdleTimeoutKwin and drop IdleManagerKwin as well as merge the abstract IdleTimeout class into the real implementation
 */

class IdleTimeout : public QObject
{
    Q_OBJECT
public:
    IdleTimeout() = default;
Q_SIGNALS:
    void idle();
    void resumeFromIdle();
};

class IdleTimeoutKwin : public IdleTimeout, public QtWayland::org_kde_kwin_idle_timeout
{
    Q_OBJECT
public:
    IdleTimeoutKwin(struct ::org_kde_kwin_idle_timeout *object)
        : IdleTimeout()
        , QtWayland::org_kde_kwin_idle_timeout(object)
    {}

    ~IdleTimeoutKwin()
    {
        if (qGuiApp) {
            release();
        }
    }

protected:
    void org_kde_kwin_idle_timeout_idle() override {
        Q_EMIT idle();
    }
    void org_kde_kwin_idle_timeout_resumed() override {
        Q_EMIT resumeFromIdle();
    }
};

class IdleTimeoutExt : public IdleTimeout, public QtWayland::ext_idle_notification_v1
{
    Q_OBJECT
public:
    IdleTimeoutExt(struct ::ext_idle_notification_v1 *object)
        : IdleTimeout()
        , QtWayland::ext_idle_notification_v1(object)
    {
    }

    ~IdleTimeoutExt()
    {
        if (qGuiApp) {
            destroy();
        }
    }

protected:
    void ext_idle_notification_v1_idled() override
    {
        Q_EMIT idle();
    }
    void ext_idle_notification_v1_resumed() override
    {
        Q_EMIT resumeFromIdle();
    }
};

class IdleManagerKwin : public QWaylandClientExtensionTemplate<IdleManagerKwin>, public QtWayland::org_kde_kwin_idle
{
public:
    IdleManagerKwin()
        : QWaylandClientExtensionTemplate<IdleManagerKwin>(1)
    {
        initialize();
    }
};

class IdleManagerExt : public QWaylandClientExtensionTemplate<IdleManagerExt>, public QtWayland::ext_idle_notifier_v1
{
public:
    IdleManagerExt()
        : QWaylandClientExtensionTemplate<IdleManagerExt>(1)
    {
        initialize();
    }
    ~IdleManagerExt()
    {
        if (qGuiApp && isActive()) {
            destroy();
        }
    }
};

Poller::Poller(QObject *parent)
    : KAbstractIdleTimePoller(parent)
    , m_idleManagerKwin(new IdleManagerKwin)
    , m_idleManagerExt(new IdleManagerExt)
{
}

Poller::~Poller() = default;

bool Poller::isAvailable()
{
    return m_idleManagerKwin->isActive() || m_idleManagerExt->isActive();
}

void Poller::addTimeout(int nextTimeout)
{
    if (m_timeouts.contains(nextTimeout)) {
        return;
    }

    auto timeout = createTimeout(nextTimeout);
    if (!timeout) {
        return;
    }

    connect(timeout, &IdleTimeout::idle, this, [this, nextTimeout] {
        Q_EMIT timeoutReached(nextTimeout);
    });
    connect(timeout, &IdleTimeout::resumeFromIdle, this, &Poller::resumingFromIdle);
    m_timeouts.insert(nextTimeout, QSharedPointer<IdleTimeout>(timeout));
}

void Poller::removeTimeout(int nextTimeout)
{
    m_timeouts.remove(nextTimeout);
}

QList<int> Poller::timeouts() const
{
    return QList<int>();
}

void Poller::catchIdleEvent()
{
    if (m_catchResumeTimeout) {
        // already setup
        return;
    }
    if (!isAvailable()) {
        return;
    }

    m_catchResumeTimeout.reset(createTimeout(0));
    if (!m_catchResumeTimeout) {
        return;
    }
    connect(m_catchResumeTimeout.get(), &IdleTimeout::resumeFromIdle, this, [this] {
        stopCatchingIdleEvents();
        Q_EMIT resumingFromIdle();
    });
}

void Poller::stopCatchingIdleEvents()
{
    m_catchResumeTimeout.reset();
}

int Poller::forcePollRequest()
{
    qCWarning(POLLER) << "This plugin does not support polling idle time";
    return 0;
}

void Poller::simulateUserActivity()
{
}

IdleTimeout* Poller::createTimeout(int timeout)
{
    auto waylandApp = qGuiApp->nativeInterface<QNativeInterface::QWaylandApplication>();
    if (!waylandApp) {
        return nullptr;
    }
    auto seat = waylandApp->seat();

    if (!seat) {
        return nullptr;
    }

    if (m_idleManagerExt->isActive()) {
        return new IdleTimeoutExt(m_idleManagerExt->get_idle_notification(timeout, seat));
    }
    if (m_idleManagerKwin->isActive()) {
        return new IdleTimeoutKwin(m_idleManagerKwin->get_idle_timeout(seat, timeout));
    }
    return nullptr;
}

#include "moc_poller.cpp"
#include "poller.moc"
