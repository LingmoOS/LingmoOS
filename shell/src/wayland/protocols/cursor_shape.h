// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

struct wl_display;
struct wl_global;

namespace Lingmo::Wayland {

wl_global* createCursorShapeGlobal(wl_display* display);

} // namespace Lingmo::Wayland
