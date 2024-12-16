// Copyright 2020  wugang <wugang@uniontech.com>
// SPDX-FileCopyrightText: 2018 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL

#include "oceanshell.h"
#include "event_queue.h"
#include "logging.h"
#include "surface.h"
#include "wayland_pointer_p.h"
// Qt
#include <QDebug>
#include <QVector>
// wayland
#include "wayland-ocean-shell-client-protocol.h"
#include <wayland-client-protocol.h>

namespace KWayland
{

namespace Client
{

class Q_DECL_HIDDEN OCEANShell::Private
{
public:
    Private(OCEANShell *q);
    void setup(ocean_shell *o);

    WaylandPointer<ocean_shell, ocean_shell_destroy> oceanShell;
    EventQueue *queue = nullptr;

private:
    OCEANShell *q;
};

class Q_DECL_HIDDEN OCEANShellSurface::Private
{
public:
    Private(OCEANShellSurface *q);
    ~Private();
    void setup(ocean_shell_surface *oceanShellSurface);

    WaylandPointer<ocean_shell_surface, ocean_shell_surface_destroy> oceanShellSurface;
    QPointer<Surface> parentSurface;
    QRect geometry;
    bool active = false;
    bool minimized = false;
    bool maximized = false;
    bool fullscreen = false;
    bool keepAbove = false;
    bool keepBelow = false;
    bool closeable = false;
    bool minimizeable = false;
    bool maximizeable = false;
    bool fullscreenable = false;
    bool movable = false;
    bool resizable = false;
    bool acceptFocus = true;
    bool modality = false;
    bool onAllDesktops = false;
    int splitable = 0;

    static OCEANShellSurface *get(wl_surface *surface);
    static OCEANShellSurface *get(Surface *surface);

private:
    static void geometryCallback(void *data, ocean_shell_surface *ocean_shell_surface, int32_t x, int32_t y, uint32_t width, uint32_t height);
    static void stateChangedCallback(void *data, ocean_shell_surface *ocean_shell_surface, uint32_t state);
    void setActive(bool set);
    void setMinimized(bool set);
    void setMaximized(bool set);
    void setFullscreen(bool set);
    void setKeepAbove(bool set);
    void setKeepBelow(bool set);
    void setOnAllDesktops(bool set);
    void setCloseable(bool set);
    void setMinimizeable(bool set);
    void setMaximizeable(bool set);
    void setFullscreenable(bool set);
    void setMovable(bool set);
    void setResizable(bool set);
    void setAcceptFocus(bool set);
    void setModal(bool set);

    static Private *cast(void *data) {
        return reinterpret_cast<Private*>(data);
    }

    OCEANShellSurface *q;
    static QVector<Private*> s_oceanShellSurfaces;
    static const ocean_shell_surface_listener s_listener;
};

QVector<OCEANShellSurface::Private*> OCEANShellSurface::Private::s_oceanShellSurfaces;

OCEANShell::Private::Private(OCEANShell *q)
    : q(q)
{
}

void OCEANShell::Private::setup(ocean_shell *o)
{
    Q_ASSERT(o);
    Q_ASSERT(!oceanShell);
    oceanShell.setup(o);
}

OCEANShell::OCEANShell(QObject *parent)
    : QObject(parent)
    , d(new Private(this))
{
}

OCEANShell::~OCEANShell()
{
    release();
}

void OCEANShell::setup(ocean_shell *OCEANShell)
{
    d->setup(OCEANShell);
}

EventQueue *OCEANShell::eventQueue() const
{
    return d->queue;
}

void OCEANShell::setEventQueue(EventQueue *queue)
{
    d->queue = queue;
}

ocean_shell *OCEANShell::oceanShell()
{
    return d->oceanShell;
}

bool OCEANShell::isValid() const
{
    return d->oceanShell.isValid();
}

OCEANShell::operator ocean_shell*() {
    return d->oceanShell;
}

OCEANShell::operator ocean_shell*() const {
    return d->oceanShell;
}

void OCEANShell::destroy()
{
    if (!d->oceanShell) {
        return;
    }
    Q_EMIT interfaceAboutToBeDestroyed();
    d->oceanShell.destroy();
}

void OCEANShell::release()
{
    if (!d->oceanShell) {
        return;
    }
    Q_EMIT interfaceAboutToBeReleased();
    d->oceanShell.release();
}

OCEANShellSurface *OCEANShell::createShellSurface(wl_surface *surface, QObject *parent)
{
    Q_ASSERT(isValid());
    auto kwS = Surface::get(surface);
    if (kwS) {
        if (auto s = OCEANShellSurface::Private::get(kwS)) {
            return s;
        }
    }
    OCEANShellSurface *s = new OCEANShellSurface(parent);
    connect(this, &OCEANShell::interfaceAboutToBeReleased, s, &OCEANShellSurface::release);
    connect(this, &OCEANShell::interfaceAboutToBeDestroyed, s, &OCEANShellSurface::destroy);
    auto w = ocean_shell_get_shell_surface(d->oceanShell, surface);
    if (d->queue) {
        d->queue->addProxy(w);
    }
    s->setup(w);
    s->d->parentSurface = QPointer<Surface>(kwS);
    return s;
}

OCEANShellSurface *OCEANShell::createShellSurface(Surface *surface, QObject *parent)
{
    return createShellSurface(*surface, parent);
}


OCEANShellSurface::Private::Private(OCEANShellSurface *q)
    : q(q)
{
    s_oceanShellSurfaces << this;
}

OCEANShellSurface::Private::~Private()
{
    s_oceanShellSurfaces.removeAll(this);
}

OCEANShellSurface *OCEANShellSurface::Private::get(wl_surface *surface)
{
    if (!surface) {
        return nullptr;
    }
    for (auto it = s_oceanShellSurfaces.constBegin(); it != s_oceanShellSurfaces.constEnd(); ++it) {
        if ((*it)->parentSurface == Surface::get(surface)) {
            return (*it)->q;
        }
    }
    return nullptr;
}

OCEANShellSurface *OCEANShellSurface::Private::get(Surface *surface)
{
    if (!surface) {
        return nullptr;
    }
    for (auto it = s_oceanShellSurfaces.constBegin(); it != s_oceanShellSurfaces.constEnd(); ++it) {
        if ((*it)->parentSurface == surface) {
            return (*it)->q;
        }
    }
    return nullptr;
}

void OCEANShellSurface::Private::setup(ocean_shell_surface *s)
{
    Q_ASSERT(s);
    Q_ASSERT(!oceanShellSurface);
    oceanShellSurface.setup(s);
    ocean_shell_surface_add_listener(oceanShellSurface, &s_listener, this);
}

void OCEANShellSurface::Private::stateChangedCallback(void *data, ocean_shell_surface *ocean_shell_surface, uint32_t state)
{
    auto p = cast(data);
    Q_UNUSED(ocean_shell_surface);
    p->setActive(state & OCEAN_SHELL_STATE_ACTIVE);
    p->setMinimized(state & OCEAN_SHELL_STATE_MINIMIZED);
    p->setMaximized(state & OCEAN_SHELL_STATE_MAXIMIZED);
    p->setFullscreen(state & OCEAN_SHELL_STATE_FULLSCREEN);
    p->setKeepAbove(state & OCEAN_SHELL_STATE_KEEP_ABOVE);
    p->setKeepBelow(state & OCEAN_SHELL_STATE_KEEP_BELOW);
    p->setOnAllDesktops(state & OCEAN_SHELL_STATE_ON_ALL_DESKTOPS);
    p->setCloseable(state & OCEAN_SHELL_STATE_CLOSEABLE);
    p->setFullscreenable(state & OCEAN_SHELL_STATE_FULLSCREENABLE);
    p->setMaximizeable(state & OCEAN_SHELL_STATE_MAXIMIZABLE);
    p->setMinimizeable(state & OCEAN_SHELL_STATE_MINIMIZABLE);
    p->setMovable(state & OCEAN_SHELL_STATE_MOVABLE);
    p->setResizable(state & OCEAN_SHELL_STATE_RESIZABLE);
    p->setAcceptFocus(state * OCEAN_SHELL_STATE_ACCEPT_FOCUS);
    p->setModal(state & OCEAN_SHELL_STATE_MODALITY);
    if (state & OCEAN_SHELL_STATE_TWO_SPLIT) {
        p->splitable = 1;
    }
    if (state & OCEAN_SHELL_STATE_FOUR_SPLIT) {
        p->splitable = 2;
    }
    if (state & OCEAN_SHELL_STATE_NO_SPLIT) {
        p->splitable = 0;
    }
}

void OCEANShellSurface::Private::geometryCallback(void *data, ocean_shell_surface *oceanShellSurface, int32_t x, int32_t y, uint32_t width, uint32_t height)
{
    Q_UNUSED(oceanShellSurface)
    Private *p = cast(data);
    QRect geo(x, y, width, height);
    if (geo == p->geometry) {
        return;
    }
    p->geometry = geo;
    Q_EMIT p->q->geometryChanged(geo);
}

const ocean_shell_surface_listener OCEANShellSurface::Private::s_listener = {
    geometryCallback,
    stateChangedCallback,
};

void OCEANShellSurface::Private::setActive(bool set)
{
    if (active == set) {
        return;
    }
    active = set;
    Q_EMIT q->activeChanged();
}

void OCEANShellSurface::Private::setOnAllDesktops(bool set)
{
    if (onAllDesktops == set) {
        return;
    }
    onAllDesktops = set;
    emit q->onAllDesktopsChanged();
}

void OCEANShellSurface::Private::setFullscreen(bool set)
{
    if (fullscreen == set) {
        return;
    }
    fullscreen = set;
    Q_EMIT q->fullscreenChanged();
}

void OCEANShellSurface::Private::setKeepAbove(bool set)
{
    if (keepAbove == set) {
        return;
    }
    keepAbove = set;
    Q_EMIT q->keepAboveChanged();
}

void OCEANShellSurface::Private::setKeepBelow(bool set)
{
    if (keepBelow == set) {
        return;
    }
    keepBelow = set;
    Q_EMIT q->keepBelowChanged();
}

void OCEANShellSurface::Private::setMaximized(bool set)
{
    if (maximized == set) {
        return;
    }
    maximized = set;
    Q_EMIT q->maximizedChanged();
}

void OCEANShellSurface::Private::setMinimized(bool set)
{
    if (minimized == set) {
        return;
    }
    minimized = set;
    Q_EMIT q->minimizedChanged();
}

void OCEANShellSurface::Private::setCloseable(bool set)
{
    if (closeable == set) {
        return;
    }
    closeable = set;
    Q_EMIT q->closeableChanged();
}

void OCEANShellSurface::Private::setFullscreenable(bool set)
{
    if (fullscreenable == set) {
        return;
    }
    fullscreenable = set;
    Q_EMIT q->fullscreenableChanged();
}

void OCEANShellSurface::Private::setMaximizeable(bool set)
{
    if (maximizeable == set) {
        return;
    }
    maximizeable = set;
    Q_EMIT q->maximizeableChanged();
}

void OCEANShellSurface::Private::setMinimizeable(bool set)
{
    if (minimizeable == set) {
        return;
    }
    minimizeable = set;
    Q_EMIT q->minimizeableChanged();
}

void OCEANShellSurface::Private::setMovable(bool set)
{
    if (movable == set) {
        return;
    }
    movable = set;
    Q_EMIT q->movableChanged();
}

void OCEANShellSurface::Private::setResizable(bool set)
{
    if (resizable == set) {
        return;
    }
    resizable = set;
    Q_EMIT q->resizableChanged();
}

void OCEANShellSurface::Private::setAcceptFocus(bool set)
{
    if (acceptFocus == set) {
        return;
    }
    acceptFocus = set;
    Q_EMIT q->acceptFocusChanged();
}

void OCEANShellSurface::Private::setModal(bool set)
{
    if (modality == set) {
        return;
    }
    modality = set;
    Q_EMIT q->modalityChanged();
}

OCEANShellSurface::OCEANShellSurface(QObject *parent)
    : QObject(parent)
    , d(new Private(this))
{
}

OCEANShellSurface::~OCEANShellSurface()
{
    release();
}

void OCEANShellSurface::release()
{
    d->oceanShellSurface.release();
}

void OCEANShellSurface::destroy()
{
    d->oceanShellSurface.destroy();
}

void OCEANShellSurface::setup(ocean_shell_surface *oceanShellSurface)
{
    d->setup(oceanShellSurface);
}

OCEANShellSurface *OCEANShellSurface::get(Surface *surface)
{
    if (auto s = OCEANShellSurface::Private::get(surface)) {
        return s;
    }

    return nullptr;
}

bool OCEANShellSurface::isValid() const
{
    return d->oceanShellSurface.isValid();
}

OCEANShellSurface::operator ocean_shell_surface*()
{
    return d->oceanShellSurface;
}

OCEANShellSurface::operator ocean_shell_surface*() const
{
    return d->oceanShellSurface;
}

void OCEANShellSurface::requestGeometry() const
{
    ocean_shell_surface_get_geometry(d->oceanShellSurface);
}

void OCEANShellSurface::requestActive() const
{
    ocean_shell_surface_request_active(d->oceanShellSurface);
}

void OCEANShellSurface::requestActivate()
{
    ocean_shell_surface_set_state(d->oceanShellSurface,
        OCEAN_SHELL_STATE_ACTIVE,
        OCEAN_SHELL_STATE_ACTIVE);
}

void OCEANShellSurface::requestKeepAbove(bool set)
{
    if (!set) {
        ocean_shell_surface_set_state(d->oceanShellSurface,
            OCEAN_SHELL_STATE_KEEP_ABOVE,
            0);
    } else {
        ocean_shell_surface_set_state(d->oceanShellSurface,
            OCEAN_SHELL_STATE_KEEP_ABOVE,
            OCEAN_SHELL_STATE_KEEP_ABOVE);
    }
}

void OCEANShellSurface::requestKeepBelow(bool set)
{
    if (!set) {
        ocean_shell_surface_set_state(d->oceanShellSurface,
            OCEAN_SHELL_STATE_KEEP_BELOW,
            0);
    } else {
        ocean_shell_surface_set_state(d->oceanShellSurface,
            OCEAN_SHELL_STATE_KEEP_BELOW,
            OCEAN_SHELL_STATE_KEEP_BELOW);
    }
}

void OCEANShellSurface::requestMinimized(bool set)
{
    if (!set) {
        ocean_shell_surface_set_state(d->oceanShellSurface,
            OCEAN_SHELL_STATE_MINIMIZED,
            0);
    } else {
        ocean_shell_surface_set_state(d->oceanShellSurface,
            OCEAN_SHELL_STATE_MINIMIZED,
            OCEAN_SHELL_STATE_MINIMIZED);
    }
}

void OCEANShellSurface::requestMaximized(bool set)
{
    if (!set) {
        ocean_shell_surface_set_state(d->oceanShellSurface,
            OCEAN_SHELL_STATE_MAXIMIZED,
            0);
    } else {
        ocean_shell_surface_set_state(d->oceanShellSurface,
            OCEAN_SHELL_STATE_MAXIMIZED,
            OCEAN_SHELL_STATE_MAXIMIZED);
    }
}

void OCEANShellSurface::requestAcceptFocus(bool set)
{
    if (!set) {
        ocean_shell_surface_set_state(d->oceanShellSurface,
            OCEAN_SHELL_STATE_ACCEPT_FOCUS,
            0);
    } else {
        ocean_shell_surface_set_state(d->oceanShellSurface,
            OCEAN_SHELL_STATE_ACCEPT_FOCUS,
            OCEAN_SHELL_STATE_ACCEPT_FOCUS);
    }
}

void OCEANShellSurface::requestModal(bool set)
{
    if (!set) {
        ocean_shell_surface_set_state(d->oceanShellSurface,
            OCEAN_SHELL_STATE_MODALITY,
            0);
    } else {
        ocean_shell_surface_set_state(d->oceanShellSurface,
            OCEAN_SHELL_STATE_MODALITY,
            OCEAN_SHELL_STATE_MODALITY);
    }
}

void OCEANShellSurface::requestMinizeable(bool set)
{
    if (!set) {
        ocean_shell_surface_set_state(d->oceanShellSurface,
            OCEAN_SHELL_STATE_MINIMIZABLE,
            0);
    } else {
        ocean_shell_surface_set_state(d->oceanShellSurface,
            OCEAN_SHELL_STATE_MINIMIZABLE,
            OCEAN_SHELL_STATE_MINIMIZABLE);
    }
}

void OCEANShellSurface::requestMaximizeable(bool set)
{
    if (!set) {
        ocean_shell_surface_set_state(d->oceanShellSurface,
            OCEAN_SHELL_STATE_MAXIMIZABLE,
            0);
    } else {
        ocean_shell_surface_set_state(d->oceanShellSurface,
            OCEAN_SHELL_STATE_MAXIMIZABLE,
            OCEAN_SHELL_STATE_MAXIMIZABLE);
    }
}

void OCEANShellSurface::requestResizable(bool set)
{
    if (!set) {
        ocean_shell_surface_set_state(d->oceanShellSurface,
            OCEAN_SHELL_STATE_RESIZABLE,
            0);
    } else {
        ocean_shell_surface_set_state(d->oceanShellSurface,
            OCEAN_SHELL_STATE_RESIZABLE,
            OCEAN_SHELL_STATE_RESIZABLE);
    }
}

void OCEANShellSurface::requestOnAllDesktops(bool set)
{
    if (!set) {
        ocean_shell_surface_set_state(d->oceanShellSurface,
            OCEAN_SHELL_STATE_ON_ALL_DESKTOPS,
            0);
    } else {
        ocean_shell_surface_set_state(d->oceanShellSurface,
            OCEAN_SHELL_STATE_ON_ALL_DESKTOPS,
            OCEAN_SHELL_STATE_ON_ALL_DESKTOPS);
    }
}

bool OCEANShellSurface::isOnAllDesktops() const
{
    return d->onAllDesktops;
}

bool OCEANShellSurface::isActive() const
{
    return d->active;
}

bool OCEANShellSurface::isFullscreen() const
{
    return d->fullscreen;
}

bool OCEANShellSurface::isKeepAbove() const
{
    return d->keepAbove;
}

bool OCEANShellSurface::isKeepBelow() const
{
    return d->keepBelow;
}

bool OCEANShellSurface::isMaximized() const
{
    return d->maximized;
}

bool OCEANShellSurface::isMinimized() const
{
    return d->minimized;
}

bool OCEANShellSurface::isCloseable() const
{
    return d->closeable;
}

bool OCEANShellSurface::isFullscreenable() const
{
    return d->fullscreenable;
}

bool OCEANShellSurface::isMaximizeable() const
{
    return d->maximizeable;
}

bool OCEANShellSurface::isMinimizeable() const
{
    return d->minimizeable;
}

bool OCEANShellSurface::isResizable() const
{
    return d->resizable;
}

bool OCEANShellSurface::isAcceptFocus() const
{
    return d->acceptFocus;
}

bool OCEANShellSurface::isModal() const
{
    return d->modality;
}

bool OCEANShellSurface::isSplitable() const
{
    if (d->splitable) {
        return true;
    }
    return false;
}

bool OCEANShellSurface::isMovable() const
{
    return d->movable;
}

QRect OCEANShellSurface::getGeometry() const
{
    return d->geometry;
}

int OCEANShellSurface::getSplitable() const
{
    return d->splitable;
}

void OCEANShellSurface::requestNoTitleBarProperty(qint32 value)
{
    struct wl_array arr;
    int *arr_data = nullptr;
    wl_array_init(&arr);
    arr_data = static_cast<int *>(wl_array_add(&arr, sizeof(int)));
    arr_data[0] = value;
    ocean_shell_surface_set_property(d->oceanShellSurface,
                                   OCEAN_SHELL_PROPERTY_NOTITLEBAR,
                                   &arr);
    wl_array_release(&arr);
}

void OCEANShellSurface::requestWindowRadiusProperty(QPointF windowRadius)
{
    struct wl_array arr;
    float *arr_data = nullptr;
    wl_array_init(&arr);
    arr_data = static_cast<float *>(wl_array_add(&arr, sizeof(float)*2));
    arr_data[0] = windowRadius.x();
    arr_data[1] = windowRadius.y();
    ocean_shell_surface_set_property(d->oceanShellSurface,
                                   OCEAN_SHELL_PROPERTY_WINDOWRADIUS,
                                   &arr);
    wl_array_release(&arr);
}

void OCEANShellSurface::requestSplitWindow(OCEANShellSurface::SplitType splitType, OCEANShellSurface::SplitMode mode)
{
    struct wl_array arr;
    int *arr_data = nullptr;
    wl_array_init(&arr);
    arr_data = static_cast<int *>(wl_array_add(&arr, sizeof(int)*2));
    arr_data[0] = (int)splitType;
    arr_data[1] = (int)mode;
    ocean_shell_surface_set_property(d->oceanShellSurface,
                                   OCEAN_SHELL_PROPERTY_QUICKTILE,
                                   &arr);
    wl_array_release(&arr);
}

}
}
