/*
    SPDX-FileCopyrightText: 2015 Martin Gräßlin <mgraesslin@kde.org>
    SPDX-FileCopyrightText: 2015 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/
#include "blur.h"
#include "event_queue.h"
#include "region.h"
#include "surface.h"
#include "wayland_pointer_p.h"

#include <wayland-blur-client-protocol.h>

namespace KWayland
{
namespace Client
{
class Q_DECL_HIDDEN BlurManager::Private
{
public:
    Private() = default;

    WaylandPointer<org_kde_kwin_blur_manager, org_kde_kwin_blur_manager_destroy> manager;
    EventQueue *queue = nullptr;
};

BlurManager::BlurManager(QObject *parent)
    : QObject(parent)
    , d(new Private)
{
}

BlurManager::~BlurManager()
{
    release();
}

void BlurManager::release()
{
    d->manager.release();
}

void BlurManager::destroy()
{
    d->manager.destroy();
}

bool BlurManager::isValid() const
{
    return d->manager.isValid();
}

void BlurManager::setup(org_kde_kwin_blur_manager *manager)
{
    Q_ASSERT(manager);
    Q_ASSERT(!d->manager);
    d->manager.setup(manager);
}

void BlurManager::setEventQueue(EventQueue *queue)
{
    d->queue = queue;
}

EventQueue *BlurManager::eventQueue()
{
    return d->queue;
}

Blur *BlurManager::createBlur(Surface *surface, QObject *parent)
{
    Q_ASSERT(isValid());
    Blur *s = new Blur(parent);
    auto w = org_kde_kwin_blur_manager_create(d->manager, *surface);
    if (d->queue) {
        d->queue->addProxy(w);
    }
    s->setup(w);
    return s;
}

void BlurManager::removeBlur(Surface *surface)
{
    Q_ASSERT(isValid());
    org_kde_kwin_blur_manager_unset(d->manager, *surface);
}

BlurManager::operator org_kde_kwin_blur_manager *()
{
    return d->manager;
}

BlurManager::operator org_kde_kwin_blur_manager *() const
{
    return d->manager;
}

class Blur::Private
{
public:
    WaylandPointer<org_kde_kwin_blur, org_kde_kwin_blur_release> blur;
};

Blur::Blur(QObject *parent)
    : QObject(parent)
    , d(new Private)
{
}

Blur::~Blur()
{
    release();
}

void Blur::release()
{
    d->blur.release();
}

void Blur::setup(org_kde_kwin_blur *blur)
{
    Q_ASSERT(blur);
    Q_ASSERT(!d->blur);
    d->blur.setup(blur);
}

void Blur::destroy()
{
    d->blur.destroy();
}

bool Blur::isValid() const
{
    return d->blur.isValid();
}

void Blur::commit()
{
    Q_ASSERT(isValid());
    org_kde_kwin_blur_commit(d->blur);
}

void Blur::setRegion(Region *region)
{
    org_kde_kwin_blur_set_region(d->blur, *region);
}

Blur::operator org_kde_kwin_blur *()
{
    return d->blur;
}

Blur::operator org_kde_kwin_blur *() const
{
    return d->blur;
}

}
}

#include "moc_blur.cpp"
