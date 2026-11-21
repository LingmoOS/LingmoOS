// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef LINGMO_WAYLAND_PROTOCOLS_WINDOW_PLACEMENT_H_
#define LINGMO_WAYLAND_PROTOCOLS_WINDOW_PLACEMENT_H_

#include <functional>
#include <vector>

#include <wayland-server-core.h>

#include "base/signal.h"

namespace Lingmo::WM {
class Window;
class WindowManager;
}

namespace Lingmo::Wayland {

// lingmo_window_placement_v1: windows a client places itself and whose
// positions it hears. Upstream: exo's zaura_toplevel set_window_bounds and
// origin_change, which its SecurityDelegate::CanSetBounds gates per client.
class WaylandWindowPlacement
{
public:
    // Only clients `authorize` accepts may bind the global; the registry filter
    // hides it from the rest.
    WaylandWindowPlacement(wl_display *display, wm::WindowManager *windows,
                           std::function<bool(const wl_client *)> authorize);
    ~WaylandWindowPlacement();

    WaylandWindowPlacement(const WaylandWindowPlacement &) = delete;
    WaylandWindowPlacement &operator=(const WaylandWindowPlacement &) = delete;

    wl_global *global() const { return m_global; }

private:
    friend struct WindowPlacementRequests;

    static void bind(wl_client *client, void *data, uint32_t version, uint32_t id);
    void sendPosition(wm::Window *window);

    wm::WindowManager *m_windows;
    std::function<bool(const wl_client *)> m_authorize;
    wl_global *m_global = nullptr;
    std::vector<wl_resource *> m_resources;
    base::Subscription m_windowAdded;
    base::Subscription m_windowMoved;
};

} // namespace Lingmo::Wayland

#endif // LINGMO_WAYLAND_PROTOCOLS_WINDOW_PLACEMENT_H_
