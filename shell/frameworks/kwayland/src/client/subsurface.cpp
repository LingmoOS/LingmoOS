/*
    SPDX-FileCopyrightText: 2014 Martin Gräßlin <mgraesslin@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/
#include "subsurface.h"
#include "surface.h"
#include "wayland_pointer_p.h"
// Wayland
#include <wayland-client-protocol.h>

namespace KWayland
{
namespace Client
{
class Q_DECL_HIDDEN SubSurface::Private
{
public:
    Private(QPointer<Surface> surface, QPointer<Surface> parentSurface, SubSurface *q);
    void setup(wl_subsurface *subsurface);

    WaylandPointer<wl_subsurface, wl_subsurface_destroy> subSurface;
    QPointer<Surface> surface;
    QPointer<Surface> parentSurface;
    Mode mode = Mode::Synchronized;
    QPoint pos = QPoint(0, 0);

    static SubSurface *cast(wl_subsurface *native);

private:
    SubSurface *q;
};

SubSurface::Private::Private(QPointer<Surface> surface, QPointer<Surface> parentSurface, SubSurface *q)
    : surface(surface)
    , parentSurface(parentSurface)
    , q(q)
{
}

void SubSurface::Private::setup(wl_subsurface *subsurface)
{
    Q_ASSERT(subsurface);
    Q_ASSERT(!subSurface.isValid());
    subSurface.setup(subsurface);
    wl_subsurface_set_user_data(subsurface, this);
}

SubSurface *SubSurface::Private::cast(wl_subsurface *native)
{
    return reinterpret_cast<Private *>(wl_subsurface_get_user_data(native))->q;
}

SubSurface::SubSurface(QPointer<Surface> surface, QPointer<Surface> parentSurface, QObject *parent)
    : QObject(parent)
    , d(new Private(surface, parentSurface, this))
{
}

SubSurface::~SubSurface()
{
    release();
}

void SubSurface::setup(wl_subsurface *subsurface)
{
    d->setup(subsurface);
}

void SubSurface::destroy()
{
    d->subSurface.destroy();
}

void SubSurface::release()
{
    d->subSurface.release();
}

bool SubSurface::isValid() const
{
    return d->subSurface.isValid();
}

QPointer<Surface> SubSurface::surface() const
{
    return d->surface;
}

QPointer<Surface> SubSurface::parentSurface() const
{
    return d->parentSurface;
}

void SubSurface::setMode(SubSurface::Mode mode)
{
    if (mode == d->mode) {
        return;
    }
    d->mode = mode;
    switch (d->mode) {
    case Mode::Synchronized:
        wl_subsurface_set_sync(d->subSurface);
        break;
    case Mode::Desynchronized:
        wl_subsurface_set_desync(d->subSurface);
        break;
    }
}

SubSurface::Mode SubSurface::mode() const
{
    return d->mode;
}

void SubSurface::setPosition(const QPoint &pos)
{
    if (pos == d->pos) {
        return;
    }
    d->pos = pos;
    wl_subsurface_set_position(d->subSurface, pos.x(), pos.y());
}

QPoint SubSurface::position() const
{
    return d->pos;
}

void SubSurface::raise()
{
    placeAbove(d->parentSurface);
}

void SubSurface::placeAbove(QPointer<SubSurface> sibling)
{
    if (sibling.isNull()) {
        return;
    }
    placeAbove(sibling->surface());
}

void SubSurface::placeAbove(QPointer<Surface> sibling)
{
    if (sibling.isNull()) {
        return;
    }
    wl_subsurface_place_above(d->subSurface, *sibling);
}

void SubSurface::lower()
{
    placeBelow(d->parentSurface);
}

void SubSurface::placeBelow(QPointer<Surface> sibling)
{
    if (sibling.isNull()) {
        return;
    }
    wl_subsurface_place_below(d->subSurface, *sibling);
}

void SubSurface::placeBelow(QPointer<SubSurface> sibling)
{
    if (sibling.isNull()) {
        return;
    }
    placeBelow(sibling->surface());
}

QPointer<SubSurface> SubSurface::get(wl_subsurface *native)
{
    return QPointer<SubSurface>(Private::cast(native));
}

SubSurface::operator wl_subsurface *() const
{
    return d->subSurface;
}

SubSurface::operator wl_subsurface *()
{
    return d->subSurface;
}

}
}

#include "moc_subsurface.cpp"
