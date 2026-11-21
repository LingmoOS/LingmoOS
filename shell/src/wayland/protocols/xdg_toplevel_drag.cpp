// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "wayland/protocols/xdg_toplevel_drag.h"

#include <xdg-toplevel-drag-v1-server-protocol.h>

#include <algorithm>
#include <memory>

#include "wayland/protocols/wl_data_device_manager.h"
#include "wayland/protocols/xdg_shell.h"
#include "wayland/server_util.h"
#include "wm/shell_surface.h"
#include "wm/toplevel_drag.h"

namespace Lingmo::Wayland {

namespace {

////////////////////////////////////////////////////////////////////////////////
// xdg_toplevel_drag_v1_interface:

void drag_destroy(wl_client *, wl_resource *resource)
{
    if (getUserDataAs<wm::ToplevelDrag>(resource)->isDragging()) {
        wl_resource_post_error(resource, XDG_TOPLEVEL_DRAG_V1_ERROR_ONGOING_DRAG,
                               "the drag has not ended");
        return;
    }
    wl_resource_destroy(resource);
}

void drag_attach(wl_client *, wl_resource *resource, wl_resource *toplevel, int32_t x, int32_t y)
{
    auto *drag = getUserDataAs<wm::ToplevelDrag>(resource);
    if (drag->hasToplevel()) {
        wl_resource_post_error(resource, XDG_TOPLEVEL_DRAG_V1_ERROR_TOPLEVEL_ATTACHED,
                               "a toplevel is already attached");
        return;
    }
    // A toplevel whose xdg_surface or wl_surface is gone has nothing left to move.
    wm::ShellSurface *shellSurface = shellSurfaceForToplevel(toplevel);
    if (shellSurface && shellSurface->rootSurface()) {
        shellSurface->dropInheritedState();
        drag->attach(shellSurface->rootSurface(), base::Point(x, y));
    }
}

const struct xdg_toplevel_drag_v1_interface drag_implementation = {
        drag_destroy,
        drag_attach,
};

////////////////////////////////////////////////////////////////////////////////
// xdg_toplevel_drag_manager_v1_interface:

void manager_destroy(wl_client *, wl_resource *resource)
{
    wl_resource_destroy(resource);
}

void manager_get_xdg_toplevel_drag(wl_client *client, wl_resource *resource, uint32_t id,
                                   wl_resource *sourceResource)
{
    auto source = claimDragSource(client, sourceResource);
    if (!source) {
        wl_resource_post_error(resource, XDG_TOPLEVEL_DRAG_MANAGER_V1_ERROR_INVALID_SOURCE,
                               "the data source is used or already has a toplevel drag");
        return;
    }
    wl_resource *drag = wl_resource_create(client, &xdg_toplevel_drag_v1_interface,
                                           wl_resource_get_version(resource), id);
    if (!drag) {
        wl_client_post_no_memory(client);
        return;
    }
    auto *windowManager = static_cast<wm::WindowManager *>(wl_resource_get_user_data(resource));
    setImplementation(drag, &drag_implementation,
                      std::make_unique<wm::ToplevelDrag>(windowManager, std::move(source)));
}

const struct xdg_toplevel_drag_manager_v1_interface manager_implementation = {
        manager_destroy,
        manager_get_xdg_toplevel_drag,
};

void bindXdgToplevelDragManager(wl_client *client, void *data, uint32_t version, uint32_t id)
{
    wl_resource *resource = wl_resource_create(client, &xdg_toplevel_drag_manager_v1_interface,
                                               std::min(kXdgToplevelDragManagerVersion, version),
                                               id);
    if (!resource) {
        wl_client_post_no_memory(client);
        return;
    }
    wl_resource_set_implementation(resource, &manager_implementation, data, nullptr);
}

} // namespace

wl_global *createXdgToplevelDragGlobal(wl_display *display, wm::WindowManager *windowManager)
{
    return wl_global_create(display, &xdg_toplevel_drag_manager_v1_interface,
                            kXdgToplevelDragManagerVersion, windowManager,
                            &bindXdgToplevelDragManager);
}

} // namespace Lingmo::Wayland
