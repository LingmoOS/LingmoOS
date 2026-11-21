// Copyright 2019 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.chromium file.
//
// Upstream: chromium/src/components/exo/wayland/wp_viewporter.cc
// Revision: 7acd40ddefff49c9d36144999aafdbb674dbd07a

#include "wayland/protocols/wp_viewporter.h"

#include <algorithm>
#include <new>
#include <viewporter-server-protocol.h>

#include "compositor/surface.h"
#include "compositor/surface_observer.h"
#include "wayland/server_util.h"

namespace Lingmo::Wayland {
namespace {

using compositor::Surface;

class Viewport : public compositor::SurfaceObserver
{
public:
    Viewport(Surface* surface, wl_resource* resource)
        : m_surface(surface)
        , m_resource(resource)
    {
        surface->addSurfaceObserver(this);
        surface->setHasViewport(true);
    }

    ~Viewport() override
    {
        if (m_surface) {
            m_surface->removeSurfaceObserver(this);
            m_surface->setCrop(std::nullopt);
            m_surface->setViewport(std::nullopt);
            m_surface->setHasViewport(false);
        }
    }

    Surface* surface()
    {
        if (!m_surface)
            wl_resource_post_error(m_resource, WP_VIEWPORT_ERROR_NO_SURFACE,
                                   "the associated surface was destroyed");
        return m_surface;
    }

    void onSurfaceDestroying(Surface* surface) override
    {
        surface->removeSurfaceObserver(this);
        m_surface = nullptr;
    }

    bool onSurfaceStateApplying(Surface* surface) override
    {
        switch (surface->cachedViewportError()) {
        case Surface::ViewportError::None:
            return true;
        case Surface::ViewportError::BadSize:
            wl_resource_post_error(m_resource, WP_VIEWPORT_ERROR_BAD_SIZE,
                                   "source size must be integral without a destination");
            return false;
        case Surface::ViewportError::OutOfBuffer:
            wl_resource_post_error(m_resource, WP_VIEWPORT_ERROR_OUT_OF_BUFFER,
                                   "source rectangle exceeds effective buffer contents");
            return false;
        }
        return false;
    }

private:
    Surface* m_surface;
    wl_resource* m_resource;
};

void destroy(wl_client*, wl_resource* resource)
{
    wl_resource_destroy(resource);
}

void setSource(wl_client*, wl_resource* resource, wl_fixed_t x, wl_fixed_t y, wl_fixed_t width,
               wl_fixed_t height)
{
    auto* surface = getUserDataAs<Viewport>(resource)->surface();
    if (!surface)
        return;
    if (x == wl_fixed_from_int(-1) && y == wl_fixed_from_int(-1) &&
        width == wl_fixed_from_int(-1) && height == wl_fixed_from_int(-1)) {
        surface->setCrop(std::nullopt);
        return;
    }
    if (x < 0 || y < 0 || width <= 0 || height <= 0) {
        wl_resource_post_error(resource, WP_VIEWPORT_ERROR_BAD_VALUE,
                               "source must have nonnegative origin and positive size");
        return;
    }
    surface->setCrop(base::RectF(wl_fixed_to_double(x), wl_fixed_to_double(y),
                                 wl_fixed_to_double(width), wl_fixed_to_double(height)));
}

void setDestination(wl_client* client, wl_resource* resource, int32_t width, int32_t height)
{
    auto* surface = getUserDataAs<Viewport>(resource)->surface();
    if (!surface)
        return;
    if (width == -1 && height == -1) {
        surface->setViewport(std::nullopt);
        return;
    }
    if (width <= 0 || height <= 0) {
        wl_resource_post_error(resource, WP_VIEWPORT_ERROR_BAD_VALUE,
                               "destination size must be positive");
        return;
    }
    // In the client's surface units; the source stays in buffer units.
    const double scale = clientScale(client);
    surface->setViewport(scale == 1 ? base::Size(width, height)
                                    : base::Size(std::max(1, base::roundToInt(width / scale)),
                                                 std::max(1, base::roundToInt(height / scale))));
}

const struct wp_viewport_interface viewportImplementation = {destroy, setSource, setDestination};

void getViewport(wl_client* client, wl_resource* resource, uint32_t id,
                 wl_resource* surfaceResource)
{
    auto* surface = getUserDataAs<Surface>(surfaceResource);
    if (surface->hasViewport()) {
        wl_resource_post_error(resource, WP_VIEWPORTER_ERROR_VIEWPORT_EXISTS,
                               "a viewport for that surface already exists");
        return;
    }
    auto* viewportResource = wl_resource_create(client, &wp_viewport_interface, 1, id);
    if (!viewportResource) {
        wl_client_post_no_memory(client);
        return;
    }
    try {
        setImplementation(viewportResource, &viewportImplementation,
                          std::make_unique<Viewport>(surface, viewportResource));
    } catch (const std::bad_alloc&) {
        wl_resource_destroy(viewportResource);
        wl_client_post_no_memory(client);
    }
}

const struct wp_viewporter_interface implementation = {destroy, getViewport};

void bind(wl_client* client, void*, uint32_t, uint32_t id)
{
    auto* resource = wl_resource_create(client, &wp_viewporter_interface, 1, id);
    if (!resource) {
        wl_client_post_no_memory(client);
        return;
    }
    wl_resource_set_implementation(resource, &implementation, nullptr, nullptr);
}

} // namespace

wl_global* createViewporterGlobal(wl_display* display)
{
    return wl_global_create(display, &wp_viewporter_interface, 1, nullptr, bind);
}

} // namespace Lingmo::Wayland
