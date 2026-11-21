// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once
#include <wayland-server-core.h>

namespace Lingmo::Wayland {
struct WaylandSeat;
wl_global *createPrimarySelectionGlobal(wl_display *display, WaylandSeat *seat);
} // namespace Lingmo::Wayland
