// Copyright 2018 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.chromium file.
//
// Upstream: chromium/src/components/exo/wayland/xdg_shell.cc
// Revision: 7acd40ddefff49c9d36144999aafdbb674dbd07a
//
// Adapted for Lingmo: integrates the upstream behavior with native
// compositor types and ownership.

#include "wayland/protocols/xdg_shell.h"

#include <wayland-server-protocol.h>
#include <xdg-shell-server-protocol.h>

#include <signal.h>
#include <unistd.h>

#include <algorithm>
#include <memory>
#include <optional>
#include <utility>
#include <vector>

#include "compositor/compositor.h"
#include "compositor/surface.h"
#include "core/logging.h"
#include "base/strings.h"
#include "input/seat.h"
#include "wayland/protocols/window_hints.h"
#include "wayland/protocols/xdg_positioner.h"
#include "wayland/protocols/wl_output.h"
#include "wayland/serial_tracker.h"
#include "wayland/server_util.h"
#include "wm/shell_surface.h"
#include "wm/window.h"
#include "wm/window_manager.h"

namespace Lingmo::Wayland {

namespace {

using Lingmo::WM::ResizeEdge;
using Lingmo::WM::ShellSurface;
using Lingmo::WM::WindowStateType;

////////////////////////////////////////////////////////////////////////////////
// xdg_positioner_interface:

void xdg_positioner_destroy(wl_client*, wl_resource* resource)
{
    wl_resource_destroy(resource);
}

void xdg_positioner_set_size(wl_client*, wl_resource* resource, int32_t width, int32_t height)
{
    if (width < 1 || height < 1) {
        wl_resource_post_error(resource, XDG_POSITIONER_ERROR_INVALID_INPUT,
                               "width and height must be positive and non-zero");
        return;
    }
    getUserDataAs<WaylandPositioner>(resource)->setSize(base::Size(width, height));
}

void xdg_positioner_set_anchor_rect(wl_client*, wl_resource* resource, int32_t x, int32_t y,
                                    int32_t width, int32_t height)
{
    if (width < 1 || height < 1) {
        wl_resource_post_error(resource, XDG_POSITIONER_ERROR_INVALID_INPUT,
                               "width and height must be positive and non-zero");
        return;
    }
    getUserDataAs<WaylandPositioner>(resource)->setAnchorRect(clientRect(x, y, width, height));
}

void xdg_positioner_set_anchor(wl_client*, wl_resource* resource, uint32_t anchor)
{
    getUserDataAs<WaylandPositioner>(resource)->setAnchor(anchor);
}

void xdg_positioner_set_gravity(wl_client*, wl_resource* resource, uint32_t gravity)
{
    getUserDataAs<WaylandPositioner>(resource)->setGravity(gravity);
}

void xdg_positioner_set_constraint_adjustment(wl_client*, wl_resource* resource,
                                              uint32_t adjustment)
{
    getUserDataAs<WaylandPositioner>(resource)->setAdjustment(adjustment);
}

void xdg_positioner_set_offset(wl_client*, wl_resource* resource, int32_t x, int32_t y)
{
    getUserDataAs<WaylandPositioner>(resource)->setOffset(base::Point(x, y));
}

// Version 3. Empty, as in Weston and Exo: Lingmo recomputes a popup's
// position whenever its parent moves anyway.
void xdg_positioner_set_reactive(wl_client*, wl_resource*) { }

void xdg_positioner_set_parent_size(wl_client*, wl_resource*, int32_t, int32_t) { }

void xdg_positioner_set_parent_configure(wl_client*, wl_resource*, uint32_t) { }

const struct xdg_positioner_interface xdg_positioner_implementation = {
    xdg_positioner_destroy,
    xdg_positioner_set_size,
    xdg_positioner_set_anchor_rect,
    xdg_positioner_set_anchor,
    xdg_positioner_set_gravity,
    xdg_positioner_set_constraint_adjustment,
    xdg_positioner_set_offset,
    xdg_positioner_set_reactive,
    xdg_positioner_set_parent_size,
    xdg_positioner_set_parent_configure,
};

////////////////////////////////////////////////////////////////////////////////
// xdg_surface state:

// The user data of an xdg_surface resource. A role resource may outlive it -
// the protocol forbids it, but a client can still get there - so the back
// reference goes through a token this object clears on the way out.
struct WaylandXdgSurface;

// xdg-foreign resolves a plain wl_surface to its shell surface, which no
// resource carries a back pointer to.
std::vector<WaylandXdgSurface*>& liveXdgSurfaces()
{
    static std::vector<WaylandXdgSurface*> surfaces;
    return surfaces;
}

// The state behind one xdg_wm_base. Exo leaves xdg_wm_base.pong
// NOTIMPLEMENTED and Ash has no hung-window handling, so this is Lingmo's
// own: one ping at a time, and a timeout that marks every toplevel created
// through this xdg_wm_base unresponsive, since the whole client has stalled.
struct WaylandWmBase {
    WaylandWmBase(WaylandXdgShell* shell, wl_resource* resource)
        : shell(shell)
        , resource(resource)
        , token(std::make_shared<WaylandWmBase*>(this))
        , timer(wl_event_loop_add_timer(
              wl_display_get_event_loop(wl_client_get_display(wl_resource_get_client(resource))),
              &WaylandWmBase::onTimeout, this))
    {
    }

    ~WaylandWmBase()
    {
        *token = nullptr;
        if (timer)
            wl_event_source_remove(timer);
    }

    void ping();
    void pong(uint32_t serial);
    void setResponsive(bool responsive);

    static int onTimeout(void* data)
    {
        static_cast<WaylandWmBase*>(data)->setResponsive(false);
        return 0;
    }

    WaylandXdgShell* shell;
    wl_resource* resource;
    std::shared_ptr<WaylandWmBase*> token;
    wl_event_source* timer;
    bool pingPending = false;
    uint32_t pingSerial = 0;
};

struct WaylandXdgSurface {
    WaylandXdgSurface(WaylandWmBase* wmBase, std::unique_ptr<ShellSurface> shellSurface,
                      wl_resource* resource)
        : shell(wmBase->shell)
        , wmBase(wmBase->token)
        , shellSurface(std::move(shellSurface))
        , resource(resource)
        , token(std::make_shared<WaylandXdgSurface*>(this))
    {
        shell->addXdgSurface(resource);
        liveXdgSurfaces().push_back(this);
    }

    ~WaylandXdgSurface()
    {
        auto& surfaces = liveXdgSurfaces();
        surfaces.erase(std::remove(surfaces.begin(), surfaces.end(), this), surfaces.end());
        // Out of the shell's registry, and unreachable through the token,
        // before the shell surface tears down its window.
        shell->removeXdgSurface(resource);
        *token = nullptr;
    }

    WaylandXdgShell* shell;
    // Null once the client is being torn down and its xdg_wm_base went first.
    std::shared_ptr<WaylandWmBase*> wmBase;
    std::unique_ptr<ShellSurface> shellSurface;
    wl_resource* resource;
    std::shared_ptr<WaylandXdgSurface*> token;
    bool hasRoleObject = false;
};

void WaylandWmBase::ping()
{
    // The timeout runs from the first unanswered ping, not the latest request.
    if (pingPending || !timer)
        return;
    pingPending = true;
    pingSerial = shell->serialTracker()->nextSerial(SerialTracker::OTHER_EVENT);
    xdg_wm_base_send_ping(resource, pingSerial);
    wl_client_flush(wl_resource_get_client(resource));
    wl_event_source_timer_update(timer, shell->pingTimeout());
}

void WaylandWmBase::pong(uint32_t serial)
{
    // A late answer still counts: it is what brings a stalled client back.
    if (!pingPending || serial != pingSerial)
        return;
    pingPending = false;
    wl_event_source_timer_update(timer, 0);
    setResponsive(true);
}

void WaylandWmBase::setResponsive(bool responsive)
{
    std::vector<wm::Window*> windows;
    for (WaylandXdgSurface* surfaceData : liveXdgSurfaces()) {
        ShellSurface* shellSurface = surfaceData->shellSurface.get();
        if (*surfaceData->wmBase == this && shellSurface->role() == ShellSurface::Role::Toplevel
            && shellSurface->window())
            windows.push_back(shellSurface->window());
    }
    wm::WindowManager* manager = shell->windowManager();
    for (wm::Window* window : windows) {
        if (manager->hasWindow(window))
            window->setResponsive(responsive);
    }
}

// Sends the xdg_surface.configure that closes any role configure sequence, and
// returns its serial. Upstream: HandleXdgSurfaceConfigureCallback.
uint32_t sendSurfaceConfigure(wl_resource* xdgSurfaceResource, SerialTracker* serialTracker)
{
    const uint32_t serial = serialTracker->nextSerial(SerialTracker::OTHER_EVENT);
    xdg_surface_send_configure(xdgSurfaceResource, serial);
    wl_client_flush(wl_resource_get_client(xdgSurfaceResource));
    return serial;
}

////////////////////////////////////////////////////////////////////////////////
// xdg_toplevel_interface:

ResizeEdge resizeEdgeFromXdg(uint32_t edges)
{
    switch (edges) {
    case XDG_TOPLEVEL_RESIZE_EDGE_TOP:
        return ResizeEdge::Top;
    case XDG_TOPLEVEL_RESIZE_EDGE_BOTTOM:
        return ResizeEdge::Bottom;
    case XDG_TOPLEVEL_RESIZE_EDGE_LEFT:
        return ResizeEdge::Left;
    case XDG_TOPLEVEL_RESIZE_EDGE_TOP_LEFT:
        return ResizeEdge::TopLeft;
    case XDG_TOPLEVEL_RESIZE_EDGE_BOTTOM_LEFT:
        return ResizeEdge::BottomLeft;
    case XDG_TOPLEVEL_RESIZE_EDGE_RIGHT:
        return ResizeEdge::Right;
    case XDG_TOPLEVEL_RESIZE_EDGE_TOP_RIGHT:
        return ResizeEdge::TopRight;
    case XDG_TOPLEVEL_RESIZE_EDGE_BOTTOM_RIGHT:
        return ResizeEdge::BottomRight;
    default:
        return ResizeEdge::None;
    }
}

enum class DragSerial {
    Invalid,
    Pointer,
    Touch,
};

// An interactive move or resize is only allowed from a serial the compositor
// really handed this client for a pointer press or a touch-down, and only while
// that press is still down. Without both checks any client could grab any
// window. A touch drag checks the second half when it looks for its point.
DragSerial dragSerial(WaylandXdgShell* shell, uint32_t serial)
{
    const std::optional<SerialTracker::EventType> type =
        shell->serialTracker()->eventType(serial);
    if (!type)
        return DragSerial::Invalid;

    input::Seat* seat = shell->seat();
    switch (*type) {
    case SerialTracker::POINTER_LEFT_BUTTON_DOWN:
    case SerialTracker::POINTER_MIDDLE_BUTTON_DOWN:
    case SerialTracker::POINTER_RIGHT_BUTTON_DOWN:
    case SerialTracker::POINTER_FORWARD_BUTTON_DOWN:
    case SerialTracker::POINTER_BACK_BUTTON_DOWN:
        return seat && seat->pressedButtons() != input::NoButton ? DragSerial::Pointer
                                                                 : DragSerial::Invalid;
    case SerialTracker::TOUCH_DOWN:
        return seat ? DragSerial::Touch : DragSerial::Invalid;
    default:
        return DragSerial::Invalid;
    }
}

void addToplevelState(wl_array* states, xdg_toplevel_state state)
{
    auto* value = static_cast<xdg_toplevel_state*>(wl_array_add(states, sizeof(xdg_toplevel_state)));
    if (value)
        *value = state;
}

// Wraps the shell surface so the case where the xdg_surface resource is
// destroyed before the toplevel resource stays survivable.
void sendWmCapabilities(wl_resource* toplevel, bool maximizable)
{
    wl_array capabilities;
    wl_array_init(&capabilities);
    const auto add = [&capabilities](xdg_toplevel_wm_capabilities capability) {
        *static_cast<uint32_t*>(wl_array_add(&capabilities, sizeof(uint32_t))) = capability;
    };
    if (maximizable)
        add(XDG_TOPLEVEL_WM_CAPABILITIES_MAXIMIZE);
    add(XDG_TOPLEVEL_WM_CAPABILITIES_FULLSCREEN);
    add(XDG_TOPLEVEL_WM_CAPABILITIES_MINIMIZE);
    xdg_toplevel_send_wm_capabilities(toplevel, &capabilities);
    wl_array_release(&capabilities);
}

class WaylandToplevel
{
public:
    WaylandToplevel(wl_resource* resource, wl_resource* xdgSurfaceResource)
        : m_resource(resource)
        , m_xdgSurfaceResource(xdgSurfaceResource)
        , m_token(getUserDataAs<WaylandXdgSurface>(xdgSurfaceResource)->token)
    {
        WaylandXdgSurface* surfaceData = *m_token;
        SerialTracker* serialTracker = surfaceData->shell->serialTracker();
        wl_resource* xdgSurface = m_xdgSurfaceResource;
        wl_resource* toplevel = m_resource;

        // What wm_capabilities last told the client; -1 before the first one.
        auto sentCapabilities = std::make_shared<int>(-1);
        auto sentResizing = std::make_shared<bool>(false);
        surfaceData->shellSurface->setConfigureCallback(
            [toplevel, xdgSurface, serialTracker, sentCapabilities, sentResizing,
             token = m_token](const ShellSurface::ConfigureState& state) {
                const int version = wl_resource_get_version(toplevel);
                if (version >= XDG_TOPLEVEL_WM_CAPABILITIES_SINCE_VERSION) {
                    // No window menu: show_window_menu does nothing yet.
                    const int capabilities = state.maximizable ? 1 : 0;
                    if (capabilities != *sentCapabilities) {
                        *sentCapabilities = capabilities;
                        sendWmCapabilities(toplevel, state.maximizable);
                    }
                }
                // Qt drops the xdg resizing state; window hints carry it too.
                if (state.resizing != *sentResizing) {
                    *sentResizing = state.resizing;
                    WaylandXdgSurface* surfaceData = *token;
                    if (wl_resource* surface = surfaceData
                                ? getSurfaceResource(surfaceData->shellSurface->rootSurface())
                                : nullptr) {
                        sendInteractiveResize(surface, state.resizing);
                    }
                }
                if (version >= XDG_TOPLEVEL_CONFIGURE_BOUNDS_SINCE_VERSION) {
                    xdg_toplevel_send_configure_bounds(toplevel, state.bounds.width(),
                                                       state.bounds.height());
                }

                wl_array states;
                wl_array_init(&states);
                if (state.stateType == WindowStateType::Maximized)
                    addToplevelState(&states, XDG_TOPLEVEL_STATE_MAXIMIZED);
                if (state.stateType == WindowStateType::Fullscreen) {
                    addToplevelState(&states, XDG_TOPLEVEL_STATE_FULLSCREEN);
                    // A window that was maximized before it went fullscreen
                    // keeps that state as far as the client is concerned. X11
                    // applications, and so most toolkits, rely on it.
                    if (state.restoreStateType == WindowStateType::Maximized)
                        addToplevelState(&states, XDG_TOPLEVEL_STATE_MAXIMIZED);
                }
                if (isSnappedStateType(state.stateType)
                    && wl_resource_get_version(toplevel)
                            >= XDG_TOPLEVEL_STATE_TILED_LEFT_SINCE_VERSION) {
                    // Half the work area: flush against three of its edges,
                    // free along the one the other half is on.
                    addToplevelState(&states, XDG_TOPLEVEL_STATE_TILED_TOP);
                    addToplevelState(&states, XDG_TOPLEVEL_STATE_TILED_BOTTOM);
                    addToplevelState(&states,
                                     state.stateType == WindowStateType::PrimarySnapped
                                             ? XDG_TOPLEVEL_STATE_TILED_LEFT
                                             : XDG_TOPLEVEL_STATE_TILED_RIGHT);
                }
                if (state.resizing)
                    addToplevelState(&states, XDG_TOPLEVEL_STATE_RESIZING);
                if (state.activated)
                    addToplevelState(&states, XDG_TOPLEVEL_STATE_ACTIVATED);
                if (state.suspended && version >= XDG_TOPLEVEL_STATE_SUSPENDED_SINCE_VERSION)
                    addToplevelState(&states, XDG_TOPLEVEL_STATE_SUSPENDED);
                xdg_toplevel_send_configure(toplevel, state.size.width(), state.size.height(),
                                            &states);
                wl_array_release(&states);
                return sendSurfaceConfigure(xdgSurface, serialTracker);
            });

        surfaceData->shellSurface->setCloseCallback([toplevel] {
            xdg_toplevel_send_close(toplevel);
            wl_client_flush(wl_resource_get_client(toplevel));
        });

        surfaceData->shellSurface->setPingCallback([token = m_token] {
            if (WaylandXdgSurface* surfaceData = *token) {
                if (WaylandWmBase* wmBase = *surfaceData->wmBase)
                    wmBase->ping();
            }
        });

        surfaceData->shellSurface->setKillCallback([toplevel] {
            pid_t pid = 0;
            wl_client_get_credentials(wl_resource_get_client(toplevel), &pid, nullptr, nullptr);
            // An in-process client, as in the tests, has the compositor's own pid.
            if (pid <= 0 || pid == getpid())
                return;
            LM_INFO(lingmoWayland) << "Killing unresponsive client" << pid;
            kill(pid, SIGKILL);
        });
    }

    WaylandToplevel(const WaylandToplevel&) = delete;
    WaylandToplevel& operator=(const WaylandToplevel&) = delete;

    ~WaylandToplevel()
    {
        // The shell surface may outlive this role object. Drop the callbacks
        // before the role goes, so unmapping cannot send to a resource that is
        // already being destroyed.
        if (ShellSurface* shellSurface = this->shellSurface()) {
            shellSurface->setConfigureCallback({});
            shellSurface->setCloseCallback({});
            shellSurface->setPingCallback({});
            shellSurface->setKillCallback({});
            shellSurface->clearRole();
        }
        if (WaylandXdgSurface* surfaceData = *m_token)
            surfaceData->hasRoleObject = false;
    }

    ShellSurface* shellSurface() const
    {
        WaylandXdgSurface* surfaceData = *m_token;
        return surfaceData ? surfaceData->shellSurface.get() : nullptr;
    }

    WaylandXdgShell* shell() const
    {
        WaylandXdgSurface* surfaceData = *m_token;
        return surfaceData ? surfaceData->shell : nullptr;
    }

private:
    wl_resource* m_resource;
    wl_resource* m_xdgSurfaceResource;
    std::shared_ptr<WaylandXdgSurface*> m_token;
};

ShellSurface* toplevelShellSurface(wl_resource* resource)
{
    return getUserDataAs<WaylandToplevel>(resource)->shellSurface();
}

void xdg_toplevel_destroy(wl_client*, wl_resource* resource)
{
    wl_resource_destroy(resource);
}

void xdg_toplevel_set_parent(wl_client*, wl_resource* resource, wl_resource* parent)
{
    ShellSurface* shellSurface = toplevelShellSurface(resource);
    if (!shellSurface)
        return;

    ShellSurface* parentShellSurface =
        parent ? getUserDataAs<WaylandToplevel>(parent)->shellSurface() : nullptr;
    if (!shellSurface->setTransientParent(parentShellSurface)) {
        wl_resource_post_error(resource, XDG_TOPLEVEL_ERROR_INVALID_PARENT,
                               "the parent is the surface itself or one of its descendants");
    }
}

void xdg_toplevel_set_title(wl_client*, wl_resource* resource, const char* title)
{
    if (ShellSurface* shellSurface = toplevelShellSurface(resource))
        shellSurface->setTitle(base::strings::sanitizedUtf8(title));
}

void xdg_toplevel_set_app_id(wl_client*, wl_resource* resource, const char* appId)
{
    if (ShellSurface* shellSurface = toplevelShellSurface(resource))
        shellSurface->setApplicationId(base::strings::sanitizedUtf8(appId));
}

// Lingmo has no window menu yet. It belongs to the decoration, but nothing
// in v0.1 draws menus; a client that asks for one simply gets nothing.
void xdg_toplevel_show_window_menu(wl_client*, wl_resource*, wl_resource*, uint32_t, int32_t,
                                   int32_t)
{
}

void xdg_toplevel_move(wl_client*, wl_resource* resource, wl_resource*, uint32_t serial)
{
    auto* toplevel = getUserDataAs<WaylandToplevel>(resource);
    ShellSurface* shellSurface = toplevel->shellSurface();
    const DragSerial type = dragSerial(toplevel->shell(), serial);
    if (!shellSurface || type == DragSerial::Invalid)
        return;
    shellSurface->startMove(type == DragSerial::Touch);
}

void xdg_toplevel_resize(wl_client*, wl_resource* resource, wl_resource*, uint32_t serial,
                         uint32_t edges)
{
    const ResizeEdge edge = resizeEdgeFromXdg(edges);
    if (edge == ResizeEdge::None) {
        wl_resource_post_error(resource, XDG_TOPLEVEL_ERROR_INVALID_RESIZE_EDGE,
                               "invalid resize edge");
        return;
    }

    auto* toplevel = getUserDataAs<WaylandToplevel>(resource);
    ShellSurface* shellSurface = toplevel->shellSurface();
    const DragSerial type = dragSerial(toplevel->shell(), serial);
    if (!shellSurface || type == DragSerial::Invalid)
        return;
    shellSurface->startResize(edge, type == DragSerial::Touch);
}

void xdg_toplevel_set_max_size(wl_client*, wl_resource* resource, int32_t width, int32_t height)
{
    if (width < 0 || height < 0) {
        wl_resource_post_error(resource, XDG_TOPLEVEL_ERROR_INVALID_SIZE,
                               "width and height must not be negative");
        return;
    }
    if (ShellSurface* shellSurface = toplevelShellSurface(resource))
        shellSurface->setMaximumSize(base::Size(width, height));
}

void xdg_toplevel_set_min_size(wl_client*, wl_resource* resource, int32_t width, int32_t height)
{
    if (width < 0 || height < 0) {
        wl_resource_post_error(resource, XDG_TOPLEVEL_ERROR_INVALID_SIZE,
                               "width and height must not be negative");
        return;
    }
    if (ShellSurface* shellSurface = toplevelShellSurface(resource))
        shellSurface->setMinimumSize(base::Size(width, height));
}

void xdg_toplevel_set_maximized(wl_client*, wl_resource* resource)
{
    if (ShellSurface* shellSurface = toplevelShellSurface(resource))
        shellSurface->maximize();
}

void xdg_toplevel_unset_maximized(wl_client*, wl_resource* resource)
{
    if (ShellSurface* shellSurface = toplevelShellSurface(resource))
        shellSurface->unmaximize();
}

void xdg_toplevel_set_fullscreen(wl_client*, wl_resource* resource, wl_resource* output)
{
    if (ShellSurface* shellSurface = toplevelShellSurface(resource)) {
        shellSurface->setFullscreenOutput(outputNameFromResource(output).value_or(std::string()));
        shellSurface->setFullscreen(true);
    }
}

void xdg_toplevel_unset_fullscreen(wl_client*, wl_resource* resource)
{
    if (ShellSurface* shellSurface = toplevelShellSurface(resource))
        shellSurface->setFullscreen(false);
}

void xdg_toplevel_set_minimized(wl_client*, wl_resource* resource)
{
    if (ShellSurface* shellSurface = toplevelShellSurface(resource))
        shellSurface->minimize();
}

const struct xdg_toplevel_interface xdg_toplevel_implementation = {
    xdg_toplevel_destroy,          xdg_toplevel_set_parent,
    xdg_toplevel_set_title,        xdg_toplevel_set_app_id,
    xdg_toplevel_show_window_menu, xdg_toplevel_move,
    xdg_toplevel_resize,           xdg_toplevel_set_max_size,
    xdg_toplevel_set_min_size,     xdg_toplevel_set_maximized,
    xdg_toplevel_unset_maximized,  xdg_toplevel_set_fullscreen,
    xdg_toplevel_unset_fullscreen, xdg_toplevel_set_minimized,
};

////////////////////////////////////////////////////////////////////////////////
// xdg_popup_interface:

class WaylandPopup
{
public:
    WaylandPopup(wl_resource* resource, wl_resource* xdgSurfaceResource)
        : m_resource(resource)
        , m_token(getUserDataAs<WaylandXdgSurface>(xdgSurfaceResource)->token)
    {
        WaylandXdgSurface* surfaceData = *m_token;
        SerialTracker* serialTracker = surfaceData->shell->serialTracker();
        wl_resource* xdgSurface = xdgSurfaceResource;
        wl_resource* popup = m_resource;

        surfaceData->shellSurface->setPopupConfigureCallback(
            [popup, xdgSurface, serialTracker](const base::Rect& bounds) {
                xdg_popup_send_configure(popup, bounds.x(), bounds.y(), bounds.width(),
                                         bounds.height());
                return sendSurfaceConfigure(xdgSurface, serialTracker);
            });

        surfaceData->shellSurface->setCloseCallback([popup] {
            xdg_popup_send_popup_done(popup);
            wl_client_flush(wl_resource_get_client(popup));
        });
    }

    WaylandPopup(const WaylandPopup&) = delete;
    WaylandPopup& operator=(const WaylandPopup&) = delete;

    ~WaylandPopup()
    {
        if (ShellSurface* shellSurface = this->shellSurface()) {
            shellSurface->setPopupConfigureCallback({});
            shellSurface->setCloseCallback({});
            shellSurface->clearRole();
        }
        if (WaylandXdgSurface* surfaceData = *m_token)
            surfaceData->hasRoleObject = false;
    }

    ShellSurface* shellSurface() const
    {
        WaylandXdgSurface* surfaceData = *m_token;
        return surfaceData ? surfaceData->shellSurface.get() : nullptr;
    }

    WaylandXdgShell* shell() const
    {
        WaylandXdgSurface* surfaceData = *m_token;
        return surfaceData ? surfaceData->shell : nullptr;
    }

    // A popup made with a null parent keeps its positioner until another
    // protocol gives it one.
    std::optional<WaylandPositioner> waitingPositioner;
    // Where the coordinates its positioners use lie in its parent's.
    base::Point anchorOffset;
    // The client of a grab asked for while it waited for a parent.
    wl_client* pendingGrab = nullptr;

private:
    wl_resource* m_resource;
    std::shared_ptr<WaylandXdgSurface*> m_token;
};

// The work area a popup is positioned inside, in the coordinates of `parent`'s
// window geometry, which is what xdg_positioner uses.
base::Rect popupWorkArea(WaylandXdgShell* shell, ShellSurface* parent)
{
    // Exo constrains popups to the work area of the display nearest the parent,
    // including shell-owned menus.
    wm::WindowManager* manager = shell->windowManager();
    const wm::Window* owner = parent->ownerWindow();
    base::Rect workArea =
        owner ? manager->workAreaForOutput(manager->outputBoundsFor(owner)) : base::Rect();
    if (workArea.isEmpty())
        workArea = manager->workAreaFor(nullptr);
    workArea.translate(-parent->absoluteOrigin());
    return workArea;
}

void xdg_popup_destroy(wl_client*, wl_resource* resource)
{
    wl_resource_destroy(resource);
}

void xdg_popup_grab(wl_client* client, wl_resource* resource, wl_resource*, uint32_t)
{
    auto* popup = getUserDataAs<WaylandPopup>(resource);
    ShellSurface* shellSurface = popup->shellSurface();
    if (!shellSurface) {
        wl_resource_post_error(resource, XDG_POPUP_ERROR_INVALID_GRAB,
                               "the surface has already been destroyed");
        return;
    }
    // Taken once the popup has its parent.
    if (popup->waitingPositioner) {
        popup->pendingGrab = client;
        return;
    }
    shellSurface->grab(client);
}

void xdg_popup_reposition(wl_client*, wl_resource* resource, wl_resource* positionerResource,
                          uint32_t token)
{
    if (wl_resource_get_version(resource) < XDG_POPUP_REPOSITIONED_SINCE_VERSION)
        return;

    auto* popup = getUserDataAs<WaylandPopup>(resource);
    ShellSurface* shellSurface = popup->shellSurface();
    if (!shellSurface || !shellSurface->popupParent())
        return;

    xdg_popup_send_repositioned(resource, token);

    auto* positioner = getUserDataAs<WaylandPositioner>(positionerResource);
    const WaylandPositioner::Result position = positioner->calculateBounds(
            popupWorkArea(popup->shell(), shellSurface->popupParent()), popup->anchorOffset);
    shellSurface->reposition(position.toRect());
}

const struct xdg_popup_interface xdg_popup_implementation = {
    xdg_popup_destroy,
    xdg_popup_grab,
    xdg_popup_reposition,
};

////////////////////////////////////////////////////////////////////////////////
// xdg_surface_interface:

void xdg_surface_destroy(wl_client*, wl_resource* resource)
{
    if (getUserDataAs<WaylandXdgSurface>(resource)->hasRoleObject) {
        wl_resource_post_error(resource, XDG_SURFACE_ERROR_DEFUNCT_ROLE_OBJECT,
                               "the role object must be destroyed before the xdg_surface");
        return;
    }
    wl_resource_destroy(resource);
}

void xdg_surface_get_toplevel(wl_client* client, wl_resource* xdgSurfaceResource, uint32_t id)
{
    auto* surfaceData = getUserDataAs<WaylandXdgSurface>(xdgSurfaceResource);
    if (surfaceData->shellSurface->hasRole()) {
        wl_resource_post_error(xdgSurfaceResource, XDG_SURFACE_ERROR_ALREADY_CONSTRUCTED,
                               "surface has already been constructed");
        return;
    }

    surfaceData->shellSurface->setToplevel();

    wl_resource* toplevelResource = wl_resource_create(
        client, &xdg_toplevel_interface, wl_resource_get_version(xdgSurfaceResource), id);
    if (!toplevelResource) {
        wl_client_post_no_memory(client);
        return;
    }
    surfaceData->hasRoleObject = true;
    setImplementation(toplevelResource, &xdg_toplevel_implementation,
                      std::make_unique<WaylandToplevel>(toplevelResource, xdgSurfaceResource));
}

void xdg_surface_get_popup(wl_client* client, wl_resource* resource, uint32_t id,
                           wl_resource* parentResource, wl_resource* positionerResource)
{
    auto* surfaceData = getUserDataAs<WaylandXdgSurface>(resource);
    if (surfaceData->shellSurface->hasRole()) {
        wl_resource_post_error(resource, XDG_SURFACE_ERROR_ALREADY_CONSTRUCTED,
                               "surface has already been constructed");
        return;
    }
    if (!parentResource) {
        // Another protocol names the parent before the first commit, as
        // lingmo_status_item_v1.get_popup does.
        auto* positioner = getUserDataAs<WaylandPositioner>(positionerResource);
        if (positioner->size().isEmpty()) {
            wl_resource_post_error(resource, XDG_WM_BASE_ERROR_INVALID_POSITIONER,
                                   "the positioner is incomplete");
            return;
        }
        wl_resource* popupResource =
            wl_resource_create(client, &xdg_popup_interface, wl_resource_get_version(resource), id);
        if (!popupResource) {
            wl_client_post_no_memory(client);
            return;
        }
        surfaceData->hasRoleObject = true;
        auto popup = std::make_unique<WaylandPopup>(popupResource, resource);
        popup->waitingPositioner = *positioner;
        setImplementation(popupResource, &xdg_popup_implementation, std::move(popup));
        return;
    }

    auto* parentData = getUserDataAs<WaylandXdgSurface>(parentResource);
    if (!parentData->shellSurface->hasRole()) {
        wl_resource_post_error(resource, XDG_SURFACE_ERROR_NOT_CONSTRUCTED,
                               "popup parent not constructed");
        return;
    }

    auto* positioner = getUserDataAs<WaylandPositioner>(positionerResource);
    if (positioner->size().isEmpty()) {
        wl_resource_post_error(resource, XDG_WM_BASE_ERROR_INVALID_POSITIONER,
                               "the positioner is incomplete");
        return;
    }

    const WaylandPositioner::Result position = positioner->calculateBounds(
        popupWorkArea(surfaceData->shell, parentData->shellSurface.get()));

    if (!surfaceData->shellSurface->setPopup(parentData->shellSurface.get(),
                                             position.toRect())) {
        wl_resource_post_error(resource, XDG_WM_BASE_ERROR_INVALID_POPUP_PARENT,
                               "the popup parent is the popup itself or one of its descendants");
        return;
    }

    wl_resource* popupResource =
        wl_resource_create(client, &xdg_popup_interface, wl_resource_get_version(resource), id);
    if (!popupResource) {
        wl_client_post_no_memory(client);
        return;
    }
    surfaceData->hasRoleObject = true;
    setImplementation(popupResource, &xdg_popup_implementation,
                      std::make_unique<WaylandPopup>(popupResource, resource));

    // The initial configure carries coordinates relative to the parent window,
    // which is why it is sent from here rather than from the shell surface.
    surfaceData->shellSurface->configurePopup();
}

void xdg_surface_set_window_geometry(wl_client*, wl_resource* resource, int32_t x, int32_t y,
                                     int32_t width, int32_t height)
{
    if (width < 0 || height < 0) {
        wl_resource_post_error(resource, XDG_SURFACE_ERROR_INVALID_SIZE,
                               "window geometry must not have a negative size");
        return;
    }
    getUserDataAs<WaylandXdgSurface>(resource)->shellSurface->setGeometry(
        clientRect(x, y, width, height));
}

void xdg_surface_ack_configure(wl_client*, wl_resource* resource, uint32_t serial)
{
    if (!getUserDataAs<WaylandXdgSurface>(resource)->shellSurface->acknowledgeConfigure(serial)) {
        wl_resource_post_error(resource, XDG_SURFACE_ERROR_INVALID_SERIAL,
                               "the serial was never sent, or has already been superseded");
    }
}

const struct xdg_surface_interface xdg_surface_implementation = {
    xdg_surface_destroy,          xdg_surface_get_toplevel,   xdg_surface_get_popup,
    xdg_surface_set_window_geometry, xdg_surface_ack_configure,
};

////////////////////////////////////////////////////////////////////////////////
// xdg_wm_base_interface:

void xdg_wm_base_destroy(wl_client* client, wl_resource* resource)
{
    if (getUserDataAs<WaylandWmBase>(resource)->shell->hasXdgSurfaceForClient(client)) {
        wl_resource_post_error(resource, XDG_WM_BASE_ERROR_DEFUNCT_SURFACES,
                               "xdg_wm_base was destroyed before its surfaces");
        return;
    }
    wl_resource_destroy(resource);
}

void xdg_wm_base_create_positioner(wl_client* client, wl_resource* resource, uint32_t id)
{
    wl_resource* positionerResource = wl_resource_create(
        client, &xdg_positioner_interface, wl_resource_get_version(resource), id);
    if (!positionerResource) {
        wl_client_post_no_memory(client);
        return;
    }
    setImplementation(positionerResource, &xdg_positioner_implementation,
                      std::make_unique<WaylandPositioner>());
}

void xdg_wm_base_get_xdg_surface(wl_client* client, wl_resource* resource, uint32_t id,
                                 wl_resource* surfaceResource)
{
    auto* wmBase = getUserDataAs<WaylandWmBase>(resource);
    WaylandXdgShell* shell = wmBase->shell;
    auto* surface = getUserDataAs<compositor::Surface>(surfaceResource);

    if (surface->hasRole()) {
        wl_resource_post_error(resource, XDG_WM_BASE_ERROR_ROLE,
                               "surface has already been assigned a role");
        return;
    }
    // Like exo, a buffer already on the surface is not an error. The shell's
    // menu bar popups have hit it under load, and the error kills the shell.

    wl_resource* xdgSurfaceResource =
        wl_resource_create(client, &xdg_surface_interface, wl_resource_get_version(resource), id);
    if (!xdgSurfaceResource) {
        wl_client_post_no_memory(client);
        return;
    }

    auto shellSurface = std::make_unique<ShellSurface>(shell->windowManager(), surface);
    // The one place the owning process can be asked for. It is metadata for the
    // task list, never a privilege check.
    pid_t pid = 0;
    wl_client_get_credentials(client, &pid, nullptr, nullptr);
    shellSurface->setProcessId(pid);

    // The role itself is only taken by get_toplevel or get_popup; until then
    // the surface is an xdg_surface with nothing to show.
    setImplementation(
        xdgSurfaceResource, &xdg_surface_implementation,
        std::make_unique<WaylandXdgSurface>(wmBase, std::move(shellSurface), xdgSurfaceResource));
}

void xdg_wm_base_pong(wl_client*, wl_resource* resource, uint32_t serial)
{
    getUserDataAs<WaylandWmBase>(resource)->pong(serial);
}

const struct xdg_wm_base_interface xdg_wm_base_implementation = {
    xdg_wm_base_destroy,
    xdg_wm_base_create_positioner,
    xdg_wm_base_get_xdg_surface,
    xdg_wm_base_pong,
};

void bindXdgShell(wl_client* client, void* data, uint32_t version, uint32_t id)
{
    wl_resource* resource = wl_resource_create(client, &xdg_wm_base_interface,
                                               std::min(kXdgWmBaseVersion, version), id);
    if (!resource) {
        wl_client_post_no_memory(client);
        return;
    }
    auto* shell = static_cast<WaylandXdgShell*>(data);
    setImplementation(resource, &xdg_wm_base_implementation,
                      std::make_unique<WaylandWmBase>(shell, resource));
}

} // namespace

////////////////////////////////////////////////////////////////////////////////
// WaylandXdgShell:

WaylandXdgShell::WaylandXdgShell(compositor::Compositor* compositor,
                                 wm::WindowManager* windowManager, input::Seat* seat,
                                 SerialTracker* serialTracker)
    : m_windowManager(windowManager)
    , m_seat(seat)
    , m_serialTracker(serialTracker)
{
}

WaylandXdgShell::~WaylandXdgShell() = default;

void WaylandXdgShell::addXdgSurface(wl_resource* resource)
{
    m_xdgSurfaces.push_back(resource);
}

void WaylandXdgShell::removeXdgSurface(wl_resource* resource)
{
    m_xdgSurfaces.erase(std::remove(m_xdgSurfaces.begin(), m_xdgSurfaces.end(), resource),
                        m_xdgSurfaces.end());
}

bool WaylandXdgShell::hasXdgSurfaceForClient(wl_client* client) const
{
    return std::any_of(m_xdgSurfaces.begin(), m_xdgSurfaces.end(), [client](wl_resource* resource) {
        return wl_resource_get_client(resource) == client;
    });
}

wm::ShellSurface* toplevelShellSurfaceForSurface(wl_resource* surfaceResource)
{
    compositor::Surface* surface = surfaceFromResource(surfaceResource);
    if (!surface)
        return nullptr;
    for (WaylandXdgSurface* surfaceData : liveXdgSurfaces()) {
        ShellSurface* shellSurface = surfaceData->shellSurface.get();
        if (shellSurface->rootSurface() != surface)
            continue;
        return shellSurface->role() == ShellSurface::Role::Toplevel ? shellSurface : nullptr;
    }
    return nullptr;
}

wm::ShellSurface* shellSurfaceForToplevel(wl_resource* toplevelResource)
{
    if (!toplevelResource)
        return nullptr;
    return getUserDataAs<WaylandToplevel>(toplevelResource)->shellSurface();
}

ParentlessPopupResult adoptParentlessPopup(wl_resource* popupResource, wm::ShellSurface* parent,
                                           const base::Point& anchorOffset,
                                           const std::function<bool(wm::ShellSurface*, const base::Rect&)>& adopt)
{
    auto* popup = getUserDataAs<WaylandPopup>(popupResource);
    ShellSurface* shellSurface = popup->shellSurface();
    if (!shellSurface || !popup->waitingPositioner || shellSurface->hasRole())
        return ParentlessPopupResult::HasParent;
    const WaylandPositioner positioner = *popup->waitingPositioner;
    popup->waitingPositioner.reset();
    const base::Rect bounds = parent
            ? positioner.calculateBounds(popupWorkArea(popup->shell(), parent), anchorOffset).toRect()
            : base::Rect();
    if (!parent || !adopt(shellSurface, bounds)) {
        shellSurface->close();
        return ParentlessPopupResult::Dismissed;
    }
    popup->anchorOffset = anchorOffset;
    if (wl_client* client = std::exchange(popup->pendingGrab, nullptr))
        shellSurface->grab(client);
    shellSurface->configurePopup();
    return ParentlessPopupResult::Adopted;
}

wl_global* createXdgShellGlobal(wl_display* display, WaylandXdgShell* shell)
{
    return wl_global_create(display, &xdg_wm_base_interface, kXdgWmBaseVersion, shell,
                            &bindXdgShell);
}

} // namespace Lingmo::Wayland
