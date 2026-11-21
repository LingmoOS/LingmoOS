// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef LINGMO_WAYLAND_PROTOCOLS_LINGMO_SHELL_H_
#define LINGMO_WAYLAND_PROTOCOLS_LINGMO_SHELL_H_

#include <wayland-server-core.h>

#include <cstdint>

namespace Lingmo::WM {
class WindowManager;
}

namespace Lingmo::Wayland {

class PrivilegedClients;

constexpr uint32_t kLingmoShellVersion = 1;

// The per-display state the lingmo_shell_v1 global hands to its resources.
//
// lingmo_shell_v1 is the private extension the trusted client (the file
// manager's desktop) uses to say that an ordinary xdg surface is an output's
// desktop. Everything it carries is adapted into wm:: concepts; no wl_resource
// crosses into the window manager.
class WaylandLingmoShell
{
public:
    WaylandLingmoShell(wm::WindowManager* windowManager, PrivilegedClients* privileged);
    WaylandLingmoShell(const WaylandLingmoShell&) = delete;
    WaylandLingmoShell& operator=(const WaylandLingmoShell&) = delete;

    wm::WindowManager* windowManager() const { return m_windowManager; }
    PrivilegedClients* privileged() const { return m_privileged; }

private:
    wm::WindowManager* m_windowManager = nullptr;
    PrivilegedClients* m_privileged = nullptr;
};

// `shell` must outlive the global. Registering the returned global with the
// privileged client registry is what hides the interface from ordinary clients.
wl_global* createLingmoShellGlobal(wl_display* display, WaylandLingmoShell* shell);

// Creates the lingmo_shell_v1 global for the display.
wl_global* createLingmoShellGlobal(wl_display* display, WaylandLingmoShell* shell);

} // namespace Lingmo::Wayland

#endif // LINGMO_WAYLAND_PROTOCOLS_LINGMO_SHELL_H_
