// Copyright 2019 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.chromium file.
//
// Upstream: chromium/src/components/exo/wayland/wl_compositor.h
// Revision: 7acd40ddefff49c9d36144999aafdbb674dbd07a
//
// Adapted for Lingmo: integrates the upstream behavior with native
// compositor types and ownership.

#ifndef LINGMO_WAYLAND_PROTOCOLS_WL_COMPOSITOR_H_
#define LINGMO_WAYLAND_PROTOCOLS_WL_COMPOSITOR_H_

#include <wayland-server-core.h>
#include <wayland-server-protocol.h>

#include <cstdint>

namespace Lingmo::Compositor {
class Compositor;
}

namespace Lingmo::Wayland {

// wl_surface.damage_buffer. Exo stops at version 3; Lingmo needs buffer
// damage, and does not implement wl_surface.offset, which is what version 5
// would add.
constexpr uint32_t kWlCompositorVersion = WL_SURFACE_DAMAGE_BUFFER_SINCE_VERSION;

wl_global* createCompositorGlobal(wl_display* display, compositor::Compositor* compositor);

} // namespace Lingmo::Wayland

#endif // LINGMO_WAYLAND_PROTOCOLS_WL_COMPOSITOR_H_
