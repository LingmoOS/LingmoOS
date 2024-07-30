/*
    SPDX-FileCopyrightText: 2014 Martin Gräßlin <mgraesslin@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/
#include "subcompositor.h"
#include "event_queue.h"
#include "subsurface.h"
#include "surface.h"
#include "wayland_pointer_p.h"
// Wayland
#include <wayland-client-protocol.h>

namespace KWayland
{
namespace Client
{
class Q_DECL_HIDDEN SubCompositor::Private
{
public:
    WaylandPointer<wl_subcompositor, wl_subcompositor_destroy> subCompositor;
    EventQueue *queue = nullptr;
};

SubCompositor::SubCompositor(QObject *parent)
    : QObject(parent)
    , d(new Private)
{
}

SubCompositor::~SubCompositor()
{
    release();
}

void SubCompositor::release()
{
    d->subCompositor.release();
}

void SubCompositor::destroy()
{
    d->subCompositor.destroy();
}

void SubCompositor::setup(wl_subcompositor *subcompositor)
{
    Q_ASSERT(subcompositor);
    Q_ASSERT(!d->subCompositor.isValid());
    d->subCompositor.setup(subcompositor);
}

SubSurface *SubCompositor::createSubSurface(QPointer<Surface> surface, QPointer<Surface> parentSurface, QObject *parent)
{
    Q_ASSERT(isValid());
    SubSurface *s = new SubSurface(surface, parentSurface, parent);
    auto w = wl_subcompositor_get_subsurface(d->subCompositor, *surface, *parentSurface);
    if (d->queue) {
        d->queue->addProxy(w);
    }
    s->setup(w);
    return s;
}

bool SubCompositor::isValid() const
{
    return d->subCompositor.isValid();
}

SubCompositor::operator wl_subcompositor *()
{
    return d->subCompositor;
}

SubCompositor::operator wl_subcompositor *() const
{
    return d->subCompositor;
}

EventQueue *SubCompositor::eventQueue()
{
    return d->queue;
}

void SubCompositor::setEventQueue(EventQueue *queue)
{
    d->queue = queue;
}

}
}

#include "moc_subcompositor.cpp"
