/*
    SPDX-FileCopyrightText: 2016 Martin Gräßlin <mgraesslin@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/
#include "pointerconstraints.h"
#include "event_queue.h"
#include "pointer.h"
#include "region.h"
#include "surface.h"
#include "wayland_pointer_p.h"

#include <wayland-pointer-constraints-unstable-v1-client-protocol.h>

namespace KWayland
{
namespace Client
{
class Q_DECL_HIDDEN PointerConstraints::Private
{
public:
    Private() = default;

    void setup(zwp_pointer_constraints_v1 *arg);

    WaylandPointer<zwp_pointer_constraints_v1, zwp_pointer_constraints_v1_destroy> pointerconstraints;
    EventQueue *queue = nullptr;
};

PointerConstraints::PointerConstraints(QObject *parent)
    : QObject(parent)
    , d(new Private)
{
}

void PointerConstraints::Private::setup(zwp_pointer_constraints_v1 *arg)
{
    Q_ASSERT(arg);
    Q_ASSERT(!pointerconstraints);
    pointerconstraints.setup(arg);
}

PointerConstraints::~PointerConstraints()
{
    release();
}

void PointerConstraints::setup(zwp_pointer_constraints_v1 *pointerconstraints)
{
    d->setup(pointerconstraints);
}

void PointerConstraints::release()
{
    d->pointerconstraints.release();
}

void PointerConstraints::destroy()
{
    d->pointerconstraints.destroy();
}

PointerConstraints::operator zwp_pointer_constraints_v1 *()
{
    return d->pointerconstraints;
}

PointerConstraints::operator zwp_pointer_constraints_v1 *() const
{
    return d->pointerconstraints;
}

bool PointerConstraints::isValid() const
{
    return d->pointerconstraints.isValid();
}

void PointerConstraints::setEventQueue(EventQueue *queue)
{
    d->queue = queue;
}

EventQueue *PointerConstraints::eventQueue()
{
    return d->queue;
}

LockedPointer *PointerConstraints::lockPointer(Surface *surface, Pointer *pointer, Region *region, LifeTime lifetime, QObject *parent)
{
    Q_ASSERT(isValid());
    auto p = new LockedPointer(parent);
    zwp_pointer_constraints_v1_lifetime lf;
    switch (lifetime) {
    case LifeTime::OneShot:
        lf = ZWP_POINTER_CONSTRAINTS_V1_LIFETIME_ONESHOT;
        break;
    case LifeTime::Persistent:
        lf = ZWP_POINTER_CONSTRAINTS_V1_LIFETIME_PERSISTENT;
        break;
    default:
        Q_UNREACHABLE();
        break;
    }
    wl_region *wr = nullptr;
    if (region) {
        wr = *region;
    }
    auto w = zwp_pointer_constraints_v1_lock_pointer(d->pointerconstraints, *surface, *pointer, wr, lf);
    if (d->queue) {
        d->queue->addProxy(w);
    }
    p->setup(w);
    return p;
}

ConfinedPointer *PointerConstraints::confinePointer(Surface *surface, Pointer *pointer, Region *region, LifeTime lifetime, QObject *parent)
{
    Q_ASSERT(isValid());
    auto p = new ConfinedPointer(parent);
    zwp_pointer_constraints_v1_lifetime lf;
    switch (lifetime) {
    case LifeTime::OneShot:
        lf = ZWP_POINTER_CONSTRAINTS_V1_LIFETIME_ONESHOT;
        break;
    case LifeTime::Persistent:
        lf = ZWP_POINTER_CONSTRAINTS_V1_LIFETIME_PERSISTENT;
        break;
    default:
        Q_UNREACHABLE();
        break;
    }
    wl_region *wr = nullptr;
    if (region) {
        wr = *region;
    }
    auto w = zwp_pointer_constraints_v1_confine_pointer(d->pointerconstraints, *surface, *pointer, wr, lf);
    if (d->queue) {
        d->queue->addProxy(w);
    }
    p->setup(w);
    return p;
}

class Q_DECL_HIDDEN LockedPointer::Private
{
public:
    Private(LockedPointer *q);

    void setup(zwp_locked_pointer_v1 *arg);

    WaylandPointer<zwp_locked_pointer_v1, zwp_locked_pointer_v1_destroy> lockedpointer;

private:
    LockedPointer *q;

private:
    static void lockedCallback(void *data, zwp_locked_pointer_v1 *zwp_locked_pointer_v1);
    static void unlockedCallback(void *data, zwp_locked_pointer_v1 *zwp_locked_pointer_v1);

    static const zwp_locked_pointer_v1_listener s_listener;
};

const zwp_locked_pointer_v1_listener LockedPointer::Private::s_listener = {lockedCallback, unlockedCallback};

void LockedPointer::Private::lockedCallback(void *data, zwp_locked_pointer_v1 *zwp_locked_pointer_v1)
{
    auto p = reinterpret_cast<LockedPointer::Private *>(data);
    Q_ASSERT(p->lockedpointer == zwp_locked_pointer_v1);
    Q_EMIT p->q->locked();
}

void LockedPointer::Private::unlockedCallback(void *data, zwp_locked_pointer_v1 *zwp_locked_pointer_v1)
{
    auto p = reinterpret_cast<LockedPointer::Private *>(data);
    Q_ASSERT(p->lockedpointer == zwp_locked_pointer_v1);
    Q_EMIT p->q->unlocked();
}

LockedPointer::Private::Private(LockedPointer *q)
    : q(q)
{
}

LockedPointer::LockedPointer(QObject *parent)
    : QObject(parent)
    , d(new Private(this))
{
}

void LockedPointer::Private::setup(zwp_locked_pointer_v1 *arg)
{
    Q_ASSERT(arg);
    Q_ASSERT(!lockedpointer);
    lockedpointer.setup(arg);
    zwp_locked_pointer_v1_add_listener(lockedpointer, &s_listener, this);
}

LockedPointer::~LockedPointer()
{
    release();
}

void LockedPointer::setup(zwp_locked_pointer_v1 *lockedpointer)
{
    d->setup(lockedpointer);
}

void LockedPointer::release()
{
    d->lockedpointer.release();
}

void LockedPointer::destroy()
{
    d->lockedpointer.destroy();
}

LockedPointer::operator zwp_locked_pointer_v1 *()
{
    return d->lockedpointer;
}

LockedPointer::operator zwp_locked_pointer_v1 *() const
{
    return d->lockedpointer;
}

bool LockedPointer::isValid() const
{
    return d->lockedpointer.isValid();
}

void LockedPointer::setCursorPositionHint(const QPointF &surfaceLocal)
{
    Q_ASSERT(isValid());
    zwp_locked_pointer_v1_set_cursor_position_hint(d->lockedpointer, wl_fixed_from_double(surfaceLocal.x()), wl_fixed_from_double(surfaceLocal.y()));
}

void LockedPointer::setRegion(Region *region)
{
    Q_ASSERT(isValid());
    wl_region *wr = nullptr;
    if (region) {
        wr = *region;
    }
    zwp_locked_pointer_v1_set_region(d->lockedpointer, wr);
}

class Q_DECL_HIDDEN ConfinedPointer::Private
{
public:
    Private(ConfinedPointer *q);

    void setup(zwp_confined_pointer_v1 *arg);

    WaylandPointer<zwp_confined_pointer_v1, zwp_confined_pointer_v1_destroy> confinedpointer;

private:
    ConfinedPointer *q;

private:
    static void confinedCallback(void *data, zwp_confined_pointer_v1 *zwp_confined_pointer_v1);
    static void unconfinedCallback(void *data, zwp_confined_pointer_v1 *zwp_confined_pointer_v1);

    static const zwp_confined_pointer_v1_listener s_listener;
};

const zwp_confined_pointer_v1_listener ConfinedPointer::Private::s_listener = {confinedCallback, unconfinedCallback};

void ConfinedPointer::Private::confinedCallback(void *data, zwp_confined_pointer_v1 *zwp_confined_pointer_v1)
{
    auto p = reinterpret_cast<ConfinedPointer::Private *>(data);
    Q_ASSERT(p->confinedpointer == zwp_confined_pointer_v1);
    Q_EMIT p->q->confined();
}

void ConfinedPointer::Private::unconfinedCallback(void *data, zwp_confined_pointer_v1 *zwp_confined_pointer_v1)
{
    auto p = reinterpret_cast<ConfinedPointer::Private *>(data);
    Q_ASSERT(p->confinedpointer == zwp_confined_pointer_v1);
    Q_EMIT p->q->unconfined();
}

ConfinedPointer::Private::Private(ConfinedPointer *q)
    : q(q)
{
}

ConfinedPointer::ConfinedPointer(QObject *parent)
    : QObject(parent)
    , d(new Private(this))
{
}

void ConfinedPointer::Private::setup(zwp_confined_pointer_v1 *arg)
{
    Q_ASSERT(arg);
    Q_ASSERT(!confinedpointer);
    confinedpointer.setup(arg);
    zwp_confined_pointer_v1_add_listener(confinedpointer, &s_listener, this);
}

ConfinedPointer::~ConfinedPointer()
{
    release();
}

void ConfinedPointer::setup(zwp_confined_pointer_v1 *confinedpointer)
{
    d->setup(confinedpointer);
}

void ConfinedPointer::release()
{
    d->confinedpointer.release();
}

void ConfinedPointer::destroy()
{
    d->confinedpointer.destroy();
}

ConfinedPointer::operator zwp_confined_pointer_v1 *()
{
    return d->confinedpointer;
}

ConfinedPointer::operator zwp_confined_pointer_v1 *() const
{
    return d->confinedpointer;
}

bool ConfinedPointer::isValid() const
{
    return d->confinedpointer.isValid();
}

void ConfinedPointer::setRegion(Region *region)
{
    Q_ASSERT(isValid());
    wl_region *wr = nullptr;
    if (region) {
        wr = *region;
    }
    zwp_confined_pointer_v1_set_region(d->confinedpointer, wr);
}

}
}

#include "moc_pointerconstraints.cpp"
