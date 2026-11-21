// Copyright 2018 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.chromium file.
//
// Upstream: chromium/src/components/exo/wayland/xdg_shell.cc
// Revision: 7acd40ddefff49c9d36144999aafdbb674dbd07a
//
// Adapted for Lingmo: integrates the upstream behavior with native
// compositor types and ownership.

#include "wayland/protocols/xdg_decoration.h"

#include <xdg-decoration-server-protocol.h>
#include <xdg-shell-server-protocol.h>

#include <algorithm>
#include <memory>
#include <optional>

#include "compositor/surface.h"
#include "core/logging.h"
#include "wayland/protocols/xdg_shell.h"
#include "wayland/server_util.h"
#include "wm/shell_surface.h"

namespace Lingmo::Wayland {

namespace {

using Lingmo::WM::ShellSurface;
using DecorationMode = ShellSurface::DecorationMode;

uint32_t toWayland(DecorationMode mode)
{
    return mode == DecorationMode::ServerSide ? ZXDG_TOPLEVEL_DECORATION_V1_MODE_SERVER_SIDE
                                              : ZXDG_TOPLEVEL_DECORATION_V1_MODE_CLIENT_SIDE;
}

std::optional<DecorationMode> fromWayland(uint32_t mode)
{
    switch (mode) {
    case ZXDG_TOPLEVEL_DECORATION_V1_MODE_CLIENT_SIDE:
        return DecorationMode::ClientSide;
    case ZXDG_TOPLEVEL_DECORATION_V1_MODE_SERVER_SIDE:
        return DecorationMode::ServerSide;
    default:
        return std::nullopt;
    }
}

// The user data of a zxdg_toplevel_decoration_v1. The protocol says the
// decoration must be destroyed before its xdg_toplevel; a client that gets it
// wrong is caught by the destroy listener rather than by a dangling pointer.
class WaylandXdgToplevelDecoration
{
public:
    WaylandXdgToplevelDecoration(wl_resource* resource, wl_resource* toplevelResource)
        : m_resource(resource)
        , m_toplevel(toplevelResource)
    {
        m_toplevelDestroy.owner = this;
        m_toplevelDestroy.listener.notify = &onToplevelDestroyed;
        wl_resource_add_destroy_listener(m_toplevel, &m_toplevelDestroy.listener);

        // A disconnecting client tears its objects down in whatever order
        // libwayland finds them, which is not a protocol error. The client's
        // destroy signal runs before its resources are destroyed, so this is
        // what tells the two cases apart.
        m_clientDestroy.owner = this;
        m_clientDestroy.listener.notify = &onClientDestroyed;
        wl_client_add_destroy_listener(wl_resource_get_client(resource),
                                       &m_clientDestroy.listener);

        if (ShellSurface* shellSurface = this->shellSurface())
            shellSurface->setHasDecorationObject(true);
    }

    WaylandXdgToplevelDecoration(const WaylandXdgToplevelDecoration&) = delete;
    WaylandXdgToplevelDecoration& operator=(const WaylandXdgToplevelDecoration&) = delete;

    ~WaylandXdgToplevelDecoration()
    {
        if (!m_clientGone)
            wl_list_remove(&m_clientDestroy.listener.link);
        if (m_toplevel) {
            wl_list_remove(&m_toplevelDestroy.listener.link);
            // Destroying the decoration object takes the negotiated mode with
            // it: the client decorates itself again from the next commit.
            if (ShellSurface* shellSurface = this->shellSurface())
                shellSurface->setHasDecorationObject(false);
        }
    }

    ShellSurface* shellSurface() const
    {
        return m_toplevel ? shellSurfaceForToplevel(m_toplevel) : nullptr;
    }

    // Applies `mode` - or the compositor's own preference, when the client has
    // none - and tells the client what it got.
    void requestMode(std::optional<DecorationMode> mode)
    {
        ShellSurface* shellSurface = this->shellSurface();
        if (!shellSurface)
            return;

        // The decoration configure goes first, so the xdg_surface.configure the
        // mode change produces is the one that closes the sequence, exactly as
        // the protocol describes it.
        sendConfigure(shellSurface->decorationModeFor(mode));
        shellSurface->setRequestedDecorationMode(mode);
    }

    void sendConfigure(DecorationMode mode)
    {
        if (m_configuredMode && *m_configuredMode == mode)
            return;
        m_configuredMode = mode;
        zxdg_toplevel_decoration_v1_send_configure(m_resource, toWayland(mode));
        wl_client_flush(wl_resource_get_client(m_resource));
    }

private:
    // wl_listener has no user data of its own.
    struct DestroyListener {
        wl_listener listener;
        WaylandXdgToplevelDecoration* owner;
    };

    static void onToplevelDestroyed(wl_listener* listener, void*)
    {
        auto* self = reinterpret_cast<DestroyListener*>(listener)->owner;
        wl_list_remove(&self->m_toplevelDestroy.listener.link);
        self->m_toplevel = nullptr;
        if (self->m_clientGone)
            return;
        LM_WARNING(lingmoWayland) << "xdg_toplevel destroyed before its decoration object";
        wl_resource_post_error(self->m_resource, ZXDG_TOPLEVEL_DECORATION_V1_ERROR_ORPHANED,
                               "the xdg_toplevel was destroyed before its decoration object");
    }

    static void onClientDestroyed(wl_listener* listener, void*)
    {
        auto* self = reinterpret_cast<DestroyListener*>(listener)->owner;
        wl_list_remove(&self->m_clientDestroy.listener.link);
        self->m_clientGone = true;
    }

    wl_resource* m_resource = nullptr;
    wl_resource* m_toplevel = nullptr;
    DestroyListener m_toplevelDestroy {};
    DestroyListener m_clientDestroy {};
    bool m_clientGone = false;
    std::optional<DecorationMode> m_configuredMode;
};

////////////////////////////////////////////////////////////////////////////////
// zxdg_toplevel_decoration_v1_interface:

void toplevel_decoration_destroy(wl_client*, wl_resource* resource)
{
    wl_resource_destroy(resource);
}

void toplevel_decoration_set_mode(wl_client*, wl_resource* resource, uint32_t mode)
{
    const std::optional<DecorationMode> requested = fromWayland(mode);
    if (!requested) {
        wl_resource_post_error(resource, ZXDG_TOPLEVEL_DECORATION_V1_ERROR_INVALID_MODE,
                               "invalid decoration mode");
        return;
    }
    getUserDataAs<WaylandXdgToplevelDecoration>(resource)->requestMode(requested);
}

void toplevel_decoration_unset_mode(wl_client*, wl_resource* resource)
{
    // No preference from the client leaves the compositor's, which is
    // server-side.
    getUserDataAs<WaylandXdgToplevelDecoration>(resource)->requestMode(std::nullopt);
}

const struct zxdg_toplevel_decoration_v1_interface toplevel_decoration_implementation = {
    toplevel_decoration_destroy,
    toplevel_decoration_set_mode,
    toplevel_decoration_unset_mode,
};

////////////////////////////////////////////////////////////////////////////////
// zxdg_decoration_manager_v1_interface:

void decoration_manager_destroy(wl_client*, wl_resource* resource)
{
    // Destroying the manager leaves the decorations it created alone.
    wl_resource_destroy(resource);
}

void decoration_manager_get_toplevel_decoration(wl_client* client, wl_resource* managerResource,
                                                uint32_t id, wl_resource* toplevelResource)
{
    ShellSurface* shellSurface = shellSurfaceForToplevel(toplevelResource);
    if (shellSurface) {
        if (shellSurface->hasDecorationObject()) {
            wl_resource_post_error(managerResource,
                                   ZXDG_TOPLEVEL_DECORATION_V1_ERROR_ALREADY_CONSTRUCTED,
                                   "the toplevel already has a decoration object");
            return;
        }
        // The mode has to be negotiated before the client paints: a buffer that
        // is already up was drawn without knowing who decorates it.
        if (shellSurface->rootSurface() && shellSurface->rootSurface()->hasContents()) {
            wl_resource_post_error(managerResource,
                                   ZXDG_TOPLEVEL_DECORATION_V1_ERROR_UNCONFIGURED_BUFFER,
                                   "the toplevel already has a buffer attached");
            return;
        }
    }

    wl_resource* resource = wl_resource_create(client, &zxdg_toplevel_decoration_v1_interface,
                                               wl_resource_get_version(managerResource), id);
    if (!resource) {
        wl_client_post_no_memory(client);
        return;
    }

    auto decoration = std::make_unique<WaylandXdgToplevelDecoration>(resource, toplevelResource);
    WaylandXdgToplevelDecoration* raw = decoration.get();
    setImplementation(resource, &toplevel_decoration_implementation, std::move(decoration));

    // A client may not attach a buffer before it has been told which mode it
    // is in, so the first configure goes out with the object. It is a
    // decoration configure only: the xdg_surface configure that ends the
    // sequence comes from the surface's own initial commit.
    if (shellSurface)
        raw->sendConfigure(shellSurface->decorationMode());
}

const struct zxdg_decoration_manager_v1_interface decoration_manager_implementation = {
    decoration_manager_destroy,
    decoration_manager_get_toplevel_decoration,
};

void bindXdgDecorationManager(wl_client* client, void* data, uint32_t version, uint32_t id)
{
    wl_resource* resource =
        wl_resource_create(client, &zxdg_decoration_manager_v1_interface,
                           std::min(kZxdgDecorationManagerVersion, version), id);
    if (!resource) {
        wl_client_post_no_memory(client);
        return;
    }
    wl_resource_set_implementation(resource, &decoration_manager_implementation, data, nullptr);
}

} // namespace

wl_global* createXdgDecorationGlobal(wl_display* display)
{
    return wl_global_create(display, &zxdg_decoration_manager_v1_interface,
                            kZxdgDecorationManagerVersion, nullptr, &bindXdgDecorationManager);
}

} // namespace Lingmo::Wayland
