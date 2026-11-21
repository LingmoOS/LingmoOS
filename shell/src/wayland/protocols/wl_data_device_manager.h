// Copyright 2017 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.chromium file.
//
// Upstream: chromium/src/components/exo/wayland/wl_data_device_manager.h
// Revision: 7acd40ddefff49c9d36144999aafdbb674dbd07a
//
// Clipboard selection and pointer drag-and-drop, wl_data_device_manager versions 1–3.
#pragma once

#include <cstdint>
#include <memory>
#include <wayland-server-core.h>

namespace Lingmo::Clipboard {
class DataSource;
}

namespace Lingmo::Wayland {

struct WaylandSeat;
constexpr uint32_t kWlDataDeviceManagerVersion = 3;
wl_global* createDataDeviceManagerGlobal(wl_display* display, WaylandSeat* seat);

// The source behind a wl_data_source for xdg_toplevel_drag_v1, which keeps it
// to drag-and-drop from then on. Null when `resource` is not a source of
// `client`, or was already used or claimed.
std::shared_ptr<clipboard::DataSource> claimDragSource(wl_client* client, wl_resource* resource);

} // namespace Lingmo::Wayland
