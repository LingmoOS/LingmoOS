/*
    SPDX-FileCopyrightText: 2015 Martin Gräßlin <mgraesslin@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/
#include "idle.h"
#include "event_queue.h"
#include "seat.h"
#include "wayland_pointer_p.h"

#include <wayland-idle-client-protocol.h>

namespace KWayland
{
namespace Client
{
class Q_DECL_HIDDEN Idle::Private
{
public:
    WaylandPointer<org_kde_kwin_idle, org_kde_kwin_idle_destroy> manager;
    EventQueue *queue = nullptr;
};

Idle::Idle(QObject *parent)
    : QObject(parent)
    , d(new Private)
{
}

Idle::~Idle()
{
    release();
}

void Idle::release()
{
    d->manager.release();
}

void Idle::destroy()
{
    d->manager.destroy();
}

bool Idle::isValid() const
{
    return d->manager.isValid();
}

void Idle::setup(org_kde_kwin_idle *manager)
{
    Q_ASSERT(manager);
    Q_ASSERT(!d->manager.isValid());
    d->manager.setup(manager);
}

EventQueue *Idle::eventQueue()
{
    return d->queue;
}

void Idle::setEventQueue(EventQueue *queue)
{
    d->queue = queue;
}

IdleTimeout *Idle::getTimeout(quint32 msecs, Seat *seat, QObject *parent)
{
    Q_ASSERT(isValid());
    Q_ASSERT(seat);
    IdleTimeout *idle = new IdleTimeout(parent);
    auto i = org_kde_kwin_idle_get_idle_timeout(d->manager, *seat, msecs);
    if (d->queue) {
        d->queue->addProxy(i);
    }
    idle->setup(i);
    return idle;
}

Idle::operator org_kde_kwin_idle *() const
{
    return d->manager;
}

Idle::operator org_kde_kwin_idle *()
{
    return d->manager;
}

class Q_DECL_HIDDEN IdleTimeout::Private
{
public:
    explicit Private(IdleTimeout *q);
    void setup(org_kde_kwin_idle_timeout *d);

    WaylandPointer<org_kde_kwin_idle_timeout, org_kde_kwin_idle_timeout_release> timeout;

private:
    static void idleCallback(void *data, org_kde_kwin_idle_timeout *org_kde_kwin_idle_timeout);
    static void resumedCallback(void *data, org_kde_kwin_idle_timeout *org_kde_kwin_idle_timeout);
    static const struct org_kde_kwin_idle_timeout_listener s_listener;

    IdleTimeout *q;
};

const org_kde_kwin_idle_timeout_listener IdleTimeout::Private::s_listener = {idleCallback, resumedCallback};

void IdleTimeout::Private::idleCallback(void *data, org_kde_kwin_idle_timeout *org_kde_kwin_idle_timeout)
{
    Q_UNUSED(org_kde_kwin_idle_timeout)
    Q_EMIT reinterpret_cast<Private *>(data)->q->idle();
}

void IdleTimeout::Private::resumedCallback(void *data, org_kde_kwin_idle_timeout *org_kde_kwin_idle_timeout)
{
    Q_UNUSED(org_kde_kwin_idle_timeout)
    Q_EMIT reinterpret_cast<Private *>(data)->q->resumeFromIdle();
}

IdleTimeout::Private::Private(IdleTimeout *q)
    : q(q)
{
}

void IdleTimeout::Private::setup(org_kde_kwin_idle_timeout *d)
{
    Q_ASSERT(d);
    Q_ASSERT(!timeout.isValid());
    timeout.setup(d);
    org_kde_kwin_idle_timeout_add_listener(timeout, &s_listener, this);
}

IdleTimeout::IdleTimeout(QObject *parent)
    : QObject(parent)
    , d(new Private(this))
{
}

IdleTimeout::~IdleTimeout()
{
    release();
}

void IdleTimeout::destroy()
{
    d->timeout.destroy();
}

void IdleTimeout::release()
{
    d->timeout.release();
}

bool IdleTimeout::isValid() const
{
    return d->timeout.isValid();
}

void IdleTimeout::setup(org_kde_kwin_idle_timeout *dataDevice)
{
    d->setup(dataDevice);
}

void IdleTimeout::simulateUserActivity()
{
    Q_ASSERT(isValid());
    org_kde_kwin_idle_timeout_simulate_user_activity(d->timeout);
}

IdleTimeout::operator org_kde_kwin_idle_timeout *()
{
    return d->timeout;
}

IdleTimeout::operator org_kde_kwin_idle_timeout *() const
{
    return d->timeout;
}

}
}
