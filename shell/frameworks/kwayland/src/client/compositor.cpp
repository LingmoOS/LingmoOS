/*
    SPDX-FileCopyrightText: 2014 Martin Gräßlin <mgraesslin@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/
#include "compositor.h"
#include "event_queue.h"
#include "region.h"
#include "surface.h"
#include "wayland_pointer_p.h"
// Qt
#include <QGuiApplication>
#include <QRegion>
#include <qpa/qplatformnativeinterface.h>

#include <wayland-client-protocol.h>

namespace KWayland
{
namespace Client
{
class Q_DECL_HIDDEN Compositor::Private
{
public:
    Private() = default;

    WaylandPointer<wl_compositor, wl_compositor_destroy> compositor;
    EventQueue *queue = nullptr;
};

Compositor::Compositor(QObject *parent)
    : QObject(parent)
    , d(new Private)
{
}

Compositor::~Compositor()
{
    release();
}

Compositor *Compositor::fromApplication(QObject *parent)
{
    QPlatformNativeInterface *native = qApp->platformNativeInterface();
    if (!native) {
        return nullptr;
    }
    wl_compositor *compositor = reinterpret_cast<wl_compositor *>(native->nativeResourceForIntegration(QByteArrayLiteral("compositor")));
    if (!compositor) {
        return nullptr;
    }
    Compositor *c = new Compositor(parent);
    c->d->compositor.setup(compositor, true);
    return c;
}

void Compositor::setup(wl_compositor *compositor)
{
    Q_ASSERT(compositor);
    Q_ASSERT(!d->compositor);
    d->compositor.setup(compositor);
}

void Compositor::release()
{
    d->compositor.release();
}

void Compositor::destroy()
{
    d->compositor.destroy();
}

void Compositor::setEventQueue(EventQueue *queue)
{
    d->queue = queue;
}

EventQueue *Compositor::eventQueue()
{
    return d->queue;
}

Surface *Compositor::createSurface(QObject *parent)
{
    Q_ASSERT(isValid());
    Surface *s = new Surface(parent);
    auto w = wl_compositor_create_surface(d->compositor);
    if (d->queue) {
        d->queue->addProxy(w);
    }
    s->setup(w);
    return s;
}

Region *Compositor::createRegion(QObject *parent)
{
    return createRegion(QRegion(), parent);
}

Region *Compositor::createRegion(const QRegion &region, QObject *parent)
{
    Q_ASSERT(isValid());
    Region *r = new Region(region, parent);
    auto w = wl_compositor_create_region(d->compositor);
    if (d->queue) {
        d->queue->addProxy(w);
    }
    r->setup(w);
    return r;
}

std::unique_ptr<Region> Compositor::createRegion(const QRegion &region)
{
    return std::unique_ptr<Region>(createRegion(region, nullptr));
}

Compositor::operator wl_compositor *()
{
    return d->compositor;
}

Compositor::operator wl_compositor *() const
{
    return d->compositor;
}

bool Compositor::isValid() const
{
    return d->compositor.isValid();
}

}
}

#include "moc_compositor.cpp"
