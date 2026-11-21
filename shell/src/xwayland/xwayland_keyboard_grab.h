// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include <wayland-server-core.h>

namespace Lingmo::Input {
class Seat;
}

namespace Lingmo::Wayland {
class PrivilegedClients;
}

namespace Lingmo::XWayland {

// zwp_xwayland_keyboard_grab_manager_v1, for Xwayland alone: an X11 client
// that grabs the keyboard - a game, a virtual machine viewer - inhibits the
// compositor's shortcuts while its window has focus, as it would on an X
// server. Exo has no counterpart.
class XwaylandKeyboardGrab
{
public:
    XwaylandKeyboardGrab(wl_display* display, wayland::PrivilegedClients* privileged,
                         wl_client* client, input::Seat* seat);
    ~XwaylandKeyboardGrab();

private:
    static void bind(wl_client*, void*, uint32_t, uint32_t);
    wl_global* m_global = nullptr;
    wayland::PrivilegedClients* m_privileged;
    wl_client* m_client;
    input::Seat* m_seat;
};

} // namespace Lingmo::XWayland
