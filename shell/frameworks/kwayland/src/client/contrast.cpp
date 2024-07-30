/*
    SPDX-FileCopyrightText: 2015 Martin Gräßlin <mgraesslin@kde.org>
    SPDX-FileCopyrightText: 2015 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/
#include "contrast.h"
#include "event_queue.h"
#include "region.h"
#include "surface.h"
#include "wayland_pointer_p.h"

#include <wayland-contrast-client-protocol.h>

namespace KWayland
{
namespace Client
{
class Q_DECL_HIDDEN ContrastManager::Private
{
public:
    Private() = default;

    WaylandPointer<org_kde_kwin_contrast_manager, org_kde_kwin_contrast_manager_destroy> manager;
    EventQueue *queue = nullptr;
};

ContrastManager::ContrastManager(QObject *parent)
    : QObject(parent)
    , d(new Private)
{
}

ContrastManager::~ContrastManager()
{
    release();
}

void ContrastManager::release()
{
    d->manager.release();
}

void ContrastManager::destroy()
{
    d->manager.destroy();
}

bool ContrastManager::isValid() const
{
    return d->manager.isValid();
}

void ContrastManager::setup(org_kde_kwin_contrast_manager *manager)
{
    Q_ASSERT(manager);
    Q_ASSERT(!d->manager);
    d->manager.setup(manager);
}

void ContrastManager::setEventQueue(EventQueue *queue)
{
    d->queue = queue;
}

EventQueue *ContrastManager::eventQueue()
{
    return d->queue;
}

Contrast *ContrastManager::createContrast(Surface *surface, QObject *parent)
{
    Q_ASSERT(isValid());
    Contrast *s = new Contrast(parent);
    auto w = org_kde_kwin_contrast_manager_create(d->manager, *surface);
    if (d->queue) {
        d->queue->addProxy(w);
    }
    s->setup(w);
    return s;
}

void ContrastManager::removeContrast(Surface *surface)
{
    Q_ASSERT(isValid());
    org_kde_kwin_contrast_manager_unset(d->manager, *surface);
}

ContrastManager::operator org_kde_kwin_contrast_manager *()
{
    return d->manager;
}

ContrastManager::operator org_kde_kwin_contrast_manager *() const
{
    return d->manager;
}

class Contrast::Private
{
public:
    WaylandPointer<org_kde_kwin_contrast, org_kde_kwin_contrast_release> contrast;
};

Contrast::Contrast(QObject *parent)
    : QObject(parent)
    , d(new Private)
{
}

Contrast::~Contrast()
{
    release();
}

void Contrast::release()
{
    d->contrast.release();
}

void Contrast::setup(org_kde_kwin_contrast *contrast)
{
    Q_ASSERT(contrast);
    Q_ASSERT(!d->contrast);
    d->contrast.setup(contrast);
}

void Contrast::destroy()
{
    d->contrast.destroy();
}

bool Contrast::isValid() const
{
    return d->contrast.isValid();
}

void Contrast::commit()
{
    Q_ASSERT(isValid());
    org_kde_kwin_contrast_commit(d->contrast);
}

void Contrast::setRegion(Region *region)
{
    org_kde_kwin_contrast_set_region(d->contrast, *region);
}

void Contrast::setContrast(qreal contrast)
{
    org_kde_kwin_contrast_set_contrast(d->contrast, wl_fixed_from_double(contrast));
}

void Contrast::setIntensity(qreal intensity)
{
    org_kde_kwin_contrast_set_intensity(d->contrast, wl_fixed_from_double(intensity));
}

void Contrast::setSaturation(qreal saturation)
{
    org_kde_kwin_contrast_set_saturation(d->contrast, wl_fixed_from_double(saturation));
}

void Contrast::setFrost(QColor frost)
{
    if (org_kde_kwin_contrast_get_version(d->contrast) < ORG_KDE_KWIN_CONTRAST_SET_FROST_SINCE_VERSION) {
        return;
    }

    if (frost.isValid()) {
        org_kde_kwin_contrast_set_frost(d->contrast, frost.red(), frost.green(), frost.blue(), frost.alpha());
    } else {
        org_kde_kwin_contrast_unset_frost(d->contrast);
    }
}

Contrast::operator org_kde_kwin_contrast *()
{
    return d->contrast;
}

Contrast::operator org_kde_kwin_contrast *() const
{
    return d->contrast;
}

}
}

#include "moc_contrast.cpp"
