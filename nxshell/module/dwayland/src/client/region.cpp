/*
    SPDX-FileCopyrightText: 2014 Martin Gräßlin <mgraesslin@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/
#include "region.h"
#include "wayland_pointer_p.h"
// Qt
#include <QRegion>
#include <QVector>
// Wayland
#include <wayland-client-protocol.h>

namespace KWayland
{
namespace Client
{
class Q_DECL_HIDDEN Region::Private
{
public:
    Private(const QRegion &region);
    void installRegion(const QRect &rect);
    void installRegion(const QRegion &region);
    void uninstallRegion(const QRect &rect);
    void uninstallRegion(const QRegion &region);

    WaylandPointer<wl_region, wl_region_destroy> region;
    QRegion qtRegion;
};

Region::Private::Private(const QRegion &region)
    : qtRegion(region)
{
}

void Region::Private::installRegion(const QRect &rect)
{
    if (!region.isValid()) {
        return;
    }
    wl_region_add(region, rect.x(), rect.y(), rect.width(), rect.height());
}

void Region::Private::installRegion(const QRegion &region)
{
    for (const QRect &rect : region) {
        installRegion(rect);
    }
}

void Region::Private::uninstallRegion(const QRect &rect)
{
    if (!region.isValid()) {
        return;
    }
    wl_region_subtract(region, rect.x(), rect.y(), rect.width(), rect.height());
}

void Region::Private::uninstallRegion(const QRegion &region)
{
    for (const QRect &rect : region) {
        uninstallRegion(rect);
    }
}

Region::Region(const QRegion &region, QObject *parent)
    : QObject(parent)
    , d(new Private(region))
{
}

Region::~Region()
{
    release();
}

void Region::release()
{
    d->region.release();
}

void Region::destroy()
{
    d->region.destroy();
}

void Region::setup(wl_region *region)
{
    Q_ASSERT(region);
    d->region.setup(region);
    d->installRegion(d->qtRegion);
}

bool Region::isValid() const
{
    return d->region.isValid();
}

void Region::add(const QRect &rect)
{
    d->qtRegion = d->qtRegion.united(rect);
    d->installRegion(rect);
}

void Region::add(const QRegion &region)
{
    d->qtRegion = d->qtRegion.united(region);
    d->installRegion(region);
}

void Region::subtract(const QRect &rect)
{
    d->qtRegion = d->qtRegion.subtracted(rect);
    d->uninstallRegion(rect);
}

void Region::subtract(const QRegion &region)
{
    d->qtRegion = d->qtRegion.subtracted(region);
    d->uninstallRegion(region);
}

QRegion Region::region() const
{
    return d->qtRegion;
}

Region::operator wl_region *() const
{
    return d->region;
}

Region::operator wl_region *()
{
    return d->region;
}

}
}
