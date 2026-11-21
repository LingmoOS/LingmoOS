// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef LINGMO_WAYLAND_SURFACE_BLUR_H_
#define LINGMO_WAYLAND_SURFACE_BLUR_H_

#include <wayland-server-core.h>

namespace Lingmo::ui {
class Path;
}

namespace Lingmo::Wayland {

wl_global* createSurfaceBlurGlobal(wl_display* display);

// An array of lingmo_surface_blur_v1.path_command values and wl_fixed
// points as a path; false for a malformed one or one past 4096 values.
// lingmo_window_hints_v1 shares the encoding.
bool pathFromArray(const wl_array* array, ui::Path* result);

} // namespace Lingmo::Wayland

#endif
