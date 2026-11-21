// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "wayland/protocols/window_placement.h"

#include <algorithm>

#include <lingmo-window-placement-v1-server-protocol.h>

#include "compositor/surface.h"
#include "wayland/server_util.h"
#include "wm/window.h"
#include "wm/window_manager.h"

namespace Lingmo::Wayland {

struct WindowPlacementRequests {
    static void destroy(wl_client *, wl_resource *resource)
    {
        wl_resource_destroy(resource);
    }

    static void setPosition(wl_client *, wl_resource *resource, wl_resource *surfaceResource,
                            int32_t x, int32_t y)
    {
        auto *self = static_cast<WaylandWindowPlacement *>(wl_resource_get_user_data(resource));
        if (!self)
            return;
        auto *surface = getUserDataAs<compositor::Surface>(surfaceResource);
        wm::Window *window = self->m_windows->windowForSurface(surface);
        if (window && !window->isPopup())
            self->m_windows->moveWindowTo(window, base::Point(x, y));
        else
            surface->setRequestedPosition(base::Point(x, y));
    }

    static void unbind(wl_resource *resource)
    {
        auto *self = static_cast<WaylandWindowPlacement *>(wl_resource_get_user_data(resource));
        if (self)
            std::erase(self->m_resources, resource);
    }
};

static const struct lingmo_window_placement_v1_interface implementation = {
        WindowPlacementRequests::destroy,
        WindowPlacementRequests::setPosition,
};

WaylandWindowPlacement::WaylandWindowPlacement(wl_display *display, wm::WindowManager *windows,
                                               std::function<bool(const wl_client *)> authorize)
    : m_windows(windows)
    , m_authorize(std::move(authorize))
{
    m_global = wl_global_create(display, &lingmo_window_placement_v1_interface, 1, this, bind);
    m_windowAdded = m_windows->windowAdded.connect([this](wm::Window *window) {
        sendPosition(window);
    });
    m_windowMoved = m_windows->windowMoved.connect([this](wm::Window *window) {
        sendPosition(window);
    });
}

WaylandWindowPlacement::~WaylandWindowPlacement()
{
    for (wl_resource *resource : m_resources)
        wl_resource_set_user_data(resource, nullptr);
    if (m_global)
        wl_global_destroy(m_global);
}

void WaylandWindowPlacement::bind(wl_client *client, void *data, uint32_t version, uint32_t id)
{
    auto *self = static_cast<WaylandWindowPlacement *>(data);
    if (!self->m_authorize(client)) {
        wl_client_post_implementation_error(client, "Window placement not authorized");
        return;
    }
    wl_resource *resource =
            wl_resource_create(client, &lingmo_window_placement_v1_interface, version, id);
    if (!resource) {
        wl_client_post_no_memory(client);
        return;
    }
    wl_resource_set_implementation(resource, &implementation, self,
                                   WindowPlacementRequests::unbind);
    self->m_resources.push_back(resource);
}

void WaylandWindowPlacement::sendPosition(wm::Window *window)
{
    if (m_resources.empty() || window->isPopup() || !window->rootSurface())
        return;
    wl_resource *surface = getSurfaceResource(window->rootSurface());
    if (!surface)
        return;
    wl_client *client = wl_resource_get_client(surface);
    const base::Point position = window->contentBounds().topLeft();
    for (wl_resource *resource : m_resources) {
        if (wl_resource_get_client(resource) == client)
            lingmo_window_placement_v1_send_position(resource, surface, position.x(),
                                                        position.y());
    }
}

} // namespace Lingmo::Wayland
