/*
    SPDX-FileCopyrightText: 2020 Vlad Zahorodnii <vlad.zahorodnii@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "xdgshell_v6_interface.h"
#include "xdgshell_v6_interface_p.h"

#include "display.h"
#include "output_interface.h"
#include "seat_interface.h"
#include "utils.h"

#include <QTimer>

namespace KWaylandServer
{
static const int s_version = 1;

XdgShellV6InterfacePrivate::XdgShellV6InterfacePrivate(XdgShellV6Interface *shell)
    : q(shell)
{
}

XdgShellV6InterfacePrivate::Resource *XdgShellV6InterfacePrivate::resourceForXdgSurface(XdgSurfaceV6Interface *surface) const
{
    return xdgSurfaces.value(surface);
}

void XdgShellV6InterfacePrivate::unregisterXdgSurface(XdgSurfaceV6Interface *surface)
{
    xdgSurfaces.remove(surface);
}

/**
 * @todo Whether the ping is delayed or has timed out is out of domain of the XdgShellV6Interface.
 * Such matter must be handled somewhere else, e.g. XdgToplevelWindow, not here!
 */
void XdgShellV6InterfacePrivate::registerPing(quint32 serial)
{
    QTimer *timer = new QTimer(q);
    timer->setInterval(1000);
    QObject::connect(timer, &QTimer::timeout, q, [this, serial, attempt = 0]() mutable {
        ++attempt;
        if (attempt == 1) {
            Q_EMIT q->pingDelayed(serial);
            return;
        }
        Q_EMIT q->pingTimeout(serial);
        delete pings.take(serial);
    });
    pings.insert(serial, timer);
    timer->start();
}

XdgShellV6InterfacePrivate *XdgShellV6InterfacePrivate::get(XdgShellV6Interface *shell)
{
    return shell->d.get();
}

void XdgShellV6InterfacePrivate::zxdg_shell_v6_destroy(Resource *resource)
{
    if (xdgSurfaces.key(resource)) {
        wl_resource_post_error(resource->handle, error_defunct_surfaces, "zxdg_shell_v6 was destroyed before children");
        return;
    }
    wl_resource_destroy(resource->handle);
}

void XdgShellV6InterfacePrivate::zxdg_shell_v6_create_positioner(Resource *resource, uint32_t id)
{
    wl_resource *positionerResource = wl_resource_create(resource->client(), &zxdg_positioner_v6_interface, resource->version(), id);
    new XdgPositionerV6Private(positionerResource);
}

void XdgShellV6InterfacePrivate::zxdg_shell_v6_get_xdg_surface(Resource *resource, uint32_t id, ::wl_resource *surfaceResource)
{
    SurfaceInterface *surface = SurfaceInterface::get(surfaceResource);

    if (surface->buffer()) {
        wl_resource_post_error(resource->handle, ZXDG_SURFACE_V6_ERROR_UNCONFIGURED_BUFFER, "zxdg_surface_v6 must not have a buffer at creation");
        return;
    }

    wl_resource *xdgSurfaceResource = wl_resource_create(resource->client(), &zxdg_surface_v6_interface, resource->version(), id);

    XdgSurfaceV6Interface *xdgSurface = new XdgSurfaceV6Interface(q, surface, xdgSurfaceResource);
    xdgSurfaces.insert(xdgSurface, resource);
}

void XdgShellV6InterfacePrivate::zxdg_shell_v6_pong(Resource *resource, uint32_t serial)
{
    if (QTimer *timer = pings.take(serial)) {
        delete timer;
    }
    Q_EMIT q->pongReceived(serial);
}

XdgShellV6Interface::XdgShellV6Interface(Display *display, QObject *parent)
    : QObject(parent)
    , d(new XdgShellV6InterfacePrivate(this))
{
    d->display = display;
    d->init(*display, s_version);
}

XdgShellV6Interface::~XdgShellV6Interface()
{
}

Display *XdgShellV6Interface::display() const
{
    return d->display;
}

quint32 XdgShellV6Interface::ping(XdgSurfaceV6Interface *surface)
{
    XdgShellV6InterfacePrivate::Resource *clientResource = d->resourceForXdgSurface(surface);
    if (!clientResource)
        return 0;

    quint32 serial = d->display->nextSerial();
    d->send_ping(clientResource->handle, serial);
    d->registerPing(serial);

    return serial;
}

XdgSurfaceV6InterfacePrivate::XdgSurfaceV6InterfacePrivate(XdgSurfaceV6Interface *xdgSurface)
    : q(xdgSurface)
{
}

void XdgSurfaceV6InterfacePrivate::commit()
{
    if (surface->buffer()) {
        firstBufferAttached = true;
    }

    if (next.acknowledgedConfigureIsSet) {
        current.acknowledgedConfigure = next.acknowledgedConfigure;
        next.acknowledgedConfigureIsSet = false;
        Q_EMIT q->configureAcknowledged(current.acknowledgedConfigure);
    }

    if (next.windowGeometryIsSet) {
        current.windowGeometry = next.windowGeometry;
        next.windowGeometryIsSet = false;
        Q_EMIT q->windowGeometryChanged(current.windowGeometry);
    }
}

void XdgSurfaceV6InterfacePrivate::reset()
{
    firstBufferAttached = false;
    isConfigured = false;
    isInitialized = false;
    current = XdgSurfaceV6State{};
    next = XdgSurfaceV6State{};
    Q_EMIT q->resetOccurred();
}

XdgSurfaceV6InterfacePrivate *XdgSurfaceV6InterfacePrivate::get(XdgSurfaceV6Interface *surface)
{
    return surface->d.get();
}

void XdgSurfaceV6InterfacePrivate::zxdg_surface_v6_destroy_resource(Resource *resource)
{
    Q_EMIT q->aboutToBeDestroyed();
    XdgShellV6InterfacePrivate::get(shell)->unregisterXdgSurface(q);
    delete q;
}

void XdgSurfaceV6InterfacePrivate::zxdg_surface_v6_destroy(Resource *resource)
{
    if (toplevel || popup) {
        qWarning() << "Tried to destroy zxdg_surface_v6 before its role object";
    }
    wl_resource_destroy(resource->handle);
}

void XdgSurfaceV6InterfacePrivate::zxdg_surface_v6_get_toplevel(Resource *resource, uint32_t id)
{
    const SurfaceRole *surfaceRole = SurfaceRole::get(surface);
    if (surfaceRole) {
        wl_resource_post_error(resource->handle, error_already_constructed, "the surface already has a role assigned %s", surfaceRole->name().constData());
        return;
    }

    wl_resource *toplevelResource = wl_resource_create(resource->client(), &zxdg_toplevel_v6_interface, resource->version(), id);

    toplevel = new XdgToplevelV6Interface(q, toplevelResource);
    Q_EMIT shell->toplevelCreated(toplevel);
}

void XdgSurfaceV6InterfacePrivate::zxdg_surface_v6_get_popup(Resource *resource, uint32_t id, ::wl_resource *parentResource, ::wl_resource *positionerResource)
{
    const SurfaceRole *surfaceRole = SurfaceRole::get(surface);
    if (surfaceRole) {
        wl_resource_post_error(resource->handle, error_already_constructed, "the surface already has a role assigned %s", surfaceRole->name().constData());
        return;
    }

    XdgPositionerV6 positioner = XdgPositionerV6::get(positionerResource);
    if (!positioner.isComplete()) {
        auto shellPrivate = XdgShellV6InterfacePrivate::get(shell);
        wl_resource_post_error(shellPrivate->resourceForXdgSurface(q)->handle,
                               QtWaylandServer::zxdg_shell_v6::error_invalid_positioner,
                               "zxdg_positioner_v6 is incomplete");
        return;
    }

    XdgSurfaceV6Interface *parentXdgSurface = XdgSurfaceV6Interface::get(parentResource);
    SurfaceInterface *parentSurface = nullptr;
    if (parentXdgSurface) {
        parentSurface = parentXdgSurface->surface();
    }

    wl_resource *popupResource = wl_resource_create(resource->client(), &zxdg_popup_v6_interface, resource->version(), id);

    popup = new XdgPopupV6Interface(q, parentSurface, positioner, popupResource);
    Q_EMIT shell->popupCreated(popup);
}

void XdgSurfaceV6InterfacePrivate::zxdg_surface_v6_set_window_geometry(Resource *resource, int32_t x, int32_t y, int32_t width, int32_t height)
{
    if (!toplevel && !popup) {
        wl_resource_post_error(resource->handle, error_not_constructed, "zxdg_surface_v6 must have a role");
        return;
    }

    if (width < 1 || height < 1) {
        wl_resource_post_error(resource->handle, -1, "invalid window geometry size (%dx%d)", width, height);
        return;
    }

    next.windowGeometry = QRect(x, y, width, height);
    next.windowGeometryIsSet = true;
}

void XdgSurfaceV6InterfacePrivate::zxdg_surface_v6_ack_configure(Resource *resource, uint32_t serial)
{
    next.acknowledgedConfigure = serial;
    next.acknowledgedConfigureIsSet = true;
}

XdgSurfaceV6Interface::XdgSurfaceV6Interface(XdgShellV6Interface *shell, SurfaceInterface *surface, ::wl_resource *resource)
    : d(new XdgSurfaceV6InterfacePrivate(this))
{
    d->shell = shell;
    d->surface = surface;
    d->init(resource);
}

XdgSurfaceV6Interface::~XdgSurfaceV6Interface()
{
}

XdgToplevelV6Interface *XdgSurfaceV6Interface::toplevel() const
{
    return d->toplevel;
}

XdgPopupV6Interface *XdgSurfaceV6Interface::popup() const
{
    return d->popup;
}

XdgShellV6Interface *XdgSurfaceV6Interface::shell() const
{
    return d->shell;
}

SurfaceInterface *XdgSurfaceV6Interface::surface() const
{
    return d->surface;
}

bool XdgSurfaceV6Interface::isConfigured() const
{
    return d->isConfigured;
}

QRect XdgSurfaceV6Interface::windowGeometry() const
{
    return d->current.windowGeometry;
}

XdgSurfaceV6Interface *XdgSurfaceV6Interface::get(::wl_resource *resource)
{
    if (auto surfacePrivate = resource_cast<XdgSurfaceV6InterfacePrivate *>(resource)) {
        return surfacePrivate->q;
    }
    return nullptr;
}

XdgToplevelV6InterfacePrivate::XdgToplevelV6InterfacePrivate(XdgToplevelV6Interface *toplevel, XdgSurfaceV6Interface *surface)
    : SurfaceRole(surface->surface(), QByteArrayLiteral("zxdg_toplevel"))
    , q(toplevel)
    , xdgSurface(surface)
{
}

void XdgToplevelV6InterfacePrivate::commit()
{
    auto xdgSurfacePrivate = XdgSurfaceV6InterfacePrivate::get(xdgSurface);
    if (xdgSurfacePrivate->firstBufferAttached && !xdgSurfacePrivate->surface->buffer()) {
        reset();
        return;
    }

    xdgSurfacePrivate->commit();

    if (current.minimumSize != next.minimumSize) {
        current.minimumSize = next.minimumSize;
        Q_EMIT q->minimumSizeChanged(current.minimumSize);
    }
    if (current.maximumSize != next.maximumSize) {
        current.maximumSize = next.maximumSize;
        Q_EMIT q->maximumSizeChanged(current.maximumSize);
    }

    if (!xdgSurfacePrivate->isInitialized) {
        Q_EMIT q->initializeRequested();
        xdgSurfacePrivate->isInitialized = true;
    }
}

void XdgToplevelV6InterfacePrivate::reset()
{
    auto xdgSurfacePrivate = XdgSurfaceV6InterfacePrivate::get(xdgSurface);
    xdgSurfacePrivate->reset();

    windowTitle = QString();
    windowClass = QString();
    current = next = State();

    Q_EMIT q->resetOccurred();
}

void XdgToplevelV6InterfacePrivate::zxdg_toplevel_v6_destroy_resource(Resource *resource)
{
    Q_EMIT q->aboutToBeDestroyed();
    delete q;
}

void XdgToplevelV6InterfacePrivate::zxdg_toplevel_v6_destroy(Resource *resource)
{
    wl_resource_destroy(resource->handle);
}

void XdgToplevelV6InterfacePrivate::zxdg_toplevel_v6_set_parent(Resource *resource, ::wl_resource *parentResource)
{
    XdgToplevelV6Interface *parent = XdgToplevelV6Interface::get(parentResource);
    if (parentXdgToplevel == parent) {
        return;
    }
    parentXdgToplevel = parent;
    Q_EMIT q->parentXdgToplevelChanged();
}

void XdgToplevelV6InterfacePrivate::zxdg_toplevel_v6_set_title(Resource *resource, const QString &title)
{
    if (windowTitle == title) {
        return;
    }
    windowTitle = title;
    Q_EMIT q->windowTitleChanged(title);
}

void XdgToplevelV6InterfacePrivate::zxdg_toplevel_v6_set_app_id(Resource *resource, const QString &app_id)
{
    if (windowClass == app_id) {
        return;
    }
    windowClass = app_id;
    Q_EMIT q->windowClassChanged(app_id);
}

void XdgToplevelV6InterfacePrivate::zxdg_toplevel_v6_show_window_menu(Resource *resource, ::wl_resource *seatResource, uint32_t serial, int32_t x, int32_t y)
{
    auto xdgSurfacePrivate = XdgSurfaceV6InterfacePrivate::get(xdgSurface);

    if (!xdgSurfacePrivate->isConfigured) {
        wl_resource_post_error(resource->handle, QtWaylandServer::zxdg_surface_v6::error_not_constructed, "surface has not been configured yet");
        return;
    }

    SeatInterface *seat = SeatInterface::get(seatResource);
    Q_EMIT q->windowMenuRequested(seat, QPoint(x, y), serial);
}

void XdgToplevelV6InterfacePrivate::zxdg_toplevel_v6_move(Resource *resource, ::wl_resource *seatResource, uint32_t serial)
{
    auto xdgSurfacePrivate = XdgSurfaceV6InterfacePrivate::get(xdgSurface);

    if (!xdgSurfacePrivate->isConfigured) {
        wl_resource_post_error(resource->handle, QtWaylandServer::zxdg_surface_v6::error_not_constructed, "surface has not been configured yet");
        return;
    }

    SeatInterface *seat = SeatInterface::get(seatResource);
    Q_EMIT q->moveRequested(seat, serial);
}

void XdgToplevelV6InterfacePrivate::zxdg_toplevel_v6_resize(Resource *resource, ::wl_resource *seatResource, uint32_t serial, uint32_t xdgEdges)
{
    auto xdgSurfacePrivate = XdgSurfaceV6InterfacePrivate::get(xdgSurface);

    if (!xdgSurfacePrivate->isConfigured) {
        wl_resource_post_error(resource->handle, QtWaylandServer::zxdg_surface_v6::error_not_constructed, "surface has not been configured yet");
        return;
    }

    SeatInterface *seat = SeatInterface::get(seatResource);
    Q_EMIT q->resizeRequested(seat, XdgToplevelV6Interface::ResizeAnchor(xdgEdges), serial);
}

void XdgToplevelV6InterfacePrivate::zxdg_toplevel_v6_set_max_size(Resource *resource, int32_t width, int32_t height)
{
    if (width < 0 || height < 0) {
        wl_resource_post_error(resource->handle, -1, "width and height must be positive or zero");
        return;
    }
    next.maximumSize = QSize(width, height);
}

void XdgToplevelV6InterfacePrivate::zxdg_toplevel_v6_set_min_size(Resource *resource, int32_t width, int32_t height)
{
    if (width < 0 || height < 0) {
        wl_resource_post_error(resource->handle, -1, "width and height must be positive or zero");
        return;
    }
    next.minimumSize = QSize(width, height);
}

void XdgToplevelV6InterfacePrivate::zxdg_toplevel_v6_set_maximized(Resource *resource)
{
    Q_EMIT q->maximizeRequested();
}

void XdgToplevelV6InterfacePrivate::zxdg_toplevel_v6_unset_maximized(Resource *resource)
{
    Q_EMIT q->unmaximizeRequested();
}

void XdgToplevelV6InterfacePrivate::zxdg_toplevel_v6_set_fullscreen(Resource *resource, ::wl_resource *outputResource)
{
    OutputInterface *output = OutputInterface::get(outputResource);
    Q_EMIT q->fullscreenRequested(output);
}

void XdgToplevelV6InterfacePrivate::zxdg_toplevel_v6_unset_fullscreen(Resource *resource)
{
    Q_EMIT q->unfullscreenRequested();
}

void XdgToplevelV6InterfacePrivate::zxdg_toplevel_v6_set_minimized(Resource *resource)
{
    Q_EMIT q->minimizeRequested();
}

XdgToplevelV6InterfacePrivate *XdgToplevelV6InterfacePrivate::get(XdgToplevelV6Interface *toplevel)
{
    return toplevel->d.get();
}

XdgToplevelV6InterfacePrivate *XdgToplevelV6InterfacePrivate::get(wl_resource *resource)
{
    return resource_cast<XdgToplevelV6InterfacePrivate *>(resource);
}

XdgToplevelV6Interface::XdgToplevelV6Interface(XdgSurfaceV6Interface *surface, ::wl_resource *resource)
    : d(new XdgToplevelV6InterfacePrivate(this, surface))
{
    d->init(resource);
}

XdgToplevelV6Interface::~XdgToplevelV6Interface()
{
}

XdgShellV6Interface *XdgToplevelV6Interface::shell() const
{
    return d->xdgSurface->shell();
}

XdgSurfaceV6Interface *XdgToplevelV6Interface::xdgSurface() const
{
    return d->xdgSurface;
}

SurfaceInterface *XdgToplevelV6Interface::surface() const
{
    return d->xdgSurface->surface();
}

bool XdgToplevelV6Interface::isConfigured() const
{
    return d->xdgSurface->isConfigured();
}

XdgToplevelV6Interface *XdgToplevelV6Interface::parentXdgToplevel() const
{
    return d->parentXdgToplevel;
}

QString XdgToplevelV6Interface::windowTitle() const
{
    return d->windowTitle;
}

QString XdgToplevelV6Interface::windowClass() const
{
    return d->windowClass;
}

QSize XdgToplevelV6Interface::minimumSize() const
{
    return d->current.minimumSize.isEmpty() ? QSize(0, 0) : d->current.minimumSize;
}

QSize XdgToplevelV6Interface::maximumSize() const
{
    return d->current.maximumSize.isEmpty() ? QSize(INT_MAX, INT_MAX) : d->current.maximumSize;
}

quint32 XdgToplevelV6Interface::sendConfigure(const QSize &size, const States &states)
{
    // Note that the states listed in the configure event must be an array of uint32_t.

    uint32_t statesData[8] = {0};
    int i = 0;

    if (states & State::MaximizedHorizontal && states & State::MaximizedVertical) {
        statesData[i++] = QtWaylandServer::zxdg_toplevel_v6::state_maximized;
    }
    if (states & State::FullScreen) {
        statesData[i++] = QtWaylandServer::zxdg_toplevel_v6::state_fullscreen;
    }
    if (states & State::Resizing) {
        statesData[i++] = QtWaylandServer::zxdg_toplevel_v6::state_resizing;
    }
    if (states & State::Activated) {
        statesData[i++] = QtWaylandServer::zxdg_toplevel_v6::state_activated;
    }

    const QByteArray xdgStates = QByteArray::fromRawData(reinterpret_cast<char *>(statesData), sizeof(uint32_t) * i);
    const quint32 serial = xdgSurface()->shell()->display()->nextSerial();

    d->send_configure(size.width(), size.height(), xdgStates);

    auto xdgSurfacePrivate = XdgSurfaceV6InterfacePrivate::get(xdgSurface());
    xdgSurfacePrivate->send_configure(serial);
    xdgSurfacePrivate->isConfigured = true;

    return serial;
}

void XdgToplevelV6Interface::sendClose()
{
    d->send_close();
}

void XdgToplevelV6Interface::sendConfigureBounds(const QSize &size)
{
}

XdgToplevelV6Interface *XdgToplevelV6Interface::get(::wl_resource *resource)
{
    if (auto toplevelPrivate = resource_cast<XdgToplevelV6InterfacePrivate *>(resource)) {
        return toplevelPrivate->q;
    }
    return nullptr;
}

XdgPopupV6InterfacePrivate *XdgPopupV6InterfacePrivate::get(XdgPopupV6Interface *popup)
{
    return popup->d.get();
}

XdgPopupV6InterfacePrivate::XdgPopupV6InterfacePrivate(XdgPopupV6Interface *popup, XdgSurfaceV6Interface *surface)
    : SurfaceRole(surface->surface(), QByteArrayLiteral("xdg_popup"))
    , q(popup)
    , xdgSurface(surface)
{
}

void XdgPopupV6InterfacePrivate::commit()
{
    if (!parentSurface) {
        auto shellPrivate = XdgShellV6InterfacePrivate::get(xdgSurface->shell());
        wl_resource_post_error(shellPrivate->resourceForXdgSurface(xdgSurface)->handle,
                               QtWaylandServer::zxdg_shell_v6::error_invalid_popup_parent,
                               "no xdg_popup parent surface has been specified");
        return;
    }

    auto xdgSurfacePrivate = XdgSurfaceV6InterfacePrivate::get(xdgSurface);
    if (xdgSurfacePrivate->firstBufferAttached && !xdgSurfacePrivate->surface->buffer()) {
        reset();
        return;
    }

    xdgSurfacePrivate->commit();

    if (!xdgSurfacePrivate->isInitialized) {
        Q_EMIT q->initializeRequested();
        xdgSurfacePrivate->isInitialized = true;
    }
}

void XdgPopupV6InterfacePrivate::reset()
{
    auto xdgSurfacePrivate = XdgSurfaceV6InterfacePrivate::get(xdgSurface);
    xdgSurfacePrivate->reset();
}

void XdgPopupV6InterfacePrivate::zxdg_popup_v6_destroy_resource(Resource *resource)
{
    Q_EMIT q->aboutToBeDestroyed();
    delete q;
}

void XdgPopupV6InterfacePrivate::zxdg_popup_v6_destroy(Resource *resource)
{
    // TODO: We need to post an error with the code XDG_WM_BASE_ERROR_NOT_THE_TOPMOST_POPUP if
    // this popup is not the topmost grabbing popup. We most likely need a grab abstraction or
    // something to determine whether the given popup has an explicit grab.
    wl_resource_destroy(resource->handle);
}

void XdgPopupV6InterfacePrivate::zxdg_popup_v6_grab(Resource *resource, ::wl_resource *seatHandle, uint32_t serial)
{
    if (xdgSurface->surface()->buffer()) {
        wl_resource_post_error(resource->handle, error_invalid_grab, "zxdg_surface_v6 is already mapped");
        return;
    }
    SeatInterface *seat = SeatInterface::get(seatHandle);
    Q_EMIT q->grabRequested(seat, serial);
}

XdgPopupV6Interface::XdgPopupV6Interface(XdgSurfaceV6Interface *surface, SurfaceInterface *parentSurface, const XdgPositionerV6 &positioner, ::wl_resource *resource)
    : d(new XdgPopupV6InterfacePrivate(this, surface))
{
    d->parentSurface = parentSurface;
    d->positioner = positioner;
    d->init(resource);
}

XdgPopupV6Interface::~XdgPopupV6Interface()
{
}

SurfaceInterface *XdgPopupV6Interface::parentSurface() const
{
    return d->parentSurface;
}

XdgSurfaceV6Interface *XdgPopupV6Interface::xdgSurface() const
{
    return d->xdgSurface;
}

SurfaceInterface *XdgPopupV6Interface::surface() const
{
    return d->xdgSurface->surface();
}

bool XdgPopupV6Interface::isConfigured() const
{
    return d->xdgSurface->isConfigured();
}

XdgPositionerV6 XdgPopupV6Interface::positioner() const
{
    return d->positioner;
}

quint32 XdgPopupV6Interface::sendConfigure(const QRect &rect)
{
    const quint32 serial = xdgSurface()->shell()->display()->nextSerial();

    d->send_configure(rect.x(), rect.y(), rect.width(), rect.height());

    auto xdgSurfacePrivate = XdgSurfaceV6InterfacePrivate::get(xdgSurface());
    xdgSurfacePrivate->send_configure(serial);
    xdgSurfacePrivate->isConfigured = true;

    return serial;
}

void XdgPopupV6Interface::sendPopupDone()
{
    d->send_popup_done();
}

void XdgPopupV6Interface::sendRepositioned(quint32 token)
{
}

XdgPopupV6Interface *XdgPopupV6Interface::get(::wl_resource *resource)
{
    if (auto popupPrivate = resource_cast<XdgPopupV6InterfacePrivate *>(resource)) {
        return popupPrivate->q;
    }
    return nullptr;
}

XdgPositionerV6Private::XdgPositionerV6Private(::wl_resource *resource)
    : data(new XdgPositionerV6Data)
{
    init(resource);
}

XdgPositionerV6Private *XdgPositionerV6Private::get(wl_resource *resource)
{
    return resource_cast<XdgPositionerV6Private *>(resource);
}

void XdgPositionerV6Private::zxdg_positioner_v6_destroy_resource(Resource *resource)
{
    delete this;
}

void XdgPositionerV6Private::zxdg_positioner_v6_destroy(Resource *resource)
{
    wl_resource_destroy(resource->handle);
}

void XdgPositionerV6Private::zxdg_positioner_v6_set_size(Resource *resource, int32_t width, int32_t height)
{
    if (width < 1 || height < 1) {
        wl_resource_post_error(resource->handle, error_invalid_input, "width and height must be positive and non-zero");
        return;
    }
    data->size = QSize(width, height);
}

void XdgPositionerV6Private::zxdg_positioner_v6_set_anchor_rect(Resource *resource, int32_t x, int32_t y, int32_t width, int32_t height)
{
    if (width < 1 || height < 1) {
        wl_resource_post_error(resource->handle, error_invalid_input, "width and height must be positive and non-zero");
        return;
    }
    data->anchorRect = QRect(x, y, width, height);
}

void XdgPositionerV6Private::zxdg_positioner_v6_set_anchor(Resource *resource, uint32_t anchor)
{
    if ((anchor & anchor_left) && (anchor & anchor_right)) {
        wl_resource_post_error(resource->handle, ZXDG_POSITIONER_V6_ERROR_INVALID_INPUT, "Invalid arguments");
        return;
    }
    if ((anchor & anchor_top) && (anchor & anchor_bottom)) {
        wl_resource_post_error(resource->handle, ZXDG_POSITIONER_V6_ERROR_INVALID_INPUT, "Invalid arguments");
        return;
    }


    if (anchor & anchor_top) {
        data->anchorEdges |= Qt::TopEdge;
    }
    if (anchor & anchor_right) {
        data->anchorEdges |= Qt::RightEdge;
    }
    if (anchor & anchor_bottom) {
        data->anchorEdges |= Qt::BottomEdge;
    }
    if (anchor & anchor_left) {
        data->anchorEdges |= Qt::LeftEdge;
    }
}

void XdgPositionerV6Private::zxdg_positioner_v6_set_gravity(Resource *resource, uint32_t gravity)
{
    if ((gravity & gravity_left) && (gravity & gravity_right)) {
        wl_resource_post_error(resource->handle, ZXDG_POSITIONER_V6_ERROR_INVALID_INPUT, "Invalid arguments");
        return;
    }
    if ((gravity & gravity_top) && (gravity & gravity_bottom)) {
        wl_resource_post_error(resource->handle, ZXDG_POSITIONER_V6_ERROR_INVALID_INPUT, "Invalid arguments");
        return;
    }


    if (gravity & gravity_top) {
        data->gravityEdges |= Qt::TopEdge;
    }
    if (gravity & gravity_right) {
        data->gravityEdges |= Qt::RightEdge;
    }
    if (gravity & gravity_bottom) {
        data->gravityEdges |= Qt::BottomEdge;
    }
    if (gravity & gravity_left) {
        data->gravityEdges |= Qt::LeftEdge;
    }
}

void XdgPositionerV6Private::zxdg_positioner_v6_set_constraint_adjustment(Resource *resource, uint32_t constraint_adjustment)
{
    if (constraint_adjustment & constraint_adjustment_flip_x) {
        data->flipConstraintAdjustments |= Qt::Horizontal;
    } else {
        data->flipConstraintAdjustments &= ~Qt::Horizontal;
    }

    if (constraint_adjustment & constraint_adjustment_flip_y) {
        data->flipConstraintAdjustments |= Qt::Vertical;
    } else {
        data->flipConstraintAdjustments &= ~Qt::Vertical;
    }

    if (constraint_adjustment & constraint_adjustment_slide_x) {
        data->slideConstraintAdjustments |= Qt::Horizontal;
    } else {
        data->slideConstraintAdjustments &= ~Qt::Horizontal;
    }

    if (constraint_adjustment & constraint_adjustment_slide_y) {
        data->slideConstraintAdjustments |= Qt::Vertical;
    } else {
        data->slideConstraintAdjustments &= ~Qt::Vertical;
    }

    if (constraint_adjustment & constraint_adjustment_resize_x) {
        data->resizeConstraintAdjustments |= Qt::Horizontal;
    } else {
        data->resizeConstraintAdjustments &= ~Qt::Horizontal;
    }

    if (constraint_adjustment & constraint_adjustment_resize_y) {
        data->resizeConstraintAdjustments |= Qt::Vertical;
    } else {
        data->resizeConstraintAdjustments &= ~Qt::Vertical;
    }
}

void XdgPositionerV6Private::zxdg_positioner_v6_set_offset(Resource *resource, int32_t x, int32_t y)
{
    data->offset = QPoint(x, y);
}

XdgPositionerV6::XdgPositionerV6()
    : d(new XdgPositionerV6Data)
{
}

XdgPositionerV6::XdgPositionerV6(const XdgPositionerV6 &other)
    : d(other.d)
{
}

XdgPositionerV6::~XdgPositionerV6()
{
}

XdgPositionerV6 &XdgPositionerV6::operator=(const XdgPositionerV6 &other)
{
    d = other.d;
    return *this;
}

bool XdgPositionerV6::isComplete() const
{
    return d->size.isValid() && d->anchorRect.isValid();
}

Qt::Orientations XdgPositionerV6::slideConstraintAdjustments() const
{
    return d->slideConstraintAdjustments;
}

Qt::Orientations XdgPositionerV6::flipConstraintAdjustments() const
{
    return d->flipConstraintAdjustments;
}

Qt::Orientations XdgPositionerV6::resizeConstraintAdjustments() const
{
    return d->resizeConstraintAdjustments;
}

Qt::Edges XdgPositionerV6::anchorEdges() const
{
    return d->anchorEdges;
}

Qt::Edges XdgPositionerV6::gravityEdges() const
{
    return d->gravityEdges;
}

QSize XdgPositionerV6::size() const
{
    return d->size;
}

QRect XdgPositionerV6::anchorRect() const
{
    return d->anchorRect;
}

QPoint XdgPositionerV6::offset() const
{
    return d->offset;
}

QSize XdgPositionerV6::parentSize() const
{
    return d->parentSize;
}

bool XdgPositionerV6::isReactive() const
{
    return d->isReactive;
}

quint32 XdgPositionerV6::parentConfigure() const
{
    return d->parentConfigure;
}

XdgPositionerV6 XdgPositionerV6::get(::wl_resource *resource)
{
    XdgPositionerV6Private *xdgPositionerPrivate = XdgPositionerV6Private::get(resource);
    if (xdgPositionerPrivate)
        return XdgPositionerV6(xdgPositionerPrivate->data);
    return XdgPositionerV6();
}

XdgPositionerV6::XdgPositionerV6(const QSharedDataPointer<XdgPositionerV6Data> &data)
    : d(data)
{
}

} // namespace KWaylandServer
