/*
    SPDX-FileCopyrightText: 2015 Marco Martin <notmart@gmail.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/
#include "slide.h"
#include "event_queue.h"
#include "surface.h"
#include "wayland_pointer_p.h"

#include <wayland-slide-client-protocol.h>

namespace KWayland
{
namespace Client
{
class Q_DECL_HIDDEN SlideManager::Private
{
public:
    Private() = default;

    WaylandPointer<org_kde_kwin_slide_manager, org_kde_kwin_slide_manager_destroy> slidemanager;
    EventQueue *queue = nullptr;
};

SlideManager::SlideManager(QObject *parent)
    : QObject(parent)
    , d(new Private)
{
}

SlideManager::~SlideManager()
{
    release();
}

void SlideManager::setup(org_kde_kwin_slide_manager *slidemanager)
{
    Q_ASSERT(slidemanager);
    Q_ASSERT(!d->slidemanager);
    d->slidemanager.setup(slidemanager);
}

void SlideManager::release()
{
    d->slidemanager.release();
}

void SlideManager::destroy()
{
    d->slidemanager.destroy();
}

void SlideManager::setEventQueue(EventQueue *queue)
{
    d->queue = queue;
}

EventQueue *SlideManager::eventQueue()
{
    return d->queue;
}

SlideManager::operator org_kde_kwin_slide_manager *()
{
    return d->slidemanager;
}

SlideManager::operator org_kde_kwin_slide_manager *() const
{
    return d->slidemanager;
}

bool SlideManager::isValid() const
{
    return d->slidemanager.isValid();
}

Slide *SlideManager::createSlide(Surface *surface, QObject *parent)
{
    Q_ASSERT(isValid());
    Slide *s = new Slide(parent);
    auto w = org_kde_kwin_slide_manager_create(d->slidemanager, *surface);
    if (d->queue) {
        d->queue->addProxy(w);
    }
    s->setup(w);
    return s;
}

void SlideManager::removeSlide(Surface *surface)
{
    org_kde_kwin_slide_manager_unset(d->slidemanager, *surface);
}

class Slide::Private
{
public:
    Private() = default;

    WaylandPointer<org_kde_kwin_slide, org_kde_kwin_slide_release> slide;
};

Slide::Slide(QObject *parent)
    : QObject(parent)
    , d(new Private)
{
}

Slide::~Slide()
{
    release();
}

void Slide::setup(org_kde_kwin_slide *slide)
{
    Q_ASSERT(slide);
    Q_ASSERT(!d->slide);
    d->slide.setup(slide);
}

void Slide::release()
{
    d->slide.release();
}

void Slide::destroy()
{
    d->slide.destroy();
}

Slide::operator org_kde_kwin_slide *()
{
    return d->slide;
}

Slide::operator org_kde_kwin_slide *() const
{
    return d->slide;
}

bool Slide::isValid() const
{
    return d->slide.isValid();
}

void Slide::commit()
{
    Q_ASSERT(isValid());
    org_kde_kwin_slide_commit(d->slide);
}

void Slide::setLocation(Slide::Location location)
{
    org_kde_kwin_slide_set_location(d->slide, location);
}

void Slide::setOffset(qint32 offset)
{
    org_kde_kwin_slide_set_offset(d->slide, offset);
}

}
}

#include "moc_slide.cpp"
