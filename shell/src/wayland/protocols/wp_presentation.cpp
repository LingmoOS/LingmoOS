// Copyright 2019 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.chromium file.
//
// Upstream: chromium/src/components/exo/wayland/wp_presentation.cc
// Revision: 7acd40ddefff49c9d36144999aafdbb674dbd07a

#include "wayland/protocols/wp_presentation.h"

#include <ctime>
#include <memory>
#include <new>
#include <presentation-time-server-protocol.h>
#include <wayland-server-core.h>

#include "compositor/surface.h"
#include "wayland/protocols/wl_output.h"

namespace Lingmo::Wayland {
namespace {

struct Feedback {
    wl_resource* resource = nullptr;
};

void destroy(wl_client*, wl_resource* resource)
{
    wl_resource_destroy(resource);
}

void feedback(wl_client* client, wl_resource*, wl_resource* surface, uint32_t id)
{
    auto* resource = wl_resource_create(client, &wp_presentation_feedback_interface, 1, id);
    if (!resource) {
        wl_client_post_no_memory(client);
        return;
    }
    try {
        auto state = std::make_shared<Feedback>();
        auto owner = std::make_unique<std::shared_ptr<Feedback>>(state);
        state->resource = resource;
        wl_resource_set_implementation(resource, nullptr, owner.release(), [](wl_resource* r) {
            auto* owner = static_cast<std::shared_ptr<Feedback>*>(wl_resource_get_user_data(r));
            (*owner)->resource = nullptr;
            delete owner;
        });
        static_cast<compositor::Surface*>(wl_resource_get_user_data(surface))
            ->requestPresentationCallback(
                [weak = std::weak_ptr<Feedback>(state)](const render::PresentationResult& result) {
                    auto state = weak.lock();
                    if (!state || !state->resource)
                        return;
                    auto* resource = state->resource;
                    if (!result.valid()) {
                        wp_presentation_feedback_send_discarded(resource);
                    } else {
                        forEachOutputBinding(wl_resource_get_client(resource), result.output,
                                             [resource](wl_resource* output) {
                                                 wp_presentation_feedback_send_sync_output(resource,
                                                                                           output);
                                             });
                        wp_presentation_feedback_send_presented(
                            resource, uint32_t(result.seconds >> 32), uint32_t(result.seconds),
                            result.nanoseconds, result.refresh, uint32_t(result.sequence >> 32),
                            uint32_t(result.sequence), result.flags & 15u);
                    }
                    wl_resource_destroy(resource);
                });
    } catch (const std::bad_alloc&) {
        wl_resource_destroy(resource);
        wl_client_post_no_memory(client);
    }
}

const struct wp_presentation_interface implementation = {destroy, feedback};

void bind(wl_client* client, void*, uint32_t, uint32_t id)
{
    auto* resource = wl_resource_create(client, &wp_presentation_interface, 1, id);
    if (!resource) {
        wl_client_post_no_memory(client);
        return;
    }
    wl_resource_set_implementation(resource, &implementation, nullptr, nullptr);
    wp_presentation_send_clock_id(resource, CLOCK_MONOTONIC);
}

} // namespace

wl_global* createPresentationGlobal(wl_display* display)
{
    return wl_global_create(display, &wp_presentation_interface, 1, nullptr, bind);
}

} // namespace Lingmo::Wayland
