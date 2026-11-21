// Copyright 2019 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.chromium file.
//
// Upstream: chromium/src/components/exo/wayland/wl_compositor.cc
// Revision: 7acd40ddefff49c9d36144999aafdbb674dbd07a
//
// Adapted for Lingmo: SkRegion becomes base::Region and the leave/enter callback
// is absent until wl_output exists.

#include "wayland/protocols/wl_compositor.h"

#include <algorithm>
#include <cmath>
#include <memory>

#include "base/region.h"
#include "compositor/compositor.h"
#include "compositor/surface.h"
#include "core/logging.h"
#include "wayland/protocols/wl_shm.h"
#include "wayland/server_util.h"
#include "wayland/server.h"

namespace Lingmo::Wayland {

namespace {

////////////////////////////////////////////////////////////////////////////////
// wl_region_interface:

void region_destroy(wl_client*, wl_resource* resource)
{
    wl_resource_destroy(resource);
}

void region_add(wl_client*, wl_resource* resource, int32_t x, int32_t y, int32_t width,
                int32_t height)
{
    *getUserDataAs<base::Region>(resource) += clientRect(x, y, width, height);
}

void region_subtract(wl_client*, wl_resource* resource, int32_t x, int32_t y, int32_t width,
                     int32_t height)
{
    *getUserDataAs<base::Region>(resource) -= clientRect(x, y, width, height);
}

const struct wl_region_interface region_implementation = { region_destroy, region_add,
                                                           region_subtract };

base::Region regionToLogical(const base::Region& region, double scale)
{
    if (scale == 1)
        return region;
    base::Region result;
    for (const base::Rect& rect : region)
        result += clientToLogical(rect, scale);
    return result;
}

// An opaque region only promises what it fully covers, so it shrinks inwards.
base::Region opaqueToLogical(const base::Region& region, double scale)
{
    if (scale == 1)
        return region;
    base::Region result;
    for (const base::Rect& rect : region) {
        const int left = int(std::ceil(rect.x() / scale));
        const int top = int(std::ceil(rect.y() / scale));
        const int right = int(std::floor((rect.x() + rect.width()) / scale));
        const int bottom = int(std::floor((rect.y() + rect.height()) / scale));
        if (right > left && bottom > top)
            result += base::Rect(left, top, right - left, bottom - top);
    }
    return result;
}

////////////////////////////////////////////////////////////////////////////////
// wl_surface_interface:

void surface_destroy(wl_client*, wl_resource* resource)
{
    wl_resource_destroy(resource);
}

void surface_attach(wl_client* client, wl_resource* resource, wl_resource* buffer, int32_t x,
                    int32_t y)
{
    getUserDataAs<compositor::Surface>(resource)->attach(
        bufferFromResource(buffer), clientToLogical(base::Point(x, y), clientScale(client)));
}

void surface_damage(wl_client* client, wl_resource* resource, int32_t x, int32_t y,
                    int32_t width, int32_t height)
{
    getUserDataAs<compositor::Surface>(resource)->damage(
        clientToLogical(clientRect(x, y, width, height), clientScale(client)));
}

void surface_damage_buffer(wl_client*, wl_resource* resource, int32_t x, int32_t y,
                           int32_t width, int32_t height)
{
    getUserDataAs<compositor::Surface>(resource)->damageBuffer(clientRect(x, y, width, height));
}

// Nulled when the wl_callback resource goes away, so a frame completing after
// the client dropped the callback cannot touch freed memory.
using FrameCallbackToken = std::shared_ptr<wl_resource*>;

struct FrameCallbackOwner {
    explicit FrameCallbackOwner(FrameCallbackToken token)
        : token(std::move(token))
    {
    }
    ~FrameCallbackOwner() { *token = nullptr; }

    FrameCallbackToken token;
};

void surface_frame(wl_client* client, wl_resource* resource, uint32_t callback)
{
    wl_resource* callbackResource =
        wl_resource_create(client, &wl_callback_interface, 1, callback);
    if (!callbackResource) {
        wl_client_post_no_memory(client);
        return;
    }

    auto token = std::make_shared<wl_resource*>(callbackResource);
    getUserDataAs<compositor::Surface>(resource)->requestFrameCallback(
        [token](std::optional<uint32_t> timeMs) {
            wl_resource* callbackResource = *token;
            if (!callbackResource)
                return;

            wl_client* client = wl_resource_get_client(callbackResource);
            if (timeMs && !isClientDestroyed(client)) {
                wl_callback_send_done(callbackResource, *timeMs);
                wl_client_flush(client);
            }
            // Destroying the resource clears the token through
            // FrameCallbackOwner.
            wl_resource_destroy(callbackResource);
        });

    setImplementation(callbackResource, nullptr,
                      std::make_unique<FrameCallbackOwner>(std::move(token)));
}

void surface_set_opaque_region(wl_client* client, wl_resource* resource,
                               wl_resource* regionResource)
{
    // The region is copied here; the client may destroy or reuse the wl_region
    // immediately afterwards.
    const base::Region region =
            regionResource ? *getUserDataAs<base::Region>(regionResource) : base::Region();
    getUserDataAs<compositor::Surface>(resource)->setOpaqueRegion(
        opaqueToLogical(region, clientScale(client)));
}

void surface_set_input_region(wl_client* client, wl_resource* resource,
                              wl_resource* regionResource)
{
    auto* surface = getUserDataAs<compositor::Surface>(resource);
    if (regionResource)
        surface->setInputRegion(
            regionToLogical(*getUserDataAs<base::Region>(regionResource), clientScale(client)));
    else
        surface->resetInputRegion();
}

void surface_commit(wl_client*, wl_resource* resource)
{
    getUserDataAs<compositor::Surface>(resource)->commit();
}

void surface_set_buffer_transform(wl_client*, wl_resource* resource, int32_t transform)
{
    compositor::Transform bufferTransform;
    switch (transform) {
    case WL_OUTPUT_TRANSFORM_NORMAL:
        bufferTransform = compositor::Transform::Normal;
        break;
    case WL_OUTPUT_TRANSFORM_90:
        bufferTransform = compositor::Transform::Rotate90;
        break;
    case WL_OUTPUT_TRANSFORM_180:
        bufferTransform = compositor::Transform::Rotate180;
        break;
    case WL_OUTPUT_TRANSFORM_270:
        bufferTransform = compositor::Transform::Rotate270;
        break;
    case WL_OUTPUT_TRANSFORM_FLIPPED:
        bufferTransform = compositor::Transform::Flipped;
        break;
    case WL_OUTPUT_TRANSFORM_FLIPPED_90:
        bufferTransform = compositor::Transform::FlippedRotate90;
        break;
    case WL_OUTPUT_TRANSFORM_FLIPPED_180:
        bufferTransform = compositor::Transform::FlippedRotate180;
        break;
    case WL_OUTPUT_TRANSFORM_FLIPPED_270:
        bufferTransform = compositor::Transform::FlippedRotate270;
        break;
    default:
        LM_WARNING(lingmoWayland).printf(
                "wl_surface.set_buffer_transform: invalid transform %d", transform);
        wl_resource_post_error(resource, WL_SURFACE_ERROR_INVALID_TRANSFORM,
                               "buffer transform must be one of the values from "
                               "the wl_output.transform enum ('%d' specified)",
                               transform);
        return;
    }

    getUserDataAs<compositor::Surface>(resource)->setBufferTransform(bufferTransform);
}

void surface_set_buffer_scale(wl_client*, wl_resource* resource, int32_t scale)
{
    if (scale < 1) {
        LM_WARNING(lingmoWayland).printf("wl_surface.set_buffer_scale: invalid scale %d", scale);
        wl_resource_post_error(resource, WL_SURFACE_ERROR_INVALID_SCALE,
                               "buffer scale must be at least one ('%d' specified)", scale);
        return;
    }

    getUserDataAs<compositor::Surface>(resource)->setBufferScale(scale);
}

const struct wl_surface_interface surface_implementation = {
    surface_destroy,
    surface_attach,
    surface_damage,
    surface_frame,
    surface_set_opaque_region,
    surface_set_input_region,
    surface_commit,
    surface_set_buffer_transform,
    surface_set_buffer_scale,
    surface_damage_buffer,
    // wl_surface.offset is version 5; we advertise 4, so it is never dispatched.
    nullptr,
};

void surface_destructor(wl_resource* resource)
{
    compositor::Surface* surface = getUserDataAs<compositor::Surface>(resource);
    // The Surface is destroyed while the association still stands, so input
    // observers reacting to its destruction can resolve it back to this
    // resource and send their leave events, as they do upstream. The resource
    // outlives this call, so those events are still valid to send.
    takeUserDataAs<compositor::Surface>(resource).reset();
    setSurfaceResource(surface, nullptr);
}

////////////////////////////////////////////////////////////////////////////////
// wl_compositor_interface:

void compositor_create_surface(wl_client* client, wl_resource* resource, uint32_t id)
{
    auto* compositor = getUserDataAs<compositor::Compositor>(resource);
    std::unique_ptr<compositor::Surface> surface = compositor->createSurface();
    if (auto* server = Server::serverForDisplay(wl_client_get_display(client))) {
        if (server->clientSurfacesInitiallyHidden(client))
            surface->setHidden(true);
        surface->setClientScale(server->clientScale(client));
    }

    wl_resource* surfaceResource = wl_resource_create(
        client, &wl_surface_interface, wl_resource_get_version(resource), id);
    if (!surfaceResource) {
        wl_client_post_no_memory(client);
        return;
    }

    compositor::Surface* surfacePtr = surface.get();
    wl_resource_set_implementation(surfaceResource, &surface_implementation, surface.release(),
                                   surface_destructor);
    setSurfaceResource(surfacePtr, surfaceResource);
}

void compositor_create_region(wl_client* client, wl_resource* resource, uint32_t id)
{
    wl_resource* regionResource = wl_resource_create(client, &wl_region_interface, 1, id);
    if (!regionResource) {
        wl_client_post_no_memory(client);
        return;
    }

    setImplementation(regionResource, &region_implementation, std::make_unique<base::Region>());
}

const struct wl_compositor_interface compositor_implementation = { compositor_create_surface,
                                                                   compositor_create_region };

void bind_compositor(wl_client* client, void* data, uint32_t version, uint32_t id)
{
    wl_resource* resource = wl_resource_create(client, &wl_compositor_interface,
                                               std::min(version, kWlCompositorVersion), id);
    if (!resource) {
        wl_client_post_no_memory(client);
        return;
    }

    wl_resource_set_implementation(resource, &compositor_implementation, data, nullptr);
}

} // namespace

wl_global* createCompositorGlobal(wl_display* display, compositor::Compositor* compositor)
{
    return wl_global_create(display, &wl_compositor_interface, kWlCompositorVersion,
                            compositor, bind_compositor);
}

} // namespace Lingmo::Wayland
