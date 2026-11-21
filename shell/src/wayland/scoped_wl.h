// Copyright 2015 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.chromium file.
//
// Upstream: chromium/src/components/exo/wayland/scoped_wl.h
//           chromium/src/components/exo/wayland/scoped_wl.cc
// Revision: 7acd40ddefff49c9d36144999aafdbb674dbd07a
//
// Adapted for Lingmo: integrates the upstream behavior with native
// compositor types and ownership.

#ifndef LINGMO_WAYLAND_SCOPED_WL_H_
#define LINGMO_WAYLAND_SCOPED_WL_H_

#include <wayland-server-core.h>

#include <memory>

namespace Lingmo::Wayland {

struct WlDisplayDeleter {
    void operator()(wl_display* display) const
    {
        // Destroy all clients in the display manually, because
        // wl_display_destroy() doesn't do it.
        wl_list* client_list = wl_display_get_client_list(display);
        while (!wl_list_empty(client_list)) {
            wl_client* client = wl_client_from_link(client_list->next);
            wl_client_destroy(client);
        }

        wl_display_destroy(display);
    }
};

using ScopedWlDisplay = std::unique_ptr<wl_display, WlDisplayDeleter>;

} // namespace Lingmo::Wayland

#endif // LINGMO_WAYLAND_SCOPED_WL_H_
