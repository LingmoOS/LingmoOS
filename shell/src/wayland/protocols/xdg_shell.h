// Copyright 2018 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.chromium file.
//
// Upstream: chromium/src/components/exo/wayland/xdg_shell.h
// Revision: 7acd40ddefff49c9d36144999aafdbb674dbd07a
//
// Adapted for Lingmo: the ChromeOS window-state types become wm::
// WindowStateType and the HT* hit-test constants become wm::ResizeEdge. The
// zxdg_decoration half lives in xdg_decoration.cc and reaches the shell surface
// through shellSurfaceForToplevel().

#ifndef LINGMO_WAYLAND_PROTOCOLS_XDG_SHELL_H_
#define LINGMO_WAYLAND_PROTOCOLS_XDG_SHELL_H_

#include <wayland-server-core.h>

#include <cstdint>
#include <functional>
#include <vector>

#include "base/geometry.h"

namespace Lingmo::Compositor {
class Compositor;
}

namespace Lingmo::Input {
class Seat;
}

namespace Lingmo::WM {
class ShellSurface;
class WindowManager;
}

namespace Lingmo::Wayland {

class SerialTracker;

// Exo stops at version 3, which brought xdg_popup.reposition and
// xdg_positioner.set_reactive. Versions 4 to 6 add configure_bounds,
// wm_capabilities and the suspended state, all Lingmo's own.
constexpr uint32_t kXdgWmBaseVersion = 6;

// How long a client has to answer xdg_wm_base.ping before its toplevels turn
// unresponsive. Mutter's default check-alive-timeout.
constexpr int kXdgPingTimeoutMs = 5000;

// The per-display state the xdg_wm_base global hands to its resources.
class WaylandXdgShell
{
public:
    WaylandXdgShell(compositor::Compositor* compositor, wm::WindowManager* windowManager,
                    input::Seat* seat, SerialTracker* serialTracker);
    WaylandXdgShell(const WaylandXdgShell&) = delete;
    WaylandXdgShell& operator=(const WaylandXdgShell&) = delete;
    ~WaylandXdgShell();

    wm::WindowManager* windowManager() const { return m_windowManager; }
    input::Seat* seat() const { return m_seat; }
    SerialTracker* serialTracker() const { return m_serialTracker; }

    int pingTimeout() const { return m_pingTimeoutMs; }

    void setPingTimeout(int milliseconds) { m_pingTimeoutMs = milliseconds; }

    // Live xdg_surface resources, so xdg_wm_base.destroy can raise
    // defunct_surfaces.
    void addXdgSurface(wl_resource* resource);
    void removeXdgSurface(wl_resource* resource);
    bool hasXdgSurfaceForClient(wl_client* client) const;

private:
    wm::WindowManager* m_windowManager = nullptr;
    input::Seat* m_seat = nullptr;
    SerialTracker* m_serialTracker = nullptr;
    int m_pingTimeoutMs = kXdgPingTimeoutMs;
    std::vector<wl_resource*> m_xdgSurfaces;
};

wl_global* createXdgShellGlobal(wl_display* display, WaylandXdgShell* shell);

// The shell surface behind an xdg_toplevel resource, or nullptr when the
// xdg_surface it belongs to has already been destroyed. xdg-decoration needs
// it, and has no business knowing how the role object is laid out.
wm::ShellSurface* shellSurfaceForToplevel(wl_resource* toplevelResource);

// The toplevel shell surface a wl_surface belongs to, or nullptr when it has no
// xdg_toplevel role. xdg-foreign works in wl_surfaces, not role objects.
wm::ShellSurface* toplevelShellSurfaceForSurface(wl_resource* surfaceResource);

enum class ParentlessPopupResult {
    Adopted,
    // It has a parent already, or its xdg_surface is gone.
    HasParent,
    // There was nowhere to put it; popup_done went out.
    Dismissed,
};

// Gives an xdg_popup made with a null parent its parent, for the protocol that
// names it: the popup is solved against the work area of `parent`'s output
// with its positioner's anchor rect moved by `anchorOffset`, `adopt` makes it
// a popup of `parent` at those bounds, and the first configure goes out. A
// null parent, or `adopt` refusing, dismisses the popup.
ParentlessPopupResult adoptParentlessPopup(wl_resource* popupResource, wm::ShellSurface* parent,
                                           const base::Point& anchorOffset,
                                           const std::function<bool(wm::ShellSurface*, const base::Rect&)>& adopt);

} // namespace Lingmo::Wayland

#endif // LINGMO_WAYLAND_PROTOCOLS_XDG_SHELL_H_
