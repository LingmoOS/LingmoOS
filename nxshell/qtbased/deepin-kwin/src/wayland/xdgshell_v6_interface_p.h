/*
    SPDX-FileCopyrightText: 2020 Vlad Zahorodnii <vlad.zahorodnii@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#pragma once

#include "qwayland-server-xdg-shell-unstable-v6.h"
#include "xdgshell_v6_interface.h"

#include "surface_interface.h"
#include "surfacerole_p.h"

namespace KWaylandServer
{
class XdgToplevelV6DecorationV1Interface;

class XdgShellV6InterfacePrivate : public QtWaylandServer::zxdg_shell_v6
{
public:
    XdgShellV6InterfacePrivate(XdgShellV6Interface *shell);

    Resource *resourceForXdgSurface(XdgSurfaceV6Interface *surface) const;

    void unregisterXdgSurface(XdgSurfaceV6Interface *surface);

    void registerPing(quint32 serial);

    static XdgShellV6InterfacePrivate *get(XdgShellV6Interface *shell);

    XdgShellV6Interface *q;
    Display *display;
    QMap<quint32, QTimer *> pings;

protected:
    void zxdg_shell_v6_destroy(Resource *resource) override;
    void zxdg_shell_v6_create_positioner(Resource *resource, uint32_t id) override;
    void zxdg_shell_v6_get_xdg_surface(Resource *resource, uint32_t id, ::wl_resource *surface) override;
    void zxdg_shell_v6_pong(Resource *resource, uint32_t serial) override;

private:
    QHash<XdgSurfaceV6Interface *, Resource *> xdgSurfaces;
};

class XdgPositionerV6Data : public QSharedData
{
public:
    Qt::Orientations slideConstraintAdjustments;
    Qt::Orientations flipConstraintAdjustments;
    Qt::Orientations resizeConstraintAdjustments;
    Qt::Edges anchorEdges;
    Qt::Edges gravityEdges;
    QPoint offset;
    QSize size;
    QRect anchorRect;
    bool isReactive = false;
    QSize parentSize;
    quint32 parentConfigure;
};

class XdgPositionerV6Private : public QtWaylandServer::zxdg_positioner_v6
{
public:
    XdgPositionerV6Private(::wl_resource *resource);

    QSharedDataPointer<XdgPositionerV6Data> data;

    static XdgPositionerV6Private *get(::wl_resource *resource);

protected:
    void zxdg_positioner_v6_destroy_resource(Resource *resource) override;
    void zxdg_positioner_v6_destroy(Resource *resource) override;
    void zxdg_positioner_v6_set_size(Resource *resource, int32_t width, int32_t height) override;
    void zxdg_positioner_v6_set_anchor_rect(Resource *resource, int32_t x, int32_t y, int32_t width, int32_t height) override;
    void zxdg_positioner_v6_set_anchor(Resource *resource, uint32_t anchor) override;
    void zxdg_positioner_v6_set_gravity(Resource *resource, uint32_t gravity) override;
    void zxdg_positioner_v6_set_constraint_adjustment(Resource *resource, uint32_t constraint_adjustment) override;
    void zxdg_positioner_v6_set_offset(Resource *resource, int32_t x, int32_t y) override;
};

struct XdgSurfaceV6State
{
    QRect windowGeometry;
    quint32 acknowledgedConfigure;
    bool acknowledgedConfigureIsSet = false;
    bool windowGeometryIsSet = false;
};

class XdgSurfaceV6InterfacePrivate : public QtWaylandServer::zxdg_surface_v6
{
public:
    XdgSurfaceV6InterfacePrivate(XdgSurfaceV6Interface *xdgSurface);

    void commit();
    void reset();

    XdgSurfaceV6Interface *q;
    XdgShellV6Interface *shell;
    QPointer<XdgToplevelV6Interface> toplevel;
    QPointer<XdgPopupV6Interface> popup;
    QPointer<SurfaceInterface> surface;
    bool firstBufferAttached = false;
    bool isConfigured = false;
    bool isInitialized = false;

    XdgSurfaceV6State next;
    XdgSurfaceV6State current;

    static XdgSurfaceV6InterfacePrivate *get(XdgSurfaceV6Interface *surface);

protected:
    void zxdg_surface_v6_destroy_resource(Resource *resource) override;
    void zxdg_surface_v6_destroy(Resource *resource) override;
    void zxdg_surface_v6_get_toplevel(Resource *resource, uint32_t id) override;
    void zxdg_surface_v6_get_popup(Resource *resource, uint32_t id, ::wl_resource *parent, ::wl_resource *positioner) override;
    void zxdg_surface_v6_set_window_geometry(Resource *resource, int32_t x, int32_t y, int32_t width, int32_t height) override;
    void zxdg_surface_v6_ack_configure(Resource *resource, uint32_t serial) override;
};

class XdgToplevelV6InterfacePrivate : public SurfaceRole, public QtWaylandServer::zxdg_toplevel_v6
{
public:
    XdgToplevelV6InterfacePrivate(XdgToplevelV6Interface *toplevel, XdgSurfaceV6Interface *surface);

    void commit() override;
    void reset();

    static XdgToplevelV6InterfacePrivate *get(XdgToplevelV6Interface *toplevel);
    static XdgToplevelV6InterfacePrivate *get(::wl_resource *resource);

    XdgToplevelV6Interface *q;
    QPointer<XdgToplevelV6Interface> parentXdgToplevel;
    QPointer<XdgToplevelV6DecorationV1Interface> decoration;
    XdgSurfaceV6Interface *xdgSurface;

    QString windowTitle;
    QString windowClass;

    struct State
    {
        QSize minimumSize;
        QSize maximumSize;
    };

    State next;
    State current;

protected:
    void zxdg_toplevel_v6_destroy_resource(Resource *resource) override;
    void zxdg_toplevel_v6_destroy(Resource *resource) override;
    void zxdg_toplevel_v6_set_parent(Resource *resource, ::wl_resource *parent) override;
    void zxdg_toplevel_v6_set_title(Resource *resource, const QString &title) override;
    void zxdg_toplevel_v6_set_app_id(Resource *resource, const QString &app_id) override;
    void zxdg_toplevel_v6_show_window_menu(Resource *resource, ::wl_resource *seat, uint32_t serial, int32_t x, int32_t y) override;
    void zxdg_toplevel_v6_move(Resource *resource, ::wl_resource *seat, uint32_t serial) override;
    void zxdg_toplevel_v6_resize(Resource *resource, ::wl_resource *seat, uint32_t serial, uint32_t edges) override;
    void zxdg_toplevel_v6_set_max_size(Resource *resource, int32_t width, int32_t height) override;
    void zxdg_toplevel_v6_set_min_size(Resource *resource, int32_t width, int32_t height) override;
    void zxdg_toplevel_v6_set_maximized(Resource *resource) override;
    void zxdg_toplevel_v6_unset_maximized(Resource *resource) override;
    void zxdg_toplevel_v6_set_fullscreen(Resource *resource, ::wl_resource *output) override;
    void zxdg_toplevel_v6_unset_fullscreen(Resource *resource) override;
    void zxdg_toplevel_v6_set_minimized(Resource *resource) override;
};

class XdgPopupV6InterfacePrivate : public SurfaceRole, public QtWaylandServer::zxdg_popup_v6
{
public:
    static XdgPopupV6InterfacePrivate *get(XdgPopupV6Interface *popup);

    XdgPopupV6InterfacePrivate(XdgPopupV6Interface *popup, XdgSurfaceV6Interface *surface);

    void commit() override;
    void reset();

    XdgPopupV6Interface *q;
    SurfaceInterface *parentSurface;
    XdgSurfaceV6Interface *xdgSurface;
    XdgPositionerV6 positioner;

protected:
    void zxdg_popup_v6_destroy_resource(Resource *resource) override;
    void zxdg_popup_v6_destroy(Resource *resource) override;
    void zxdg_popup_v6_grab(Resource *resource, ::wl_resource *seat, uint32_t serial) override;
};

} // namespace KWaylandServer
