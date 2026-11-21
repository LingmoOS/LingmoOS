// Copyright 2019 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.chromium file.
//
// Upstream: chromium/src/components/exo/wayland/wl_subcompositor.cc
// Revision: 7acd40ddefff49c9d36144999aafdbb674dbd07a
//
// Adapted for Lingmo: integrates the upstream behavior with native
// compositor types and ownership.

#include "wayland/protocols/wl_subcompositor.h"

#include <wayland-server-protocol.h>

#include "compositor/sub_surface.h"
#include "compositor/surface.h"
#include "wayland/server_util.h"

namespace Lingmo::Wayland {

namespace {

using compositor::SubSurface;
using compositor::Surface;

void subsurface_destroy(wl_client*, wl_resource* resource)
{
    wl_resource_destroy(resource);
}

void subsurface_set_position(wl_client*, wl_resource* resource, int32_t x, int32_t y)
{
    getUserDataAs<SubSurface>(resource)->setPosition(base::Point(x, y));
}

void subsurface_place_above(wl_client*, wl_resource* resource, wl_resource* reference)
{
    if (!getUserDataAs<SubSurface>(resource)->placeAbove(getUserDataAs<Surface>(reference)))
        wl_resource_post_error(resource, WL_SUBSURFACE_ERROR_BAD_SURFACE,
                               "reference must be a parent or sibling surface");
}

void subsurface_place_below(wl_client*, wl_resource* resource, wl_resource* reference)
{
    if (!getUserDataAs<SubSurface>(resource)->placeBelow(getUserDataAs<Surface>(reference)))
        wl_resource_post_error(resource, WL_SUBSURFACE_ERROR_BAD_SURFACE,
                               "reference must be a parent or sibling surface");
}

void subsurface_set_sync(wl_client*, wl_resource* resource)
{
    getUserDataAs<SubSurface>(resource)->setCommitBehavior(true);
}

void subsurface_set_desync(wl_client*, wl_resource* resource)
{
    getUserDataAs<SubSurface>(resource)->setCommitBehavior(false);
}

const struct wl_subsurface_interface subsurface_implementation = {
    subsurface_destroy,     subsurface_set_position, subsurface_place_above,
    subsurface_place_below, subsurface_set_sync,     subsurface_set_desync};

void subcompositor_destroy(wl_client*, wl_resource* resource)
{
    wl_resource_destroy(resource);
}

void subcompositor_get_subsurface(wl_client* client, wl_resource* resource, uint32_t id,
                                  wl_resource* surface, wl_resource* parent)
{
    auto subsurface =
        SubSurface::create(getUserDataAs<Surface>(surface), getUserDataAs<Surface>(parent));
    if (!subsurface) {
        wl_resource_post_error(resource, WL_SUBCOMPOSITOR_ERROR_BAD_SURFACE, "invalid surface");
        return;
    }
    wl_resource* sub = wl_resource_create(client, &wl_subsurface_interface, 1, id);
    if (!sub) {
        wl_client_post_no_memory(client);
        return;
    }
    setImplementation(sub, &subsurface_implementation, std::move(subsurface));
}

const struct wl_subcompositor_interface subcompositor_implementation = {
    subcompositor_destroy, subcompositor_get_subsurface};

void bind_subcompositor(wl_client* client, void*, uint32_t, uint32_t id)
{
    wl_resource* resource = wl_resource_create(client, &wl_subcompositor_interface, 1, id);
    if (!resource) {
        wl_client_post_no_memory(client);
        return;
    }
    wl_resource_set_implementation(resource, &subcompositor_implementation, nullptr, nullptr);
}
} // namespace

wl_global* createSubcompositorGlobal(wl_display* display)
{
    return wl_global_create(display, &wl_subcompositor_interface, 1, nullptr, bind_subcompositor);
}
} // namespace Lingmo::Wayland
