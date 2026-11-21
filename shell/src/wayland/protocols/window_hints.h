// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include <wayland-server-core.h>

namespace Lingmo::WM {
class WindowManager;
}

namespace Lingmo::Wayland {

// With `windows`, managers hear the work areas of its outputs.
wl_global *createWindowHintsGlobal(wl_display *display, wm::WindowManager *windows = nullptr);

// lingmo_window_hints_v1.interactive_resize, to every manager the surface's
// client has bound.
void sendInteractiveResize(wl_resource *surface, bool active);

} // namespace Lingmo::Wayland

