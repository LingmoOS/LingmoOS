// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include <chrono>
#include <map>
#include <string>

#include "compositor/surface_observer.h"

struct wl_client;
struct wl_display;
struct wl_global;
struct wl_resource;

namespace Lingmo::WM {
class WindowManager;
}

namespace Lingmo::Wayland {

struct WaylandSeat;

// Keep alive until all display clients have been destroyed.
class WaylandXdgActivation final : public compositor::SurfaceObserver
{
public:
    WaylandXdgActivation(wm::WindowManager *manager, WaylandSeat *seat);
    ~WaylandXdgActivation() override;
    WaylandXdgActivation(const WaylandXdgActivation &) = delete;
    WaylandXdgActivation &operator=(const WaylandXdgActivation &) = delete;

    std::string createToken(wl_client *client, wl_resource *seat, uint32_t serial,
                            wl_resource *surface, bool surfaceWasSet);
    // A token for a program the compositor's own shell starts on the user's
    // input to it, which the shell vouches for.
    std::string createShellToken();
    void activate(const char *token, compositor::Surface *surface);
    void onSurfaceDestroying(compositor::Surface *surface) override;
    void onSurfaceCommit(compositor::Surface *surface) override;

private:
    using Clock = std::chrono::steady_clock;
    void prune();
    bool activateMapped(compositor::Surface *surface);

    wm::WindowManager *m_manager;
    WaylandSeat *m_seat;
    std::map<std::string, Clock::time_point> m_tokens;
    std::map<compositor::Surface *, Clock::time_point> m_pending;
};

wl_global *createXdgActivationGlobal(wl_display *display, WaylandXdgActivation *activation);

} // namespace Lingmo::Wayland
