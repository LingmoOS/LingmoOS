// Copyright (C) 2023 Dingyuan Zhang <lxz@mkacg.com>.
// SPDX-License-Identifier: Apache-2.0 OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "foreigntoplevelmanagerv1.h"

#include "server-protocol.h"

#include <wayland-server-core.h>
#include <wayland-server.h>
#include <wayland-util.h>
#include <woutput.h>
#include <wxdgshell.h>
#include <wxdgsurface.h>
#include <wxwaylandsurface.h>

#include <qwcompositor.h>
#include <qwdisplay.h>
#include <qwoutput.h>
#include <qwsignalconnector.h>
#include <qwxdgshell.h>

#include <QDebug>
#include <QFile>
#include <QRegularExpression>
#include <QTimer>

#include <sys/socket.h>
#include <unistd.h>

extern "C" {
#define static
#include <wlr/types/wlr_xdg_shell.h>
#undef static
}

static ForeignToplevelV1 *FOREIGN_TOPLEVEL_MANAGER = nullptr;

ForeignToplevelAttached::ForeignToplevelAttached(WSurface *target, ForeignToplevelV1 *manager)
    : QObject(target)
    , m_target(target)
    , m_manager(manager)
{
    connect(manager,
            &ForeignToplevelV1::requestActivate,
            this,
            [this](WToplevelSurface *surface,
                   [[maybe_unused]] treeland_foreign_toplevel_handle_v1_activated_event *event) {
                if (surface->surface() != m_target) {
                    return;
                }

                Q_EMIT requestActivate(true);
            });
    connect(manager,
            &ForeignToplevelV1::requestMinimize,
            this,
            [this](WToplevelSurface *surface,
                   treeland_foreign_toplevel_handle_v1_minimized_event *event) {
                if (surface->surface() != m_target) {
                    return;
                }

                Q_EMIT requestMinimize(event->minimized);
            });
    connect(manager,
            &ForeignToplevelV1::requestMaximize,
            this,
            [this](WToplevelSurface *surface,
                   treeland_foreign_toplevel_handle_v1_maximized_event *event) {
                if (surface->surface() != m_target) {
                    return;
                }

                Q_EMIT requestMaximize(event->maximized);
            });
    connect(manager,
            &ForeignToplevelV1::requestFullscreen,
            this,
            [this](WToplevelSurface *surface,
                   treeland_foreign_toplevel_handle_v1_fullscreen_event *event) {
                if (surface->surface() != m_target) {
                    return;
                }

                Q_EMIT requestFullscreen(event->fullscreen);
            });
    connect(manager, &ForeignToplevelV1::requestClose, this, [this](WToplevelSurface *surface) {
        if (surface->surface() != m_target) {
            return;
        }

        Q_EMIT requestClose();
    });
    connect(manager,
            &ForeignToplevelV1::rectangleChanged,
            this,
            [this](WToplevelSurface *surface,
                   treeland_foreign_toplevel_handle_v1_set_rectangle_event *event) {
                if (surface->surface() != m_target) {
                    return;
                }

                Q_EMIT rectangleChanged({ event->x, event->y, event->width, event->height });
            });
}

ForeignToplevelV1::ForeignToplevelV1(QObject *parent)
    : QObject(parent)
{
    if (FOREIGN_TOPLEVEL_MANAGER) {
        qFatal("There are multiple instances of QuickForeignToplevelManagerV1");
    }

    FOREIGN_TOPLEVEL_MANAGER = this;
}

void ForeignToplevelV1::create(WServer *server)
{
    m_manager = treeland_foreign_toplevel_manager_v1::create(server->handle());

    connect(m_manager,
            &treeland_foreign_toplevel_manager_v1::dockPreviewContextCreated,
            this,
            &ForeignToplevelV1::onDockPreviewContextCreated);
}

void ForeignToplevelV1::destroy(WServer *server) { }

wl_global *ForeignToplevelV1::global() const
{
    return m_manager->global;
}

void ForeignToplevelV1::add(WToplevelSurface *surface)
{
    /* TODO: ocean-desktop and ocean-launchpad are not supported yet */
    wl_client *client = surface->surface()->handle()->handle()->resource->client;
    pid_t pid;
    wl_client_get_credentials(client, &pid, nullptr, nullptr);

    QString programName;
    QFile file(QString("/proc/%1/status").arg(pid));
    if (file.open(QFile::ReadOnly)) {
        programName =
            QString(file.readLine()).section(QRegularExpression("([\\t ]*:[\\t ]*|\\n)"), 1, 1);
    }

    if (programName == "ocean-desktop" || programName == "ocean-launchpad") {
        return;
    }
    /* TODO: end */

    auto handle = std::shared_ptr<treeland_foreign_toplevel_handle_v1>(
        treeland_foreign_toplevel_handle_v1::create(m_manager));
    m_surfaces.insert({ surface, handle });

    // initSurface
    std::vector<QMetaObject::Connection> connection;

    connection.push_back(surface->safeConnect(&WToplevelSurface::titleChanged, this, [=] {
        handle->set_title(surface->title());
    }));

    connection.push_back(surface->safeConnect(&WToplevelSurface::appIdChanged, this, [=] {
        handle->set_app_id(surface->appId());
    }));

    connection.push_back(surface->safeConnect(&WToplevelSurface::minimizeChanged, this, [=] {
        handle->set_minimized(surface->isMinimized());
    }));

    connection.push_back(surface->safeConnect(&WToplevelSurface::maximizeChanged, this, [=] {
        handle->set_maximized(surface->isMaximized());
    }));

    connection.push_back(surface->safeConnect(&WToplevelSurface::fullscreenChanged, this, [=] {
        handle->set_fullscreen(surface->isFullScreen());
    }));

    connection.push_back(surface->safeConnect(&WToplevelSurface::activateChanged, this, [=] {
        handle->set_activated(surface->isActivated());
    }));

    connection.push_back(
        connect(surface, &WToplevelSurface::parentSurfaceChanged, this, [this, surface, handle] {
            auto find = std::find_if(m_surfaces.begin(), m_surfaces.end(), [surface](auto pair) {
                return pair.first == surface;
            });

            handle->setParent(find != m_surfaces.end() ? find->second.get() : nullptr);
        }));

    connection.push_back(
        connect(surface->surface(), &WSurface::outputEntered, this, [handle](WOutput *output) {
            handle->output_enter(output->handle());
        }));

    connection.push_back(
        connect(surface->surface(), &WSurface::outputLeft, this, [handle](WOutput *output) {
            handle->output_leave(output->handle());
        }));

    connection.push_back(connect(
        handle.get(),
        &treeland_foreign_toplevel_handle_v1::requestActivate,
        this,
        [surface, this, handle](treeland_foreign_toplevel_handle_v1_activated_event *event) {
            Q_EMIT requestActivate(surface, event);
        }));

    connection.push_back(
        connect(handle.get(),
                &treeland_foreign_toplevel_handle_v1::requestMaximize,
                this,
                [surface, this](treeland_foreign_toplevel_handle_v1_maximized_event *event) {
                    Q_EMIT requestMaximize(surface, event);
                }));

    connection.push_back(connect(
        handle.get(),
        &treeland_foreign_toplevel_handle_v1::requestMinimize,
        this,
        [surface, this, handle](treeland_foreign_toplevel_handle_v1_minimized_event *event) {
            Q_EMIT requestMinimize(surface, event);
        }));

    connection.push_back(
        connect(handle.get(),
                &treeland_foreign_toplevel_handle_v1::requestFullscreen,
                this,
                [surface, this](treeland_foreign_toplevel_handle_v1_fullscreen_event *event) {
                    Q_EMIT requestFullscreen(surface, event);
                }));

    connection.push_back(connect(handle.get(),
                                 &treeland_foreign_toplevel_handle_v1::requestClose,
                                 this,
                                 [surface, this] {
                                     Q_EMIT requestClose(surface);
                                 }));

    if (auto *xdgSurface = qobject_cast<WXdgSurface *>(surface)) {
        wl_client *client = xdgSurface->handle()->handle()->resource->client;
        pid_t pid;
        wl_client_get_credentials(client, &pid, nullptr, nullptr);
        handle->set_pid(pid);
    } else if (auto *xwaylandSurface = qobject_cast<WXWaylandSurface *>(surface)) {
        handle->set_pid(xwaylandSurface->pid());
    } else {
        qFatal("TreelandForeignToplevelManager only support WXdgSurface or WXWaylandSurface");
    }

    handle->set_identifier(
        *reinterpret_cast<const uint32_t *>(surface->surface()->handle()->handle()));

    handle->set_title(surface->title());
    handle->set_app_id(surface->appId());
    handle->set_minimized(surface->isMinimized());
    handle->set_maximized(surface->isMaximized());
    handle->set_fullscreen(surface->isFullScreen());
    handle->set_activated(surface->isActivated());

    m_connections.insert({ surface, connection });
}

void ForeignToplevelV1::remove(WToplevelSurface *surface)
{
    if (!m_surfaces.count(surface)) {
        return;
    }

    for (auto co : m_connections[surface]) {
        QObject::disconnect(co);
    }

    m_connections.erase(surface);
    m_surfaces.erase(surface);
}

void ForeignToplevelV1::enterDockPreview(WSurface *relative_surface)
{
    for (auto *context : m_dockPreviews) {
        if (context->relative_surface == relative_surface->handle()->handle()->resource) {
            context->enter();
            break;
        }
    }
}

void ForeignToplevelV1::leaveDockPreview(WSurface *relative_surface)
{
    for (auto *context : m_dockPreviews) {
        if (context->relative_surface == relative_surface->handle()->handle()->resource) {
            context->leave();
            break;
        }
    }
}

void ForeignToplevelV1::onDockPreviewContextCreated(treeland_dock_preview_context_v1 *context)
{
    m_dockPreviews.push_back(context);
    connect(context, &treeland_dock_preview_context_v1::beforeDestroy, this, [this, context] {
        std::erase_if(m_dockPreviews, [context](auto *p) {
            return p == context;
        });
    });
    connect(context,
            &treeland_dock_preview_context_v1::requestShow,
            this,
            [this](treeland_dock_preview_context_v1_preview_event *event) {
                std::vector<WSurface *> surfaces;
                for (auto it = event->toplevels.cbegin(); it != event->toplevels.cend(); ++it) {
                    const uint32_t identifier = *it;
                    for (auto it = m_surfaces.cbegin(); it != m_surfaces.cend(); ++it) {
                        if (it->second->identifier == identifier) {
                            surfaces.push_back(it->first->surface());
                            break;
                        }
                    }
                };

                Q_EMIT requestDockPreview(surfaces,
                                          WSurface::fromHandle(wlr_surface_from_resource(
                                              event->toplevel->relative_surface)),
                                          QPoint(event->x, event->y),
                                          event->direction);
            });

    connect(context,
            &treeland_dock_preview_context_v1::requestClose,
            this,
            &ForeignToplevelV1::requestDockClose);
}

ForeignToplevelAttached *ForeignToplevelV1::Attached(QObject *target)
{
    if (auto *surface = qobject_cast<WXdgSurface *>(target)) {
        return new ForeignToplevelAttached(surface->surface(), FOREIGN_TOPLEVEL_MANAGER);
    }

    if (auto *surface = qobject_cast<WXWaylandSurface *>(target)) {
        return new ForeignToplevelAttached(surface->surface(), FOREIGN_TOPLEVEL_MANAGER);
    }

    return nullptr;
}
