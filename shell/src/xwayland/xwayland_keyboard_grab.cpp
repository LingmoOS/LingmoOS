// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwayland_keyboard_grab.h"

#include <memory>

#include "compositor/surface.h"
#include "compositor/surface_observer.h"
#include "input/seat.h"
#include "input/shortcuts_inhibition.h"
#include "wayland/privileged_client.h"
#include "wayland/server_util.h"
#include "xwayland-keyboard-grab-unstable-v1-server-protocol.h"

namespace Lingmo::XWayland {
namespace {

// The protocol has no events: the grab stays inert rather than failing when
// the surface is already inhibited.
class Grab : public input::ShortcutsInhibitorDelegate, public compositor::SurfaceObserver
{
public:
    Grab(input::Seat* seat, compositor::Surface* surface)
        : m_seat(seat)
        , m_surface(surface)
    {
        m_surface->addSurfaceObserver(this);
        m_seat->shortcutsInhibition()->add(this);
    }

    ~Grab() override
    {
        m_seat->shortcutsInhibition()->remove(this);
        if (m_surface)
            m_surface->removeSurfaceObserver(this);
    }

    compositor::Surface* inhibitedSurface() const override { return m_surface; }

    void onInhibitorActivated() override { }

    void onInhibitorDeactivated() override { }

    void onSurfaceDestroying(compositor::Surface* surface) override
    {
        surface->removeSurfaceObserver(this);
        m_surface = nullptr;
    }

private:
    input::Seat* m_seat;
    compositor::Surface* m_surface;
};

void destroy(wl_client*, wl_resource* resource)
{
    wl_resource_destroy(resource);
}

const struct zwp_xwayland_keyboard_grab_v1_interface grabImplementation = {destroy};

void grabKeyboard(wl_client* client, wl_resource* resource, uint32_t id,
                  wl_resource* surfaceResource, wl_resource*)
{
    auto* grabResource = wl_resource_create(client, &zwp_xwayland_keyboard_grab_v1_interface,
                                            wl_resource_get_version(resource), id);
    if (!grabResource) {
        wl_client_post_no_memory(client);
        return;
    }
    wayland::setImplementation(
        grabResource, &grabImplementation,
        std::make_unique<Grab>(static_cast<input::Seat*>(wl_resource_get_user_data(resource)),
                               wayland::surfaceFromResource(surfaceResource)));
}

const struct zwp_xwayland_keyboard_grab_manager_v1_interface managerImplementation = {
    destroy, grabKeyboard};

} // namespace

XwaylandKeyboardGrab::XwaylandKeyboardGrab(wl_display* display,
                                           wayland::PrivilegedClients* privileged,
                                           wl_client* client, input::Seat* seat)
    : m_privileged(privileged)
    , m_client(client)
    , m_seat(seat)
{
    m_global =
        wl_global_create(display, &zwp_xwayland_keyboard_grab_manager_v1_interface, 1, this, bind);
    m_privileged->addRestrictedGlobal(m_global, [client](const wl_client* c) {
        return c && c == client;
    });
}

XwaylandKeyboardGrab::~XwaylandKeyboardGrab()
{
    m_privileged->removeRestrictedGlobal(m_global);
    if (m_global)
        wl_global_destroy(m_global);
}

void XwaylandKeyboardGrab::bind(wl_client* client, void* data, uint32_t version, uint32_t id)
{
    auto* self = static_cast<XwaylandKeyboardGrab*>(data);
    if (client != self->m_client) {
        wl_client_post_implementation_error(client, "unauthorized Xwayland keyboard grab bind");
        return;
    }
    auto* resource =
        wl_resource_create(client, &zwp_xwayland_keyboard_grab_manager_v1_interface, version, id);
    if (!resource) {
        wl_client_post_no_memory(client);
        return;
    }
    wl_resource_set_implementation(resource, &managerImplementation, self->m_seat, nullptr);
}

} // namespace Lingmo::XWayland
