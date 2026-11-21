// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef LINGMO_WAYLAND_PROTOCOLS_XDG_DIALOG_H_
#define LINGMO_WAYLAND_PROTOCOLS_XDG_DIALOG_H_

#include <wayland-server-core.h>

#include <cstdint>

namespace Lingmo::Wayland {

constexpr uint32_t kXdgWmDialogVersion = 1;

// xdg_wm_dialog_v1. A modal xdg_dialog_v1 makes its toplevel window-modal on
// the toplevel's xdg_toplevel.set_parent.
wl_global *createXdgDialogGlobal(wl_display *display);

} // namespace Lingmo::Wayland

#endif // LINGMO_WAYLAND_PROTOCOLS_XDG_DIALOG_H_
