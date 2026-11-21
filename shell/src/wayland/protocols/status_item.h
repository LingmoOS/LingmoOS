// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include <wayland-server-core.h>

namespace Lingmo::WM {
class StatusItems;
}

namespace Lingmo::Wayland {

// lingmo_status_item_manager_v1, over the window manager's status items.
wl_global *createStatusItemGlobal(wl_display *display, wm::StatusItems *items);

} // namespace Lingmo::Wayland

