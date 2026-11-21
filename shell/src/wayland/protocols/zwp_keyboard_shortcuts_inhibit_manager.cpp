// Copyright 2022 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.chromium file.
//
// Upstream: chromium/src/components/exo/wayland/zwp_keyboard_shortcuts_inhibit_manager.cc
// Revision: 7acd40ddefff49c9d36144999aafdbb674dbd07a
//
// Adapted for Lingmo: the inhibitor registers with the seat, which sends the
// active and inactive events Exo never does.

#include "wayland/protocols/zwp_keyboard_shortcuts_inhibit_manager.h"

#include <keyboard-shortcuts-inhibit-unstable-v1-server-protocol.h>

#include "compositor/surface.h"
#include "compositor/surface_observer.h"
#include "input/seat.h"
#include "input/shortcuts_inhibition.h"
#include "wayland/protocols/wl_seat.h"
#include "wayland/server_util.h"

namespace Lingmo::Wayland {

namespace {

class KeyboardShortcutsInhibitor : public input::ShortcutsInhibitorDelegate,
                                   public compositor::SurfaceObserver
{
public:
    KeyboardShortcutsInhibitor(wl_resource *resource, input::Seat *seat,
                               compositor::Surface *surface)
        : m_resource(resource)
        , m_seat(seat)
        , m_surface(surface)
    {
        m_surface->addSurfaceObserver(this);
    }

    ~KeyboardShortcutsInhibitor() override
    {
        m_seat->shortcutsInhibition()->remove(this);
        if (m_surface)
            m_surface->removeSurfaceObserver(this);
    }

    // input::ShortcutsInhibitorDelegate:
    compositor::Surface *inhibitedSurface() const override { return m_surface; }

    void onInhibitorActivated() override
    {
        zwp_keyboard_shortcuts_inhibitor_v1_send_active(m_resource);
    }

    void onInhibitorDeactivated() override
    {
        zwp_keyboard_shortcuts_inhibitor_v1_send_inactive(m_resource);
    }

    // compositor::SurfaceObserver:
    void onSurfaceDestroying(compositor::Surface *surface) override
    {
        surface->removeSurfaceObserver(this);
        m_surface = nullptr;
    }

private:
    wl_resource *const m_resource;
    input::Seat *const m_seat;
    compositor::Surface *m_surface;
};

void keyboard_shortcuts_inhibitor_destroy(wl_client *, wl_resource *resource)
{
    wl_resource_destroy(resource);
}

constexpr struct zwp_keyboard_shortcuts_inhibitor_v1_interface
        keyboard_shortcuts_inhibitor_implementation = {
                keyboard_shortcuts_inhibitor_destroy,
};

void keyboard_shortcuts_inhibit_manager_destroy(wl_client *, wl_resource *resource)
{
    wl_resource_destroy(resource);
}

void keyboard_shortcuts_inhibit_manager_inhibit_shortcuts(wl_client *client,
                                                          wl_resource *resource, uint32_t id,
                                                          wl_resource *surfaceResource,
                                                          wl_resource *)
{
    auto *seat = getUserDataAs<WaylandSeat>(resource)->seat;
    compositor::Surface *surface = surfaceFromResource(surfaceResource);
    wl_resource *inhibitorResource = wl_resource_create(
            client, &zwp_keyboard_shortcuts_inhibitor_v1_interface,
            wl_resource_get_version(resource), id);
    if (!inhibitorResource) {
        wl_client_post_no_memory(client);
        return;
    }
    auto inhibitor = std::make_unique<KeyboardShortcutsInhibitor>(inhibitorResource, seat, surface);
    auto *delegate = inhibitor.get();
    setImplementation(inhibitorResource, &keyboard_shortcuts_inhibitor_implementation,
                      std::move(inhibitor));
    if (!seat->shortcutsInhibition()->add(delegate)) {
        wl_resource_post_error(resource,
                               ZWP_KEYBOARD_SHORTCUTS_INHIBIT_MANAGER_V1_ERROR_ALREADY_INHIBITED,
                               "the associated surface has already been set to inhibit keyboard "
                               "shortcuts");
    }
}

constexpr struct zwp_keyboard_shortcuts_inhibit_manager_v1_interface
        keyboard_shortcuts_inhibit_manager_implementation = {
                keyboard_shortcuts_inhibit_manager_destroy,
                keyboard_shortcuts_inhibit_manager_inhibit_shortcuts,
};

void bind_keyboard_shortcuts_inhibit_manager(wl_client *client, void *data, uint32_t version,
                                             uint32_t id)
{
    wl_resource *resource = wl_resource_create(
            client, &zwp_keyboard_shortcuts_inhibit_manager_v1_interface, version, id);
    if (!resource) {
        wl_client_post_no_memory(client);
        return;
    }
    wl_resource_set_implementation(resource, &keyboard_shortcuts_inhibit_manager_implementation,
                                   data, nullptr);
}

} // namespace

wl_global *createKeyboardShortcutsInhibitManagerGlobal(wl_display *display, WaylandSeat *seat)
{
    return wl_global_create(display, &zwp_keyboard_shortcuts_inhibit_manager_v1_interface, 1, seat,
                            bind_keyboard_shortcuts_inhibit_manager);
}

} // namespace Lingmo::Wayland
