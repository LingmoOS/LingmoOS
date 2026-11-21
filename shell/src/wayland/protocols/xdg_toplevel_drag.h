// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef LINGMO_WAYLAND_PROTOCOLS_XDG_TOPLEVEL_DRAG_H_
#define LINGMO_WAYLAND_PROTOCOLS_XDG_TOPLEVEL_DRAG_H_

#include <wayland-server-core.h>

#include <cstdint>

namespace Lingmo::WM {
class WindowManager;
}

namespace Lingmo::Wayland {

constexpr uint32_t kXdgToplevelDragManagerVersion = 1;

// xdg_toplevel_drag_manager_v1: a toplevel that follows the pointer through a
// wl_data_device drag, which is how a tab is pulled out into a window of its own.
wl_global *createXdgToplevelDragGlobal(wl_display *display, wm::WindowManager *windowManager);

} // namespace Lingmo::Wayland

#endif // LINGMO_WAYLAND_PROTOCOLS_XDG_TOPLEVEL_DRAG_H_
