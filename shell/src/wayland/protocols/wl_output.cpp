// Copyright 2018 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.chromium file.
//
// Upstream: chromium/src/components/exo/wayland/wl_output.cc
//           wayland_display_observer.cc (SendDisplayMetrics/SendDisplayMetricsChanges)
//           output_metrics.cc, wayland_display_output.cc
// Revision: 7acd40ddefff49c9d36144999aafdbb674dbd07a
//
// Adapted for Lingmo: integrates the upstream behavior with native
// compositor types and ownership.
// Adaptation: each protocol state owns its metrics and retained resources
// become inert after explicit global removal.

#include "wayland/protocols/wl_output.h"

#include <algorithm>
#include <chrono>
#include <cstring>
#include <memory>
#include <new>
#include <vector>
#include <wayland-server-protocol.h>
#include <xdg-output-unstable-v1-server-protocol.h>

#include "compositor/output.h"
#include "base/event_loop.h"
#include "compositor/surface.h"
#include "core/main_loop.h"
#include "scene/surface_node.h"
#include "wayland/server_util.h"

namespace Lingmo::Wayland {
namespace {

struct Binding;

// Exo's WaylandDisplayOutput::kDeleteTaskDelay.
constexpr std::chrono::seconds kRemovedOutputLifetime(3);

struct OutputState {
    compositor::Output metrics;
    wl_global *global = nullptr;
    bool active = true;
    wl_listener destroy;
    std::vector<Binding *> bindings;
    std::shared_ptr<compositor::CaptureSource> capture =
            std::make_shared<compositor::CaptureSource>();
    // The loop's timer for the delayed destruction, which dies with the
    // display. The loop keeps its task alive while it runs, so the task may
    // delete this state.
    int removal = 0;
};

struct Membership {
    std::shared_ptr<compositor::Surface *> token;
    wl_resource *resource;
};

struct Binding {
    OutputState *output;
    wl_resource *resource;
    std::vector<wl_resource *> extensions;
    std::vector<Membership> members;
};

void destroy(wl_client *, wl_resource *resource)
{
    wl_resource_destroy(resource);
}

const struct wl_output_interface outputImplementation = {destroy};
const struct zxdg_output_v1_interface xdgImplementation = {destroy};

int subpixel(compositor::Output::Subpixel value)
{
    using S = compositor::Output::Subpixel;
    switch (value) {
    case S::None:
        return WL_OUTPUT_SUBPIXEL_NONE;
    case S::HorizontalRgb:
        return WL_OUTPUT_SUBPIXEL_HORIZONTAL_RGB;
    case S::HorizontalBgr:
        return WL_OUTPUT_SUBPIXEL_HORIZONTAL_BGR;
    case S::VerticalRgb:
        return WL_OUTPUT_SUBPIXEL_VERTICAL_RGB;
    case S::VerticalBgr:
        return WL_OUTPUT_SUBPIXEL_VERTICAL_BGR;
    default:
        return WL_OUTPUT_SUBPIXEL_UNKNOWN;
    }
}

int transform(compositor::Output::Transform value)
{
    using T = compositor::Output::Transform;
    switch (value) {
    case T::Rotate90:
        return WL_OUTPUT_TRANSFORM_90;
    case T::Rotate180:
        return WL_OUTPUT_TRANSFORM_180;
    case T::Rotate270:
        return WL_OUTPUT_TRANSFORM_270;
    case T::Normal:
        return WL_OUTPUT_TRANSFORM_NORMAL;
    }
    return WL_OUTPUT_TRANSFORM_NORMAL;
}

bool showsOn(const compositor::Surface *surface, const OutputState &output)
{
    const auto *node = surface->node();
    return node && node->isEffectivelyVisible() && !surface->contentSize().isEmpty()
            && node->absoluteGeometry().intersects(
                    base::Rect(output.metrics.position, output.metrics.logicalSize()));
}

void updateMembership(Binding *binding)
{
    if (!binding->output->active)
        return;
    auto previous = std::move(binding->members);

    struct Context {
        Binding *binding;
        std::vector<Membership> *previous;
    } context {binding, &previous};

    wl_client_for_each_resource(
            wl_resource_get_client(binding->resource),
            [](wl_resource *resource, void *data) {
                auto *context = static_cast<Context *>(data);
                if (std::strcmp(wl_resource_get_class(resource), "wl_surface") != 0)
                    return WL_ITERATOR_CONTINUE;
                auto *surface =
                        static_cast<compositor::Surface *>(wl_resource_get_user_data(resource));
                if (!surface || !*surface->lifetimeToken())
                    return WL_ITERATOR_CONTINUE;
                const bool visible = showsOn(surface, *context->binding->output);
                auto &previous = *context->previous;
                const auto found = std::find_if(previous.begin(), previous.end(),
                                                [surface](const Membership &member) {
                                                    return *member.token == surface;
                                                });
                if (visible) {
                    context->binding->members.push_back({surface->lifetimeToken(), resource});
                    if (found == previous.end())
                        wl_surface_send_enter(resource, context->binding->resource);
                    else
                        previous.erase(found);
                }
                return WL_ITERATOR_CONTINUE;
            },
            &context);
    for (const auto &member : previous)
        if (*member.token)
            wl_surface_send_leave(member.resource, binding->resource);
}

void sendXdg(Binding *binding, wl_resource *resource, bool initial)
{
    if (!binding->output->active)
        return;
    const auto &metrics = binding->output->metrics;
    const int version = wl_resource_get_version(resource);
    // A scaled client lays its outputs out in its own units: Xwayland sizes
    // its X11 screen from these.
    const double scale = clientScale(wl_resource_get_client(resource));
    const base::Point position = (base::PointF(metrics.position) * scale).toPoint();
    zxdg_output_v1_send_logical_position(resource, position.x(), position.y());
    const base::Size size = (base::SizeF(metrics.logicalSize()) * scale).toSize();
    zxdg_output_v1_send_logical_size(resource, size.width(), size.height());
    if (initial && version >= ZXDG_OUTPUT_V1_NAME_SINCE_VERSION)
        zxdg_output_v1_send_name(resource, metrics.name.c_str());
    if (initial && version >= ZXDG_OUTPUT_V1_DESCRIPTION_SINCE_VERSION)
        zxdg_output_v1_send_description(resource, metrics.model.c_str());
    // wl_output v1 has no done event; its xdg extension retains the legacy delimiter.
    if (version < 3 || wl_resource_get_version(binding->resource) < WL_OUTPUT_DONE_SINCE_VERSION)
        zxdg_output_v1_send_done(resource);
}

void done(Binding *binding)
{
    if (wl_resource_get_version(binding->resource) >= WL_OUTPUT_DONE_SINCE_VERSION)
        wl_output_send_done(binding->resource);
}

void sendMetrics(Binding *binding, bool initial)
{
    if (!binding->output->active)
        return;
    const auto &metrics = binding->output->metrics;
    auto *resource = binding->resource;
    const base::Point position =
            (base::PointF(metrics.position) * clientScale(wl_resource_get_client(resource)))
                    .toPoint();
    wl_output_send_geometry(resource, position.x(), position.y(),
                            metrics.physicalSizeMm.width(), metrics.physicalSizeMm.height(),
                            subpixel(metrics.subpixel), metrics.make.c_str(),
                            metrics.model.c_str(), transform(metrics.transform));
    wl_output_send_mode(
            resource, WL_OUTPUT_MODE_CURRENT | (metrics.preferred ? WL_OUTPUT_MODE_PREFERRED : 0),
            metrics.pixelSize.width(), metrics.pixelSize.height(), metrics.refreshMilliHz);
    if (wl_resource_get_version(resource) >= WL_OUTPUT_SCALE_SINCE_VERSION)
        wl_output_send_scale(resource, metrics.scale.integerScale());
    if (initial && wl_resource_get_version(resource) >= WL_OUTPUT_NAME_SINCE_VERSION) {
        wl_output_send_name(resource, metrics.name.c_str());
        const std::string &description =
                metrics.description.empty() ? metrics.model : metrics.description;
        wl_output_send_description(resource, description.c_str());
    }
    for (auto *extension : binding->extensions)
        sendXdg(binding, extension, false);
    done(binding);
}

void bindingDestroyed(wl_resource *resource)
{
    auto *binding = static_cast<Binding *>(wl_resource_get_user_data(resource));
    if (!binding)
        return;
    auto &bindings = binding->output->bindings;
    bindings.erase(std::remove(bindings.begin(), bindings.end(), binding), bindings.end());
    for (auto *extension : binding->extensions)
        wl_resource_set_user_data(extension, nullptr);
    delete binding;
}

void bindOutput(wl_client *client, void *data, uint32_t version, uint32_t id)
{
    auto *output = static_cast<OutputState *>(data);
    auto *resource = wl_resource_create(client, &wl_output_interface, std::min(version, 4u), id);
    if (!resource) {
        wl_client_post_no_memory(client);
        return;
    }
    try {
        auto binding = std::make_unique<Binding>(Binding {output, resource, {}});
        output->bindings.push_back(binding.get());
        wl_resource_set_implementation(resource, &outputImplementation, binding.get(),
                                       bindingDestroyed);
        auto *installed = binding.release();
        sendMetrics(installed, true);
        updateMembership(installed);
    } catch (const std::bad_alloc &) {
        wl_resource_destroy(resource);
        wl_client_post_no_memory(client);
    }
}

// The resources clients still hold stay valid but inert.
void destroyState(OutputState *state)
{
    wl_list_remove(&state->destroy.link);
    if (state->removal)
        core::mainLoop()->stopTimer(state->removal);
    const auto bindings = std::move(state->bindings);
    for (auto *binding : bindings) {
        for (auto *extension : binding->extensions)
            wl_resource_set_user_data(extension, nullptr);
        wl_resource_set_user_data(binding->resource, nullptr);
        delete binding;
    }
    state->capture->active = false;
    state->capture->changed();
    delete state;
}

void extensionDestroyed(wl_resource *resource)
{
    auto *binding = static_cast<Binding *>(wl_resource_get_user_data(resource));
    if (!binding)
        return;
    auto &extensions = binding->extensions;
    extensions.erase(std::remove(extensions.begin(), extensions.end(), resource), extensions.end());
}

void getXdgOutput(wl_client *client, wl_resource *manager, uint32_t id, wl_resource *output)
{
    auto *resource = wl_resource_create(client, &zxdg_output_v1_interface,
                                        wl_resource_get_version(manager), id);
    if (!resource) {
        wl_client_post_no_memory(client);
        return;
    }
    auto *binding = static_cast<Binding *>(wl_resource_get_user_data(output));
    wl_resource_set_implementation(resource, &xdgImplementation, nullptr, extensionDestroyed);
    if (!binding)
        return;
    try {
        binding->extensions.push_back(resource);
    } catch (const std::bad_alloc &) {
        wl_resource_destroy(resource);
        wl_client_post_no_memory(client);
        return;
    }
    wl_resource_set_user_data(resource, binding);
    sendXdg(binding, resource, true);
    done(binding);
}

const struct zxdg_output_manager_v1_interface managerImplementation = {destroy, getXdgOutput};

void bindManager(wl_client *client, void *, uint32_t version, uint32_t id)
{
    auto *resource = wl_resource_create(client, &zxdg_output_manager_v1_interface,
                                        std::min(version, 3u), id);
    if (!resource) {
        wl_client_post_no_memory(client);
        return;
    }
    wl_resource_set_implementation(resource, &managerImplementation, nullptr, nullptr);
}

} // namespace

wl_global *createOutputGlobal(wl_display *display, const compositor::Output *output)
{
    if (!output)
        return nullptr;
    auto *state = new (std::nothrow) OutputState {*output, nullptr, true, {}, {}};
    if (!state)
        return nullptr;
    auto *global = wl_global_create(display, &wl_output_interface, 4, state, bindOutput);
    if (!global) {
        delete state;
        return nullptr;
    }
    state->capture->output = *output;
    state->global = global;
    state->destroy.notify = [](wl_listener *listener, void *) {
        OutputState *state = wl_container_of(listener, state, destroy);
        destroyState(state);
    };
    wl_display_add_destroy_listener(display, &state->destroy);
    return global;
}

void forEachOutputBinding(wl_client *client,
                          const std::shared_ptr<render::PresentationOutput> &output,
                          const std::function<void(wl_resource *)> &callback)
{
    struct Context {
        const std::shared_ptr<render::PresentationOutput> &output;
        const std::function<void(wl_resource *)> &callback;
    } context {output, callback};

    wl_client_for_each_resource(
            client,
            [](wl_resource *resource, void *data) {
                auto &context = *static_cast<Context *>(data);
                if (wl_resource_instance_of(resource, &wl_output_interface,
                                            &outputImplementation)) {
                    auto *binding = static_cast<Binding *>(wl_resource_get_user_data(resource));
                    if (binding && binding->output->active
                        && binding->output->metrics.presentation == context.output)
                        context.callback(resource);
                }
                return WL_ITERATOR_CONTINUE;
            },
            &context);
}

void updateOutputGlobal(wl_global *global, const compositor::Output *output)
{
    if (!global || !output)
        return;
    auto *state = static_cast<OutputState *>(wl_global_get_user_data(global));
    if (!state || !state->active)
        return;
    state->metrics = *output;
    state->capture->output = *output;
    state->capture->changed();
    for (auto *binding : state->bindings)
        sendMetrics(binding, false);
}

void updateOutputMembership(wl_global *global)
{
    if (!global)
        return;
    auto *state = static_cast<OutputState *>(wl_global_get_user_data(global));
    if (!state || !state->active)
        return;
    for (auto *binding : state->bindings)
        updateMembership(binding);
}

void updateOutputMembership(wl_global *global,
                            const std::vector<std::shared_ptr<compositor::Surface *>> &surfaces)
{
    if (!global)
        return;
    auto *state = static_cast<OutputState *>(wl_global_get_user_data(global));
    if (!state || !state->active)
        return;
    for (auto *binding : state->bindings) {
        std::erase_if(binding->members, [](const Membership &member) {
            return !*member.token;
        });
    }
    for (const auto &token : surfaces) {
        compositor::Surface *surface = *token;
        wl_resource *resource = surface ? getSurfaceResource(surface) : nullptr;
        if (!resource)
            continue;
        wl_client *client = wl_resource_get_client(resource);
        for (auto *binding : state->bindings) {
            if (wl_resource_get_client(binding->resource) != client)
                continue;
            auto &members = binding->members;
            const auto found = std::find_if(members.begin(), members.end(),
                                            [surface](const Membership &member) {
                                                return *member.token == surface;
                                            });
            const bool member = found != members.end();
            if (showsOn(surface, *state) == member)
                continue;
            if (member) {
                members.erase(found);
                wl_surface_send_leave(resource, binding->resource);
            } else {
                members.push_back({surface->lifetimeToken(), resource});
                wl_surface_send_enter(resource, binding->resource);
            }
        }
    }
}

void removeOutputGlobal(wl_global *global)
{
    if (!global)
        return;
    auto *state = static_cast<OutputState *>(wl_global_get_user_data(global));
    if (!state || !state->active)
        return;
    state->active = false;
    state->capture->active = false;
    state->capture->changed();
    for (auto *binding : state->bindings) {
        for (const auto &member : binding->members) {
            if (*member.token)
                wl_surface_send_leave(member.resource, binding->resource);
        }
        binding->members.clear();
    }
    // Exo's WaylandDisplayOutput::OnDisplayRemoved: withdraw the global and
    // destroy it later. A client can still be binding it from an announcement
    // it has not seen withdrawn, and binding a destroyed global is a protocol
    // error that disconnects it. Such late bindings get no events, as upstream.
    wl_global_remove(global);
    state->removal = core::mainLoop()->startTimer(kRemovedOutputLifetime, false, [state] {
        state->removal = 0;
        wl_global_destroy(state->global);
        destroyState(state);
    });
}

std::shared_ptr<compositor::CaptureSource> outputCaptureSource(wl_resource *resource)
{
    if (!resource
        || !wl_resource_instance_of(resource, &wl_output_interface, &outputImplementation))
        return {};
    auto *binding = static_cast<Binding *>(wl_resource_get_user_data(resource));
    return binding ? binding->output->capture : nullptr;
}

std::optional<std::string> outputNameFromResource(wl_resource *resource)
{
    if (!resource
        || !wl_resource_instance_of(resource, &wl_output_interface, &outputImplementation)) {
        return std::nullopt;
    }
    auto *binding = static_cast<Binding *>(wl_resource_get_user_data(resource));
    if (!binding || !binding->output->active)
        return std::nullopt;
    return binding->output->metrics.name;
}

wl_global *createXdgOutputManagerGlobal(wl_display *display)
{
    return wl_global_create(display, &zxdg_output_manager_v1_interface, 3, nullptr, bindManager);
}

} // namespace Lingmo::Wayland
