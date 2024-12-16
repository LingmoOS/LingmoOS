// Copyright (C) 2023 Dingyuan Zhang <lxz@mkacg.com>.
// SPDX-License-Identifier: Apache-2.0 OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#pragma once

#include "impl/foreign_toplevel_manager_impl.h"

#include <wserver.h>
#include <wxdgsurface.h>

#include <QObject>

QW_USE_NAMESPACE
WAYLIB_SERVER_USE_NAMESPACE

class ForeignToplevelV1;

class ForeignToplevelAttached : public QObject
{
    Q_OBJECT
    QML_ANONYMOUS

public:
    ForeignToplevelAttached(WSurface *target, ForeignToplevelV1 *manager);

Q_SIGNALS:
    void requestMaximize(bool maximized);
    void requestMinimize(bool minimized);
    void requestActivate(bool activated);
    void requestFullscreen(bool fullscreen);
    void requestClose();
    void rectangleChanged(const QRect &rect);

private:
    WSurface *m_target;
    ForeignToplevelV1 *m_manager;
};

class ForeignToplevelV1 : public QObject, public WServerInterface
{
    Q_OBJECT

public:
    explicit ForeignToplevelV1(QObject *parent = nullptr);

    Q_INVOKABLE void add(WToplevelSurface *surface);
    Q_INVOKABLE void remove(WToplevelSurface *surface);

    Q_INVOKABLE void enterDockPreview(WSurface *relative_surface);
    Q_INVOKABLE void leaveDockPreview(WSurface *relative_surface);

    Q_INVOKABLE ForeignToplevelAttached *Attached(QObject *target);

Q_SIGNALS:
    void requestMaximize(WToplevelSurface *surface,
                         treeland_foreign_toplevel_handle_v1_maximized_event *event);
    void requestMinimize(WToplevelSurface *surface,
                         treeland_foreign_toplevel_handle_v1_minimized_event *event);
    void requestActivate(WToplevelSurface *surface,
                         treeland_foreign_toplevel_handle_v1_activated_event *event);
    void requestFullscreen(WToplevelSurface *surface,
                           treeland_foreign_toplevel_handle_v1_fullscreen_event *event);
    void requestClose(WToplevelSurface *surface);
    void rectangleChanged(WToplevelSurface *surface,
                          treeland_foreign_toplevel_handle_v1_set_rectangle_event *event);
    void requestDockPreview(std::vector<WSurface *> surfaces,
                            WSurface *target,
                            QPoint abs,
                            int direction);
    void requestDockClose();

protected:
    void create(WServer *server) override;
    void destroy(WServer *server) override;
    wl_global *global() const override;

private Q_SLOTS:
    void onDockPreviewContextCreated(treeland_dock_preview_context_v1 *context);

private:
    treeland_foreign_toplevel_manager_v1 *m_manager = nullptr;
    std::map<WToplevelSurface *, std::shared_ptr<treeland_foreign_toplevel_handle_v1>> m_surfaces;
    std::map<WToplevelSurface *, std::vector<QMetaObject::Connection>> m_connections;
    std::vector<treeland_dock_preview_context_v1 *> m_dockPreviews; // TODO: remove m_dockPreviews
};

Q_DECLARE_OPAQUE_POINTER(treeland_foreign_toplevel_handle_v1_maximized_event *);
Q_DECLARE_OPAQUE_POINTER(treeland_foreign_toplevel_handle_v1_minimized_event *);
Q_DECLARE_OPAQUE_POINTER(treeland_foreign_toplevel_handle_v1_activated_event *);
Q_DECLARE_OPAQUE_POINTER(treeland_foreign_toplevel_handle_v1_fullscreen_event *);
Q_DECLARE_OPAQUE_POINTER(treeland_foreign_toplevel_handle_v1_set_rectangle_event *);
