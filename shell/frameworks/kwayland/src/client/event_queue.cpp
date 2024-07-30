/*
    SPDX-FileCopyrightText: 2014 Martin Gräßlin <mgraesslin@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/
#include "event_queue.h"
#include "connection_thread.h"
#include "wayland_pointer_p.h"

#include <wayland-client.h>

namespace KWayland
{
namespace Client
{
class Q_DECL_HIDDEN EventQueue::Private
{
public:
    wl_display *display = nullptr;
    WaylandPointer<wl_event_queue, wl_event_queue_destroy> queue;
};

EventQueue::EventQueue(QObject *parent)
    : QObject(parent)
    , d(new Private)
{
}

EventQueue::~EventQueue()
{
    release();
}

void EventQueue::release()
{
    d->queue.release();
    d->display = nullptr;
}

void EventQueue::destroy()
{
    d->queue.destroy();
    d->display = nullptr;
}

bool EventQueue::isValid()
{
    return d->queue.isValid();
}

void EventQueue::setup(wl_display *display)
{
    Q_ASSERT(display);
    Q_ASSERT(!d->display);
    Q_ASSERT(!d->queue);
    d->display = display;
    d->queue.setup(wl_display_create_queue(display));
}

void EventQueue::setup(ConnectionThread *connection)
{
    setup(connection->display());
    connect(connection, &ConnectionThread::eventsRead, this, &EventQueue::dispatch, Qt::QueuedConnection);
}

void EventQueue::dispatch()
{
    if (!d->display || !d->queue) {
        return;
    }
    wl_display_dispatch_queue_pending(d->display, d->queue);
    wl_display_flush(d->display);
}

void EventQueue::addProxy(wl_proxy *proxy)
{
    Q_ASSERT(d->queue);
    wl_proxy_set_queue(proxy, d->queue);
}

EventQueue::operator wl_event_queue *() const
{
    return d->queue;
}

EventQueue::operator wl_event_queue *()
{
    return d->queue;
}

}
}

#include "moc_event_queue.cpp"
