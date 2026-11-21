// Copyright 2023 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.chromium file.
//
// Upstream: chromium/src/components/exo/wayland/wp_fractional_scale.cc
// Revision: 7acd40ddefff49c9d36144999aafdbb674dbd07a

#include "wayland/protocols/wp_fractional_scale.h"

#include <fractional-scale-v1-server-protocol.h>
#include <new>

#include "compositor/surface.h"
#include "compositor/surface_observer.h"
#include "wayland/server_util.h"

namespace Lingmo::Wayland {
namespace {

class FractionalScale final : public compositor::SurfaceObserver
{
public:
    FractionalScale(compositor::Surface* surface, wl_resource* resource)
        : m_surface(surface)
        , m_resource(resource)
    {
        surface->addSurfaceObserver(this);
        surface->setHasFractionalScale(true);
        onScaleFactorChanged(surface);
    }

    ~FractionalScale() override
    {
        if (m_surface) {
            m_surface->removeSurfaceObserver(this);
            m_surface->setHasFractionalScale(false);
        }
    }

    void onScaleFactorChanged(compositor::Surface* surface) override
    {
        const uint32_t scale = surface->preferredScale();
        if (scale != m_wireScale) {
            m_wireScale = scale;
            wp_fractional_scale_v1_send_preferred_scale(m_resource, scale);
        }
    }

    void onSurfaceDestroying(compositor::Surface* surface) override
    {
        surface->removeSurfaceObserver(this);
        m_surface = nullptr;
    }

private:
    compositor::Surface* m_surface;
    wl_resource* m_resource;
    uint32_t m_wireScale = 0;
};

void destroy(wl_client*, wl_resource* resource)
{
    wl_resource_destroy(resource);
}

const struct wp_fractional_scale_v1_interface scaleImplementation = {destroy};

void getScale(wl_client* client, wl_resource* manager, uint32_t id, wl_resource* resource)
{
    auto* surface = getUserDataAs<compositor::Surface>(resource);
    if (surface->hasFractionalScale()) {
        wl_resource_post_error(manager,
                               WP_FRACTIONAL_SCALE_MANAGER_V1_ERROR_FRACTIONAL_SCALE_EXISTS,
                               "a fractional scale object for that surface already exists");
        return;
    }
    auto* extension = wl_resource_create(client, &wp_fractional_scale_v1_interface, 1, id);
    if (!extension) {
        wl_client_post_no_memory(client);
        return;
    }
    try {
        setImplementation(extension, &scaleImplementation,
                          std::make_unique<FractionalScale>(surface, extension));
    } catch (const std::bad_alloc&) {
        wl_resource_destroy(extension);
        wl_client_post_no_memory(client);
    }
}

const struct wp_fractional_scale_manager_v1_interface implementation = {destroy, getScale};

void bind(wl_client* client, void*, uint32_t, uint32_t id)
{
    auto* resource = wl_resource_create(client, &wp_fractional_scale_manager_v1_interface, 1, id);
    if (!resource) {
        wl_client_post_no_memory(client);
        return;
    }
    wl_resource_set_implementation(resource, &implementation, nullptr, nullptr);
}

} // namespace

wl_global* createFractionalScaleGlobal(wl_display* display)
{
    return wl_global_create(display, &wp_fractional_scale_manager_v1_interface, 1, nullptr, bind);
}

} // namespace Lingmo::Wayland
