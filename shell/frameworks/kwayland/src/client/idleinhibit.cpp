/*
    SPDX-FileCopyrightText: 2017 Martin Flöser <mgraesslin@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/
#include "idleinhibit.h"
#include "event_queue.h"
#include "surface.h"
#include "wayland_pointer_p.h"

#include <wayland-idle-inhibit-unstable-v1-client-protocol.h>

namespace KWayland
{
namespace Client
{
class Q_DECL_HIDDEN IdleInhibitManager::Private
{
public:
    Private() = default;

    void setup(zwp_idle_inhibit_manager_v1 *arg);

    WaylandPointer<zwp_idle_inhibit_manager_v1, zwp_idle_inhibit_manager_v1_destroy> idleinhibitmanager;
    EventQueue *queue = nullptr;
};

IdleInhibitManager::IdleInhibitManager(QObject *parent)
    : QObject(parent)
    , d(new Private)
{
}

void IdleInhibitManager::Private::setup(zwp_idle_inhibit_manager_v1 *arg)
{
    Q_ASSERT(arg);
    Q_ASSERT(!idleinhibitmanager);
    idleinhibitmanager.setup(arg);
}

IdleInhibitManager::~IdleInhibitManager()
{
    release();
}

void IdleInhibitManager::setup(zwp_idle_inhibit_manager_v1 *idleinhibitmanager)
{
    d->setup(idleinhibitmanager);
}

void IdleInhibitManager::release()
{
    d->idleinhibitmanager.release();
}

void IdleInhibitManager::destroy()
{
    d->idleinhibitmanager.destroy();
}

IdleInhibitManager::operator zwp_idle_inhibit_manager_v1 *()
{
    return d->idleinhibitmanager;
}

IdleInhibitManager::operator zwp_idle_inhibit_manager_v1 *() const
{
    return d->idleinhibitmanager;
}

bool IdleInhibitManager::isValid() const
{
    return d->idleinhibitmanager.isValid();
}

void IdleInhibitManager::setEventQueue(EventQueue *queue)
{
    d->queue = queue;
}

EventQueue *IdleInhibitManager::eventQueue()
{
    return d->queue;
}

IdleInhibitor *IdleInhibitManager::createInhibitor(Surface *surface, QObject *parent)
{
    Q_ASSERT(isValid());
    auto p = new IdleInhibitor(parent);
    auto w = zwp_idle_inhibit_manager_v1_create_inhibitor(d->idleinhibitmanager, *surface);
    if (d->queue) {
        d->queue->addProxy(w);
    }
    p->setup(w);
    return p;
}

class Q_DECL_HIDDEN IdleInhibitor::Private
{
public:
    void setup(zwp_idle_inhibitor_v1 *arg);

    WaylandPointer<zwp_idle_inhibitor_v1, zwp_idle_inhibitor_v1_destroy> idleinhibitor;
};

IdleInhibitor::IdleInhibitor(QObject *parent)
    : QObject(parent)
    , d(new Private)
{
}

void IdleInhibitor::Private::setup(zwp_idle_inhibitor_v1 *arg)
{
    Q_ASSERT(arg);
    Q_ASSERT(!idleinhibitor);
    idleinhibitor.setup(arg);
}

IdleInhibitor::~IdleInhibitor()
{
    release();
}

void IdleInhibitor::setup(zwp_idle_inhibitor_v1 *idleinhibitor)
{
    d->setup(idleinhibitor);
}

void IdleInhibitor::release()
{
    d->idleinhibitor.release();
}

void IdleInhibitor::destroy()
{
    d->idleinhibitor.destroy();
}

IdleInhibitor::operator zwp_idle_inhibitor_v1 *()
{
    return d->idleinhibitor;
}

IdleInhibitor::operator zwp_idle_inhibitor_v1 *() const
{
    return d->idleinhibitor;
}

bool IdleInhibitor::isValid() const
{
    return d->idleinhibitor.isValid();
}

}
}

#include "moc_idleinhibit.cpp"
