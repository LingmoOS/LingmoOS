// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwayland_shell.h"

#include "compositor/surface.h"
#include "compositor/surface_observer.h"
#include "wayland/privileged_client.h"
#include "wayland/server.h"
#include "wayland/server_util.h"
#include "xwayland-shell-v1-server-protocol.h"

namespace Lingmo::XWayland {
namespace {

void destroy(wl_client*, wl_resource* resource)
{
    wl_resource_destroy(resource);
}

} // namespace

struct XwaylandShell::Role : compositor::SurfaceObserver {
    compositor::Surface* surface;
    wl_resource* resource = nullptr;
    SurfaceAssociation* association;
    uint64_t pending = 0;
    bool committed = false;

    Role(compositor::Surface* s, SurfaceAssociation* a)
        : surface(s)
        , association(a)
    {
        surface->setHasRole(true);
        surface->setHidden(true);
        surface->addSurfaceObserver(this);
    }

    ~Role() override
    {
        if (resource)
            wl_resource_destroy(resource);
        if (surface) {
            association->removeSurface(surface);
            surface->removeSurfaceObserver(this);
        }
    }

    void onSurfaceDestroying(compositor::Surface* s) override
    {
        association->removeSurface(s);
        s->removeSurfaceObserver(this);
        surface = nullptr;
    }

    std::function<void()> onSurfaceCommitRequested(compositor::Surface*) override
    {
        if (!pending)
            return {};
        const auto serial = std::exchange(pending, 0);
        if (committed) {
            if (resource)
                wl_resource_post_error(resource, XWAYLAND_SURFACE_V1_ERROR_ALREADY_ASSOCIATED,
                                       "surface association already committed");
            return {};
        }
        committed = true;
        return [this, serial] {
            if (surface)
                association->addSurface(serial, surface);
        };
    }

    static void setSerial(wl_client*, wl_resource* resource, uint32_t low, uint32_t high)
    {
        auto* role = static_cast<Role*>(wl_resource_get_user_data(resource));
        const auto serial = SurfaceAssociation::serial(low, high);
        if (!role->surface || !role->association->claimSurfaceSerial(serial)) {
            wl_resource_post_error(resource, XWAYLAND_SURFACE_V1_ERROR_INVALID_SERIAL,
                                   "serial not above the last one, or destroyed surface");
            return;
        }
        role->pending = serial;
    }
};

XwaylandShell::XwaylandShell(wl_display* display, wayland::PrivilegedClients* privileged,
                             wl_client* client, SurfaceAssociation* association)
    : m_privileged(privileged)
    , m_client(client)
    , m_association(association)
{
    m_server = wayland::Server::serverForDisplay(display);
    if (m_server)
        m_server->setClientSurfacesInitiallyHidden(client, true);
    m_global = wl_global_create(display, &xwayland_shell_v1_interface, 1, this, bind);
    m_privileged->addRestrictedGlobal(m_global, [this](const wl_client* c) {
        return c && c == m_client;
    });
}

XwaylandShell::~XwaylandShell()
{
    revokeClient();
    m_privileged->removeRestrictedGlobal(m_global);
    if (m_global)
        wl_global_destroy(m_global);
}

void XwaylandShell::revokeClient()
{
    if (m_server && m_client)
        m_server->setClientSurfacesInitiallyHidden(m_client, false);
    m_client = nullptr;
}

void XwaylandShell::bind(wl_client* client, void* data, uint32_t, uint32_t id)
{
    auto* self = static_cast<XwaylandShell*>(data);
    if (client != self->m_client) {
        wl_client_post_implementation_error(client, "unauthorized Xwayland shell bind");
        return;
    }
    auto* resource = wl_resource_create(client, &xwayland_shell_v1_interface, 1, id);
    static const struct xwayland_shell_v1_interface implementation = {destroy, getSurface};
    if (!resource) {
        wl_client_post_no_memory(client);
        return;
    }
    wl_resource_set_implementation(resource, &implementation, self, nullptr);
    self->m_bound = true;
    if (self->bound)
        self->bound();
}

void XwaylandShell::getSurface(wl_client* client, wl_resource* resource, uint32_t id,
                               wl_resource* surfaceResource)
{
    auto* self = static_cast<XwaylandShell*>(wl_resource_get_user_data(resource));
    auto* surface = wayland::surfaceFromResource(surfaceResource);
    if (!surface || surface->hasRole()) {
        wl_resource_post_error(resource, XWAYLAND_SHELL_V1_ERROR_ROLE, "surface has a role");
        return;
    }
    std::erase_if(self->m_roles, [](const auto& role) {
        return !role->surface && !role->resource;
    });
    if (self->m_roles.size() >= SurfaceAssociation::limit) {
        wl_client_post_no_memory(client);
        return;
    }
    auto role = std::make_unique<Role>(surface, self->m_association);
    role->resource = wl_resource_create(client, &xwayland_surface_v1_interface, 1, id);
    static const struct xwayland_surface_v1_interface implementation = {Role::setSerial, destroy};
    wl_resource_set_implementation(role->resource, &implementation, role.get(), [](wl_resource* r) {
        auto* role = static_cast<Role*>(wl_resource_get_user_data(r));
        role->resource = nullptr;
        role->pending = 0;
    });
    self->m_roles.push_back(std::move(role));
}

} // namespace Lingmo::XWayland
