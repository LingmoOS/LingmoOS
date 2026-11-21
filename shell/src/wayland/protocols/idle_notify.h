// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

struct wl_display;
struct wl_global;

namespace Lingmo::Wayland {
class IdleInhibition;
wl_global *createIdleNotifierGlobal(wl_display *display, IdleInhibition *inhibition);
} // namespace Lingmo::Wayland
