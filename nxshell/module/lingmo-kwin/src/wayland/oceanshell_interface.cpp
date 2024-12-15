/********************************************************************
Copyright 2022  luochaojiang <luochaojiang@uniontech.com>

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) version 3, or any
later version accepted by the membership of KDE e.V. (or its
successor approved by the membership of KDE e.V.), which shall
act as a proxy defined in Section 6 of version 3 of the license.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library.  If not, see <http://www.gnu.org/licenses/>.
*********************************************************************/
#include "oceanshell_interface.h"
#include "display.h"
#include "surface_interface.h"
#include "utils.h"

#include "qwayland-server-ocean-shell.h"

#include <QRect>

#define MAX_WINDOWS 50

namespace KWaylandServer
{
static const quint32 s_version = 2;
static QList<OCEANShellSurfaceInterface *> s_shellSurfaces;

class OCEANShellInterfacePrivate : public QtWaylandServer::ocean_shell
{
public:
    OCEANShellInterfacePrivate(OCEANShellInterface *q, Display *display);

private:
    void ocean_shell_get_shell_surface(Resource *resource, uint32_t id, struct ::wl_resource *surface) override;
    OCEANShellInterface *q;
    Display *display;
};

class OCEANShellSurfaceInterfacePrivate : public QtWaylandServer::ocean_shell_surface
{
public:
    OCEANShellSurfaceInterfacePrivate(OCEANShellSurfaceInterface *q, SurfaceInterface *surface, wl_resource *resource);

    SurfaceInterface *surface;
    OCEANShellSurfaceInterface *q;

    void setState(ocean_shell_state flag, bool set);
    void sendGeometry(const QRect &geom);
    void sendSplitable(bool splitable);
    void sendWorkArea(OutputInterface* output, const QRect& geometry);

    ClientConnection *client = nullptr;
private:
    quint32 m_state = 0;
    QRect m_geometry;

    void ocean_shell_surface_destroy_resource(Resource *resource) override;

    void ocean_shell_surface_request_active(Resource *resource) override;
    void ocean_shell_surface_set_state(Resource *resource, uint32_t flags, uint32_t state) override;
    void ocean_shell_surface_set_property(Resource *resource, uint32_t property, wl_array *dataArr) override;
    void ocean_shell_surface_request_split_window(Resource *resource, uint32_t split_type) override;
    void ocean_shell_surface_request_window_effect(Resource *resource, uint32_t type) override;
    void ocean_shell_surface_request_window_startup_effect(Resource *resource, uint32_t type) override;
};

/*********************************
 * OCEANShellInterface
 *********************************/
OCEANShellInterface::OCEANShellInterface(Display *display, QObject *parent)
    : QObject(parent)
    , d(new OCEANShellInterfacePrivate(this, display))
{
}

OCEANShellInterface::~OCEANShellInterface() = default;

OCEANShellInterfacePrivate::OCEANShellInterfacePrivate(OCEANShellInterface *_q, Display *display)
    : QtWaylandServer::ocean_shell(*display, s_version)
    , q(_q)
    , display(display)
{
}

void OCEANShellInterfacePrivate::ocean_shell_get_shell_surface(Resource *resource, uint32_t id, struct ::wl_resource *surface)
{
    SurfaceInterface *s = SurfaceInterface::get(surface);
    if (!s) {
        wl_resource_post_error(resource->handle, 0, "Invalid  surface");
        return;
    }

    if (OCEANShellSurfaceInterface::get(s)) {
        wl_resource_post_error(resource->handle, 0, "ocean_shell_surface already exists");
        return;
    }

    wl_resource *shell_resource = wl_resource_create(resource->client(), &ocean_shell_surface_interface, resource->version(), id);

    auto shellSurface = new OCEANShellSurfaceInterface(display, s, shell_resource);
    s_shellSurfaces.append(shellSurface);

    QObject::connect(shellSurface, &QObject::destroyed, [shellSurface]() {
        s_shellSurfaces.removeOne(shellSurface);
    });

    Q_EMIT q->shellSurfaceCreated(shellSurface);
}

/*********************************
 * OCEANShellSurfaceInterface
 *********************************/
OCEANShellSurfaceInterface::OCEANShellSurfaceInterface(Display *display, SurfaceInterface *surface, wl_resource *resource)
    : QObject(surface)
    , d(new OCEANShellSurfaceInterfacePrivate(this, surface, resource))
{
    d->client = display->getConnection(d->resource()->client());
}

OCEANShellSurfaceInterface::~OCEANShellSurfaceInterface() = default;

SurfaceInterface *OCEANShellSurfaceInterface::surface() const
{
    return d->surface;
}

OCEANShellSurfaceInterface *OCEANShellSurfaceInterface::get(wl_resource *native)
{
    if (auto surfacePrivate = resource_cast<OCEANShellSurfaceInterfacePrivate *>(native)) {
        return surfacePrivate->q;
    }
    return nullptr;
}

OCEANShellSurfaceInterface *OCEANShellSurfaceInterface::get(SurfaceInterface *surface)
{
    for (OCEANShellSurfaceInterface *shellSurface : qAsConst(s_shellSurfaces)) {
        if (shellSurface->surface() == surface) {
            return shellSurface;
        }
    }
    return nullptr;
}

void OCEANShellSurfaceInterfacePrivate::sendWorkArea(OutputInterface* output, const QRect& geometry)
{
    if (!output)
        return;
    auto resources = output->clientResources(client);
    for (wl_resource *outputResource : resources) {
        send_workarea(outputResource, geometry.x(), geometry.y(), geometry.width(), geometry.height());
    }
}

void OCEANShellSurfaceInterfacePrivate::setState(ocean_shell_state flag, bool set)
{
    quint32 newState = m_state;
    if (set) {
        newState |= flag;
    } else {
        newState &= ~flag;
    }
    if (newState == m_state) {
        return;
    }
    m_state = newState;
    send_state_changed(m_state);
}

void OCEANShellSurfaceInterfacePrivate::sendGeometry(const QRect &geometry)
{
    if (m_geometry == geometry) {
        return;
    }
    m_geometry = geometry;
    if (!m_geometry.isValid()) {
        return;
    }
    send_geometry(m_geometry.x(), m_geometry.y(), m_geometry.width(), m_geometry.height());
}

OCEANShellSurfaceInterfacePrivate::OCEANShellSurfaceInterfacePrivate(OCEANShellSurfaceInterface *_q, SurfaceInterface *_surface, wl_resource *resource)
    : QtWaylandServer::ocean_shell_surface(resource)
    , surface(_surface)
    , q(_q)
{
}

void OCEANShellSurfaceInterfacePrivate::ocean_shell_surface_destroy_resource(Resource *resource)
{
    Q_UNUSED(resource)
    delete q;
}

void OCEANShellSurfaceInterfacePrivate::ocean_shell_surface_request_active(Resource *resource)
{
    Q_UNUSED(resource)

    Q_EMIT q->activationRequested();
}

void OCEANShellSurfaceInterfacePrivate::ocean_shell_surface_set_state(Resource *resource, uint32_t flags, uint32_t state)
{
    Q_UNUSED(resource)

    if (flags & OCEAN_SHELL_STATE_ACTIVE) {
        Q_EMIT q->activeRequested(state & OCEAN_SHELL_STATE_ACTIVE);
    }
    if (flags & OCEAN_SHELL_STATE_MINIMIZED) {
        Q_EMIT q->minimizedRequested(state & OCEAN_SHELL_STATE_MINIMIZED);
    }
    if (flags & OCEAN_SHELL_STATE_MAXIMIZED) {
        Q_EMIT q->maximizedRequested(state & OCEAN_SHELL_STATE_MAXIMIZED);
    }
    if (flags & OCEAN_SHELL_STATE_FULLSCREEN) {
        Q_EMIT q->fullscreenRequested(state & OCEAN_SHELL_STATE_FULLSCREEN);
    }
    if (flags & OCEAN_SHELL_STATE_KEEP_ABOVE) {
        Q_EMIT q->keepAboveRequested(state & OCEAN_SHELL_STATE_KEEP_ABOVE);
    }
    if (flags & OCEAN_SHELL_STATE_KEEP_BELOW) {
        Q_EMIT q->keepBelowRequested(state & OCEAN_SHELL_STATE_KEEP_BELOW);
    }
    if (flags & OCEAN_SHELL_STATE_ON_ALL_DESKTOPS) {
        Q_EMIT q->onAllDesktopsRequested(state & OCEAN_SHELL_STATE_ON_ALL_DESKTOPS);
    }
    if (flags & OCEAN_SHELL_STATE_CLOSEABLE) {
        Q_EMIT q->closeableRequested(state & OCEAN_SHELL_STATE_CLOSEABLE);
    }
    if (flags & OCEAN_SHELL_STATE_MINIMIZABLE) {
        Q_EMIT q->minimizeableRequested(state & OCEAN_SHELL_STATE_MINIMIZABLE);
    }
    if (flags & OCEAN_SHELL_STATE_MAXIMIZABLE) {
        Q_EMIT q->maximizeableRequested(state & OCEAN_SHELL_STATE_MAXIMIZABLE);
    }
    if (flags & OCEAN_SHELL_STATE_FULLSCREENABLE) {
        Q_EMIT q->fullscreenableRequested(state & OCEAN_SHELL_STATE_FULLSCREENABLE);
    }
    if (flags & OCEAN_SHELL_STATE_MOVABLE) {
        Q_EMIT q->movableRequested(state & OCEAN_SHELL_STATE_MOVABLE);
    }
    if (flags & OCEAN_SHELL_STATE_RESIZABLE) {
        Q_EMIT q->resizableRequested(state & OCEAN_SHELL_STATE_RESIZABLE);
    }
    if (flags & OCEAN_SHELL_STATE_ACCEPT_FOCUS) {
        Q_EMIT q->acceptFocusRequested(state & OCEAN_SHELL_STATE_ACCEPT_FOCUS);
    }
    if (flags & OCEAN_SHELL_STATE_MODALITY) {
        Q_EMIT q->modalityRequested(state & OCEAN_SHELL_STATE_MODALITY);
    }
}

void OCEANShellSurfaceInterfacePrivate::ocean_shell_surface_set_property(Resource *resource, uint32_t property, wl_array *dataArr)
{
    Q_UNUSED(resource)
    if (property & OCEAN_SHELL_PROPERTY_NOTITLEBAR) {
        int *value = static_cast<int *>(dataArr->data);
        Q_EMIT q->noTitleBarPropertyRequested(*value);
    }
    if (property & OCEAN_SHELL_PROPERTY_WINDOWRADIUS) {
        float *value = static_cast<float *>(dataArr->data);
        QPointF pnt = QPointF(value[0],value[1]);
        Q_EMIT q->windowRadiusPropertyRequested(pnt);
    }
    if (property & OCEAN_SHELL_PROPERTY_SHADOWCOLOR) {
        char *value = static_cast<char *>(dataArr->data);
        Q_EMIT q->shadowColorPropertyRequested(value);
    }
    if (property & OCEAN_SHELL_PROPERTY_SHADOWRADIUS) {
        int *value = static_cast<int *>(dataArr->data);
        Q_EMIT q->shadowRadiusPropertyRequested(*value);
    }
    if (property & OCEAN_SHELL_PROPERTY_SHADOWOFFSET) {
        float *value = static_cast<float *>(dataArr->data);
        QPointF pnt = QPointF(value[0],value[1]);
        Q_EMIT q->shadowOffsetPropertyRequested(pnt);
    }
    if (property & OCEAN_SHELL_PROPERTY_BORDERWIDTH) {
        int *value = static_cast<int *>(dataArr->data);
        Q_EMIT q->borderWidthPropertyRequested(*value);
    }
    if (property & OCEAN_SHELL_PROPERTY_BORDERCOLOR) {
        char *value = static_cast<char *>(dataArr->data);
        Q_EMIT q->borderColorPropertyRequested(value);
    }
}

void OCEANShellSurfaceInterfacePrivate::ocean_shell_surface_request_split_window(Resource *resource, uint32_t split_type)
{
    Q_UNUSED(resource)
    switch (split_type) {
    case OCEAN_SHELL_SPLIT_TYPE_LEFT_SPLIT:
        Q_EMIT q->splitWindowRequested(KWaylandServer::SplitType::leftSplit);
        break;
    case OCEAN_SHELL_SPLIT_TYPE_RIGHT_SPLIT:
        Q_EMIT q->splitWindowRequested(KWaylandServer::SplitType::rightSplit);
        break;
    default:
        break;
    }
}

void OCEANShellSurfaceInterfacePrivate::ocean_shell_surface_request_window_effect(Resource *resource, uint32_t type)
{
    Q_UNUSED(resource)
    Q_EMIT q->windowEffectRequested(type);
}

void OCEANShellSurfaceInterfacePrivate::ocean_shell_surface_request_window_startup_effect(Resource *resource, uint32_t type)
{
    Q_UNUSED(resource)
    Q_EMIT q->windowStartUpEffectRequested(type);
}

void OCEANShellSurfaceInterfacePrivate::sendSplitable(bool splitable)
{
    send_splitable(int(splitable));
}

void OCEANShellSurfaceInterface::setActive(bool set)
{
    d->setState(OCEAN_SHELL_STATE_ACTIVE, set);
}

void OCEANShellSurfaceInterface::setFullscreen(bool set)
{
    d->setState(OCEAN_SHELL_STATE_FULLSCREEN, set);
}

void OCEANShellSurfaceInterface::setKeepAbove(bool set)
{
    d->setState(OCEAN_SHELL_STATE_KEEP_ABOVE, set);
}

void OCEANShellSurfaceInterface::setKeepBelow(bool set)
{
    d->setState(OCEAN_SHELL_STATE_KEEP_BELOW, set);
}

void OCEANShellSurfaceInterface::setOnAllDesktops(bool set)
{
    d->setState(OCEAN_SHELL_STATE_ON_ALL_DESKTOPS, set);
}

void OCEANShellSurfaceInterface::setMaximized(bool set)
{
    d->setState(OCEAN_SHELL_STATE_MAXIMIZED, set);
}

void OCEANShellSurfaceInterface::setMinimized(bool set)
{
    d->setState(OCEAN_SHELL_STATE_MINIMIZED, set);
}

void OCEANShellSurfaceInterface::setCloseable(bool set)
{
    d->setState(OCEAN_SHELL_STATE_CLOSEABLE, set);
}

void OCEANShellSurfaceInterface::setFullscreenable(bool set)
{
    d->setState(OCEAN_SHELL_STATE_FULLSCREENABLE, set);
}

void OCEANShellSurfaceInterface::setMaximizeable(bool set)
{
    d->setState(OCEAN_SHELL_STATE_MAXIMIZABLE, set);
}

void OCEANShellSurfaceInterface::setMinimizeable(bool set)
{
    d->setState(OCEAN_SHELL_STATE_MINIMIZABLE, set);
}

void OCEANShellSurfaceInterface::setMovable(bool set)
{
    d->setState(OCEAN_SHELL_STATE_MOVABLE, set);
}

void OCEANShellSurfaceInterface::setResizable(bool set)
{
    d->setState(OCEAN_SHELL_STATE_RESIZABLE, set);
}

void OCEANShellSurfaceInterface::setAcceptFocus(bool set)
{
    d->setState(OCEAN_SHELL_STATE_ACCEPT_FOCUS, set);
}

void OCEANShellSurfaceInterface::setModal(bool set)
{
    d->setState(OCEAN_SHELL_STATE_MODALITY, set);
}

void OCEANShellSurfaceInterface::sendWorkArea(OutputInterface* output, const QRect& geometry)
{
    if (d->resource()->version() >= OCEAN_SHELL_SURFACE_WORKAREA_SINCE_VERSION) {
        d->sendWorkArea(output, geometry);
    }
}

void OCEANShellSurfaceInterface::sendGeometry(const QRect &geom)
{
    d->sendGeometry(geom);
}

void OCEANShellSurfaceInterface::sendSplitable(bool splitable)
{
    d->sendSplitable(splitable);
}

}
