// Copyright 2018 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.chromium file.
//
// Upstream: chromium/src/components/exo/wayland/zwp_pointer_gestures.h
// Revision: 7acd40ddefff49c9d36144999aafdbb674dbd07a

#pragma once
struct wl_display;
struct wl_global;

namespace Lingmo::Wayland {
struct WaylandSeat;
wl_global *createPointerGesturesGlobal(wl_display *display, WaylandSeat *seat);
} // namespace Lingmo::Wayland
