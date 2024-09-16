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
#include "ddeshell_interface.h"
#include "display.h"
#include "surface_interface.h"
#include "utils.h"

#include "qwayland-server-dde-shell.h"

#include <QRect>

#define MAX_WINDOWS 50

namespace KWaylandServer
{
static const quint32 s_version = 2;
static QList<DDEShellSurfaceInterface *> s_shellSurfaces;

class DDEShellInterfacePrivate : public QtWaylandServer::dde_shell
{
public:
    DDEShellInterfacePrivate(DDEShellInterface *q, Display *display);

private:
    void dde_shell_get_shell_surface(Resource *resource, uint32_t id, struct ::wl_resource *surface) override;
    DDEShellInterface *q;
    Display *display;
};

class DDEShellSurfaceInterfacePrivate : public QtWaylandServer::dde_shell_surface
{
public:
    DDEShellSurfaceInterfacePrivate(DDEShellSurfaceInterface *q, SurfaceInterface *surface, wl_resource *resource);

    SurfaceInterface *surface;
    DDEShellSurfaceInterface *q;

    void setState(dde_shell_state flag, bool set);
    void sendGeometry(const QRect &geom);
    void sendSplitable(bool splitable);
    void sendWorkArea(OutputInterface* output, const QRect& geometry);

    ClientConnection *client = nullptr;
private:
    quint32 m_state = 0;
    QRect m_geometry;

    void dde_shell_surface_destroy_resource(Resource *resource) override;

    void dde_shell_surface_request_active(Resource *resource) override;
    void dde_shell_surface_set_state(Resource *resource, uint32_t flags, uint32_t state) override;
    void dde_shell_surface_set_property(Resource *resource, uint32_t property, wl_array *dataArr) override;
    void dde_shell_surface_request_split_window(Resource *resource, uint32_t split_type) override;
    void dde_shell_surface_request_window_effect(Resource *resource, uint32_t type) override;
    void dde_shell_surface_request_window_startup_effect(Resource *resource, uint32_t type) override;
};

/*********************************
 * DDEShellInterface
 *********************************/
DDEShellInterface::DDEShellInterface(Display *display, QObject *parent)
    : QObject(parent)
    , d(new DDEShellInterfacePrivate(this, display))
{
}

DDEShellInterface::~DDEShellInterface() = default;

DDEShellInterfacePrivate::DDEShellInterfacePrivate(DDEShellInterface *_q, Display *display)
    : QtWaylandServer::dde_shell(*display, s_version)
    , q(_q)
    , display(display)
{
}

void DDEShellInterfacePrivate::dde_shell_get_shell_surface(Resource *resource, uint32_t id, struct ::wl_resource *surface)
{
    SurfaceInterface *s = SurfaceInterface::get(surface);
    if (!s) {
        wl_resource_post_error(resource->handle, 0, "Invalid  surface");
        return;
    }

    if (DDEShellSurfaceInterface::get(s)) {
        wl_resource_post_error(resource->handle, 0, "dde_shell_surface already exists");
        return;
    }

    wl_resource *shell_resource = wl_resource_create(resource->client(), &dde_shell_surface_interface, resource->version(), id);

    auto shellSurface = new DDEShellSurfaceInterface(display, s, shell_resource);
    s_shellSurfaces.append(shellSurface);

    QObject::connect(shellSurface, &QObject::destroyed, [shellSurface]() {
        s_shellSurfaces.removeOne(shellSurface);
    });

    Q_EMIT q->shellSurfaceCreated(shellSurface);
}

/*********************************
 * DDEShellSurfaceInterface
 *********************************/
DDEShellSurfaceInterface::DDEShellSurfaceInterface(Display *display, SurfaceInterface *surface, wl_resource *resource)
    : QObject(surface)
    , d(new DDEShellSurfaceInterfacePrivate(this, surface, resource))
{
    d->client = display->getConnection(d->resource()->client());
}

DDEShellSurfaceInterface::~DDEShellSurfaceInterface() = default;

SurfaceInterface *DDEShellSurfaceInterface::surface() const
{
    return d->surface;
}

DDEShellSurfaceInterface *DDEShellSurfaceInterface::get(wl_resource *native)
{
    if (auto surfacePrivate = resource_cast<DDEShellSurfaceInterfacePrivate *>(native)) {
        return surfacePrivate->q;
    }
    return nullptr;
}

DDEShellSurfaceInterface *DDEShellSurfaceInterface::get(SurfaceInterface *surface)
{
    for (DDEShellSurfaceInterface *shellSurface : qAsConst(s_shellSurfaces)) {
        if (shellSurface->surface() == surface) {
            return shellSurface;
        }
    }
    return nullptr;
}

void DDEShellSurfaceInterfacePrivate::sendWorkArea(OutputInterface* output, const QRect& geometry)
{
    if (!output)
        return;
    auto resources = output->clientResources(client);
    for (wl_resource *outputResource : resources) {
        send_workarea(outputResource, geometry.x(), geometry.y(), geometry.width(), geometry.height());
    }
}

void DDEShellSurfaceInterfacePrivate::setState(dde_shell_state flag, bool set)
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

void DDEShellSurfaceInterfacePrivate::sendGeometry(const QRect &geometry)
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

DDEShellSurfaceInterfacePrivate::DDEShellSurfaceInterfacePrivate(DDEShellSurfaceInterface *_q, SurfaceInterface *_surface, wl_resource *resource)
    : QtWaylandServer::dde_shell_surface(resource)
    , surface(_surface)
    , q(_q)
{
}

void DDEShellSurfaceInterfacePrivate::dde_shell_surface_destroy_resource(Resource *resource)
{
    Q_UNUSED(resource)
    delete q;
}

void DDEShellSurfaceInterfacePrivate::dde_shell_surface_request_active(Resource *resource)
{
    Q_UNUSED(resource)

    Q_EMIT q->activationRequested();
}

void DDEShellSurfaceInterfacePrivate::dde_shell_surface_set_state(Resource *resource, uint32_t flags, uint32_t state)
{
    Q_UNUSED(resource)

    if (flags & DDE_SHELL_STATE_ACTIVE) {
        Q_EMIT q->activeRequested(state & DDE_SHELL_STATE_ACTIVE);
    }
    if (flags & DDE_SHELL_STATE_MINIMIZED) {
        Q_EMIT q->minimizedRequested(state & DDE_SHELL_STATE_MINIMIZED);
    }
    if (flags & DDE_SHELL_STATE_MAXIMIZED) {
        Q_EMIT q->maximizedRequested(state & DDE_SHELL_STATE_MAXIMIZED);
    }
    if (flags & DDE_SHELL_STATE_FULLSCREEN) {
        Q_EMIT q->fullscreenRequested(state & DDE_SHELL_STATE_FULLSCREEN);
    }
    if (flags & DDE_SHELL_STATE_KEEP_ABOVE) {
        Q_EMIT q->keepAboveRequested(state & DDE_SHELL_STATE_KEEP_ABOVE);
    }
    if (flags & DDE_SHELL_STATE_KEEP_BELOW) {
        Q_EMIT q->keepBelowRequested(state & DDE_SHELL_STATE_KEEP_BELOW);
    }
    if (flags & DDE_SHELL_STATE_ON_ALL_DESKTOPS) {
        Q_EMIT q->onAllDesktopsRequested(state & DDE_SHELL_STATE_ON_ALL_DESKTOPS);
    }
    if (flags & DDE_SHELL_STATE_CLOSEABLE) {
        Q_EMIT q->closeableRequested(state & DDE_SHELL_STATE_CLOSEABLE);
    }
    if (flags & DDE_SHELL_STATE_MINIMIZABLE) {
        Q_EMIT q->minimizeableRequested(state & DDE_SHELL_STATE_MINIMIZABLE);
    }
    if (flags & DDE_SHELL_STATE_MAXIMIZABLE) {
        Q_EMIT q->maximizeableRequested(state & DDE_SHELL_STATE_MAXIMIZABLE);
    }
    if (flags & DDE_SHELL_STATE_FULLSCREENABLE) {
        Q_EMIT q->fullscreenableRequested(state & DDE_SHELL_STATE_FULLSCREENABLE);
    }
    if (flags & DDE_SHELL_STATE_MOVABLE) {
        Q_EMIT q->movableRequested(state & DDE_SHELL_STATE_MOVABLE);
    }
    if (flags & DDE_SHELL_STATE_RESIZABLE) {
        Q_EMIT q->resizableRequested(state & DDE_SHELL_STATE_RESIZABLE);
    }
    if (flags & DDE_SHELL_STATE_ACCEPT_FOCUS) {
        Q_EMIT q->acceptFocusRequested(state & DDE_SHELL_STATE_ACCEPT_FOCUS);
    }
    if (flags & DDE_SHELL_STATE_MODALITY) {
        Q_EMIT q->modalityRequested(state & DDE_SHELL_STATE_MODALITY);
    }
}

void DDEShellSurfaceInterfacePrivate::dde_shell_surface_set_property(Resource *resource, uint32_t property, wl_array *dataArr)
{
    Q_UNUSED(resource)
    if (property & DDE_SHELL_PROPERTY_NOTITLEBAR) {
        int *value = static_cast<int *>(dataArr->data);
        Q_EMIT q->noTitleBarPropertyRequested(*value);
    }
    if (property & DDE_SHELL_PROPERTY_WINDOWRADIUS) {
        float *value = static_cast<float *>(dataArr->data);
        QPointF pnt = QPointF(value[0],value[1]);
        Q_EMIT q->windowRadiusPropertyRequested(pnt);
    }
    if (property & DDE_SHELL_PROPERTY_SHADOWCOLOR) {
        char *value = static_cast<char *>(dataArr->data);
        Q_EMIT q->shadowColorPropertyRequested(value);
    }
    if (property & DDE_SHELL_PROPERTY_SHADOWRADIUS) {
        int *value = static_cast<int *>(dataArr->data);
        Q_EMIT q->shadowRadiusPropertyRequested(*value);
    }
    if (property & DDE_SHELL_PROPERTY_SHADOWOFFSET) {
        float *value = static_cast<float *>(dataArr->data);
        QPointF pnt = QPointF(value[0],value[1]);
        Q_EMIT q->shadowOffsetPropertyRequested(pnt);
    }
    if (property & DDE_SHELL_PROPERTY_BORDERWIDTH) {
        int *value = static_cast<int *>(dataArr->data);
        Q_EMIT q->borderWidthPropertyRequested(*value);
    }
    if (property & DDE_SHELL_PROPERTY_BORDERCOLOR) {
        char *value = static_cast<char *>(dataArr->data);
        Q_EMIT q->borderColorPropertyRequested(value);
    }
}

void DDEShellSurfaceInterfacePrivate::dde_shell_surface_request_split_window(Resource *resource, uint32_t split_type)
{
    Q_UNUSED(resource)
    switch (split_type) {
    case DDE_SHELL_SPLIT_TYPE_LEFT_SPLIT:
        Q_EMIT q->splitWindowRequested(KWaylandServer::SplitType::leftSplit);
        break;
    case DDE_SHELL_SPLIT_TYPE_RIGHT_SPLIT:
        Q_EMIT q->splitWindowRequested(KWaylandServer::SplitType::rightSplit);
        break;
    default:
        break;
    }
}

void DDEShellSurfaceInterfacePrivate::dde_shell_surface_request_window_effect(Resource *resource, uint32_t type)
{
    Q_UNUSED(resource)
    Q_EMIT q->windowEffectRequested(type);
}

void DDEShellSurfaceInterfacePrivate::dde_shell_surface_request_window_startup_effect(Resource *resource, uint32_t type)
{
    Q_UNUSED(resource)
    Q_EMIT q->windowStartUpEffectRequested(type);
}

void DDEShellSurfaceInterfacePrivate::sendSplitable(bool splitable)
{
    send_splitable(int(splitable));
}

void DDEShellSurfaceInterface::setActive(bool set)
{
    d->setState(DDE_SHELL_STATE_ACTIVE, set);
}

void DDEShellSurfaceInterface::setFullscreen(bool set)
{
    d->setState(DDE_SHELL_STATE_FULLSCREEN, set);
}

void DDEShellSurfaceInterface::setKeepAbove(bool set)
{
    d->setState(DDE_SHELL_STATE_KEEP_ABOVE, set);
}

void DDEShellSurfaceInterface::setKeepBelow(bool set)
{
    d->setState(DDE_SHELL_STATE_KEEP_BELOW, set);
}

void DDEShellSurfaceInterface::setOnAllDesktops(bool set)
{
    d->setState(DDE_SHELL_STATE_ON_ALL_DESKTOPS, set);
}

void DDEShellSurfaceInterface::setMaximized(bool set)
{
    d->setState(DDE_SHELL_STATE_MAXIMIZED, set);
}

void DDEShellSurfaceInterface::setMinimized(bool set)
{
    d->setState(DDE_SHELL_STATE_MINIMIZED, set);
}

void DDEShellSurfaceInterface::setCloseable(bool set)
{
    d->setState(DDE_SHELL_STATE_CLOSEABLE, set);
}

void DDEShellSurfaceInterface::setFullscreenable(bool set)
{
    d->setState(DDE_SHELL_STATE_FULLSCREENABLE, set);
}

void DDEShellSurfaceInterface::setMaximizeable(bool set)
{
    d->setState(DDE_SHELL_STATE_MAXIMIZABLE, set);
}

void DDEShellSurfaceInterface::setMinimizeable(bool set)
{
    d->setState(DDE_SHELL_STATE_MINIMIZABLE, set);
}

void DDEShellSurfaceInterface::setMovable(bool set)
{
    d->setState(DDE_SHELL_STATE_MOVABLE, set);
}

void DDEShellSurfaceInterface::setResizable(bool set)
{
    d->setState(DDE_SHELL_STATE_RESIZABLE, set);
}

void DDEShellSurfaceInterface::setAcceptFocus(bool set)
{
    d->setState(DDE_SHELL_STATE_ACCEPT_FOCUS, set);
}

void DDEShellSurfaceInterface::setModal(bool set)
{
    d->setState(DDE_SHELL_STATE_MODALITY, set);
}

void DDEShellSurfaceInterface::sendWorkArea(OutputInterface* output, const QRect& geometry)
{
    if (d->resource()->version() >= DDE_SHELL_SURFACE_WORKAREA_SINCE_VERSION) {
        d->sendWorkArea(output, geometry);
    }
}

void DDEShellSurfaceInterface::sendGeometry(const QRect &geom)
{
    d->sendGeometry(geom);
}

void DDEShellSurfaceInterface::sendSplitable(bool splitable)
{
    d->sendSplitable(splitable);
}

}
