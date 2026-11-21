// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "wayland/protocols/lingmo_shell.h"

#include <lingmo-shell-v1-server-protocol.h>

#include <algorithm>
#include <memory>
#include <optional>

#include "base/strings.h"
#include "compositor/surface.h"
#include "compositor/surface_observer.h"
#include "core/logging.h"
#include "display/display_manager.h"
#include "wayland/privileged_client.h"
#include "wayland/server_util.h"
#include "wm/activation.h"
#include "wm/window.h"
#include "wm/window_manager.h"

namespace Lingmo::Wayland {

namespace {

using Lingmo::WM::FocusPolicy;
using Lingmo::WM::ShellWindowMetadata;
using Lingmo::WM::WindowRole;

std::optional<WindowRole> roleFromWayland(uint32_t role)
{
    switch (role) {
    case LINGMO_SHELL_SURFACE_V1_ROLE_DESKTOP:
        return WindowRole::Desktop;
    default:
        return std::nullopt;
    }
}

// The user data of a lingmo_shell_surface_v1. It owns no window and no
// surface: it holds the client's description of one surface and pushes it into
// the window manager, which is the only thing that outlives a client restart.
class LingmoShellSurface : public compositor::SurfaceObserver
{
public:
    LingmoShellSurface(wm::WindowManager* manager, compositor::Surface* surface,
                         wl_resource* resource)
        : m_manager(manager)
        , m_surface(surface)
        , m_resource(resource)
    {
        m_surface->addSurfaceObserver(this);
        m_outputsChanged = m_manager->outputsChanged.connect([this] {
            placeOnOutput();
            sendWorkArea();
        });
        m_zonesChanged = m_manager->exclusiveZonesChanged.connect([this] { sendWorkArea(); });
    }

    ~LingmoShellSurface() override
    {
        // The resource is going: nothing is sent on it any more.
        m_outputsChanged.reset();
        m_zonesChanged.reset();
        m_resource = nullptr;
        if (m_surface) {
            m_surface->removeSurfaceObserver(this);
            // Only the stored metadata is dropped. A window that is still
            // mapped keeps the role it was given: a client may destroy and
            // recreate this object whenever it recreates the wl_surface, and
            // pushing a Normal role into the live window in between would drop
            // the desktop out of its stacking layer.
            m_manager->clearShellMetadata(m_surface);
        }
    }

    compositor::Surface* surface() const { return m_surface; }

    void setRole(WindowRole role)
    {
        m_metadata.role = role;
        placeOnOutput();
        apply();
        sendWorkArea();
    }

    void setFocusPolicy(FocusPolicy policy)
    {
        m_metadata.focusPolicy = policy;
        apply();
    }

    bool setOutput(const std::string& name)
    {
        auto* displays = m_manager->displayManager();
        if (!displays || !displays->applied())
            return false;
        const auto& logical = displays->applied()->logicalDisplays;
        if (std::none_of(logical.begin(), logical.end(), [&](const auto& output) {
                return output.waylandName == name;
            })) {
            return false;
        }
        m_metadata.outputName = name;
        placeOnOutput();
        apply();
        sendWorkArea();
        return true;
    }

    void activate()
    {
        if (!m_surface)
            return;
        wm::Window* window = m_manager->windowForSurface(m_surface);
        if (!window || window->rootSurface() != m_surface)
            return;
        m_manager->activation()->activate(window);
    }

    // compositor::SurfaceObserver
    void onSurfaceDestroying(compositor::Surface* surface) override
    {
        if (surface != m_surface)
            return;
        m_manager->clearShellMetadata(m_surface);
        m_surface = nullptr;
    }

private:
    void apply()
    {
        if (m_surface)
            m_manager->setShellMetadata(m_surface, m_metadata);
    }

    // The bounds of the desktop's output: the one named, else the primary.
    base::Rect outputBounds() const
    {
        base::Rect output;
        const auto* displays = m_manager->displayManager();
        if (displays && displays->applied()) {
            for (const auto& logical : displays->applied()->logicalDisplays) {
                if (m_metadata.outputName.empty() ? logical.primary
                                                  : logical.waylandName == m_metadata.outputName)
                    output = logical.bounds;
            }
        } else if (const auto areas = m_manager->outputAreas(); !areas.empty()) {
            output = areas.front().bounds;
        }
        return output;
    }

    // A desktop sits at its output's origin, and moves with it.
    void placeOnOutput()
    {
        if (m_metadata.role != WindowRole::Desktop)
            return;
        const base::Rect output = outputBounds();
        if (output.isEmpty() || (m_metadata.hasPosition && m_metadata.position == output.topLeft()))
            return;
        const bool placed = m_metadata.hasPosition;
        m_metadata.position = output.topLeft();
        m_metadata.hasPosition = true;
        if (placed)
            apply();
    }

    // A desktop lays out its content in its output's work area, relative to
    // that output's origin, where the desktop is placed.
    void sendWorkArea()
    {
        if (!m_resource || m_metadata.role != WindowRole::Desktop)
            return;
        const base::Rect output = outputBounds();
        base::Rect area = output.isEmpty() ? base::Rect() : m_manager->restingWorkAreaForOutput(output);
        if (area.isEmpty())
            return;
        area.translate(-output.x(), -output.y());
        if (m_sentWorkArea && *m_sentWorkArea == area)
            return;
        m_sentWorkArea = area;
        lingmo_shell_surface_v1_send_work_area(m_resource, area.x(), area.y(), area.width(),
                                                 area.height());
    }

    wm::WindowManager* m_manager = nullptr;
    compositor::Surface* m_surface = nullptr;
    wl_resource* m_resource = nullptr;
    ShellWindowMetadata m_metadata;
    std::optional<base::Rect> m_sentWorkArea;
    base::Subscription m_outputsChanged;
    base::Subscription m_zonesChanged;
};

void shellSurfaceDestroy(wl_client*, wl_resource* resource)
{
    wl_resource_destroy(resource);
}

void shellSurfaceSetRole(wl_client*, wl_resource* resource, uint32_t role)
{
    const std::optional<WindowRole> parsed = roleFromWayland(role);
    if (!parsed) {
        wl_resource_post_error(resource, LINGMO_SHELL_SURFACE_V1_ERROR_INVALID_ROLE,
                               "unknown role %u", role);
        return;
    }
    getUserDataAs<LingmoShellSurface>(resource)->setRole(*parsed);
}

void shellSurfaceSetFocusPolicy(wl_client*, wl_resource* resource, uint32_t policy)
{
    FocusPolicy parsed = FocusPolicy::Normal;
    switch (policy) {
    case LINGMO_SHELL_SURFACE_V1_FOCUS_POLICY_NORMAL:
        parsed = FocusPolicy::Normal;
        break;
    case LINGMO_SHELL_SURFACE_V1_FOCUS_POLICY_NO_FOCUS:
        parsed = FocusPolicy::NoFocus;
        break;
    default:
        wl_resource_post_error(resource, LINGMO_SHELL_SURFACE_V1_ERROR_INVALID_FOCUS_POLICY,
                               "unknown focus policy %u", policy);
        return;
    }
    getUserDataAs<LingmoShellSurface>(resource)->setFocusPolicy(parsed);
}

void shellSurfaceActivate(wl_client*, wl_resource* resource)
{
    getUserDataAs<LingmoShellSurface>(resource)->activate();
}

void shellSurfaceSetOutput(wl_client*, wl_resource* resource, const char* name)
{
    auto* surface = getUserDataAs<LingmoShellSurface>(resource);
    if (!surface->setOutput(base::strings::sanitizedUtf8(name))) {
        wl_resource_post_error(resource, LINGMO_SHELL_SURFACE_V1_ERROR_INVALID_OUTPUT,
                               "unknown logical output %s", name);
    }
}

const struct lingmo_shell_surface_v1_interface kShellSurfaceImplementation = {
    shellSurfaceDestroy,
    shellSurfaceSetRole,
    shellSurfaceSetFocusPolicy,
    shellSurfaceActivate,
    shellSurfaceSetOutput,
};

void shellDestroy(wl_client*, wl_resource* resource)
{
    wl_resource_destroy(resource);
}

void shellGetShellSurface(wl_client* client, wl_resource* resource, uint32_t id,
                          wl_resource* surfaceResource)
{
    auto* shell = getUserDataAs<WaylandLingmoShell>(resource);
    compositor::Surface* surface = surfaceFromResource(surfaceResource);
    if (!surface) {
        wl_resource_post_error(resource, LINGMO_SHELL_V1_ERROR_ROLE,
                               "the surface is already gone");
        return;
    }
    if (shell->windowManager()->shellMetadata(surface)) {
        wl_resource_post_error(resource, LINGMO_SHELL_V1_ERROR_ROLE,
                               "the surface already has a lingmo_shell_surface_v1");
        return;
    }

    wl_resource* shellSurfaceResource =
        wl_resource_create(client, &lingmo_shell_surface_v1_interface,
                           wl_resource_get_version(resource), id);
    if (!shellSurfaceResource) {
        wl_client_post_no_memory(client);
        return;
    }

    auto shellSurface =
        std::make_unique<LingmoShellSurface>(shell->windowManager(), surface, shellSurfaceResource);
    // An unconfigured shell surface is already claimed, so a second
    // get_shell_surface on the same wl_surface is caught above.
    shell->windowManager()->setShellMetadata(surface, ShellWindowMetadata {});
    setImplementation(shellSurfaceResource, &kShellSurfaceImplementation, std::move(shellSurface));
}

const struct lingmo_shell_v1_interface kShellImplementation = {
    shellDestroy,
    shellGetShellSurface,
};

void bindShell(wl_client* client, void* data, uint32_t version, uint32_t id)
{
    auto* shell = static_cast<WaylandLingmoShell*>(data);

    // Defence in depth: the global filter already hides this interface, but a
    // bind must never be served to anything but the current trusted shell.
    if (!shell->privileged()->isTrusted(client)) {
        wl_client_post_implementation_error(client, "lingmo_shell_v1 is not available");
        return;
    }

    wl_resource* resource =
        wl_resource_create(client, &lingmo_shell_v1_interface,
                           std::min(version, kLingmoShellVersion), id);
    if (!resource) {
        wl_client_post_no_memory(client);
        return;
    }
    wl_resource_set_implementation(resource, &kShellImplementation, shell, nullptr);
}

} // namespace

WaylandLingmoShell::WaylandLingmoShell(wm::WindowManager* windowManager,
                                           PrivilegedClients* privileged)
    : m_windowManager(windowManager)
    , m_privileged(privileged)
{
}

wl_global* createLingmoShellGlobal(wl_display* display, WaylandLingmoShell* shell)
{
    wl_global* global = wl_global_create(display, &lingmo_shell_v1_interface,
                                         kLingmoShellVersion, shell, bindShell);
    if (!global)
        return nullptr;
    shell->privileged()->addPrivateGlobal(global);
    return global;
}

} // namespace Lingmo::Wayland
