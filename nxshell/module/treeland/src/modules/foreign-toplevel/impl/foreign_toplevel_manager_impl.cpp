// Copyright (C) 2023 Dingyuan Zhang <lxz@mkacg.com>.
// SPDX-License-Identifier: Apache-2.0 OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "foreign_toplevel_manager_impl.h"

#include "server-protocol.h"

#include <cassert>

extern "C" {
#define static
#include <wlr/types/wlr_compositor.h>
#include <wlr/types/wlr_output.h>
#include <wlr/types/wlr_seat.h>
#include <wlr/util/log.h>
#undef static
}

using QW_NAMESPACE::QWDisplay, QW_NAMESPACE::QWOutput;

#define FOREIGN_TOPLEVEL_MANAGEMENT_V1_VERSION 1

static void treeland_foreign_toplevel_handle_set_maximized(struct wl_client *client,
                                                           struct wl_resource *resource);
static void treeland_foreign_toplevel_handle_unset_maximized(struct wl_client *client,
                                                             struct wl_resource *resource);
static void treeland_foreign_toplevel_handle_set_minimized(struct wl_client *client,
                                                           struct wl_resource *resource);
static void treeland_foreign_toplevel_handle_unset_minimized(struct wl_client *client,
                                                             struct wl_resource *resource);
static void treeland_foreign_toplevel_handle_activate(struct wl_client *client,
                                                      struct wl_resource *resource,
                                                      struct wl_resource *seat);
static void treeland_foreign_toplevel_handle_close(struct wl_client *client,
                                                   struct wl_resource *resource);
static void treeland_foreign_toplevel_handle_set_rectangle(struct wl_client *client,
                                                           struct wl_resource *resource,
                                                           struct wl_resource *surface,
                                                           int32_t x,
                                                           int32_t y,
                                                           int32_t width,
                                                           int32_t height);
static void treeland_foreign_toplevel_handle_set_fullscreen(struct wl_client *client,
                                                            struct wl_resource *resource,
                                                            struct wl_resource *output);
static void treeland_foreign_toplevel_handle_unset_fullscreen(struct wl_client *client,
                                                              struct wl_resource *resource);
static void treeland_foreign_toplevel_handle_destroy(struct wl_client *client,
                                                     struct wl_resource *resource);
static void treeland_foreign_toplevel_manager_handle_stop(struct wl_client *client,
                                                          struct wl_resource *resource);
static void treeland_foreign_toplevel_manager_handle_get_dock_preview_context(
    struct wl_client *client,
    struct wl_resource *resource,
    struct wl_resource *relative_surface,
    uint32_t id);

static void treeland_dock_preview_context_handle_show(struct wl_client *client,
                                                      struct wl_resource *resource,
                                                      struct wl_array *surfaces,
                                                      int32_t x,
                                                      int32_t y,
                                                      uint32_t direction);
static void treeland_dock_preview_context_handle_close(struct wl_client *client,
                                                       struct wl_resource *resource);

static void treeland_dock_preview_context_handle_destroy(struct wl_client *client,
                                                         struct wl_resource *resource);

static const struct ztreeland_foreign_toplevel_handle_v1_interface treeland_toplevel_handle_impl = {
    .set_maximized = treeland_foreign_toplevel_handle_set_maximized,
    .unset_maximized = treeland_foreign_toplevel_handle_unset_maximized,
    .set_minimized = treeland_foreign_toplevel_handle_set_minimized,
    .unset_minimized = treeland_foreign_toplevel_handle_unset_minimized,
    .activate = treeland_foreign_toplevel_handle_activate,
    .close = treeland_foreign_toplevel_handle_close,
    .set_rectangle = treeland_foreign_toplevel_handle_set_rectangle,
    .destroy = treeland_foreign_toplevel_handle_destroy,
    .set_fullscreen = treeland_foreign_toplevel_handle_set_fullscreen,
    .unset_fullscreen = treeland_foreign_toplevel_handle_unset_fullscreen,
};

static const struct treeland_dock_preview_context_v1_interface
    treeland_dock_preview_context_impl = {
        .show = treeland_dock_preview_context_handle_show,
        .close = treeland_dock_preview_context_handle_close,
        .destroy = treeland_dock_preview_context_handle_destroy,
    };

static struct treeland_foreign_toplevel_handle_v1 *toplevel_handle_from_resource(
    struct wl_resource *resource)
{
    assert(wl_resource_instance_of(resource,
                                   &ztreeland_foreign_toplevel_handle_v1_interface,
                                   &treeland_toplevel_handle_impl));
    return static_cast<struct treeland_foreign_toplevel_handle_v1 *>(
        wl_resource_get_user_data(resource));
}

static struct treeland_dock_preview_context_v1 *toplevel_dock_preview_context_from_resource(
    struct wl_resource *resource)
{
    assert(wl_resource_instance_of(resource,
                                   &treeland_dock_preview_context_v1_interface,
                                   &treeland_dock_preview_context_impl));
    return static_cast<struct treeland_dock_preview_context_v1 *>(
        wl_resource_get_user_data(resource));
}

static void toplevel_handle_send_maximized_event(struct wl_resource *resource, bool state)
{
    struct treeland_foreign_toplevel_handle_v1 *toplevel = toplevel_handle_from_resource(resource);
    if (!toplevel) {
        return;
    }
    struct treeland_foreign_toplevel_handle_v1_maximized_event event = {
        .toplevel = toplevel,
        .maximized = state,
    };
    Q_EMIT toplevel->requestMaximize(&event);
}

static void treeland_foreign_toplevel_handle_set_maximized(
    [[maybe_unused]] struct wl_client *client, struct wl_resource *resource)
{
    toplevel_handle_send_maximized_event(resource, true);
}

static void treeland_foreign_toplevel_handle_unset_maximized(
    [[maybe_unused]] struct wl_client *client, struct wl_resource *resource)
{
    toplevel_handle_send_maximized_event(resource, false);
}

static void treeland_toplevel_send_minimized_event(struct wl_resource *resource, bool state)
{
    auto *toplevel = toplevel_handle_from_resource(resource);
    if (!toplevel) {
        return;
    }

    struct treeland_foreign_toplevel_handle_v1_minimized_event event = {
        .toplevel = toplevel,
        .minimized = state,
    };
    Q_EMIT toplevel->requestMinimize(&event);
}

static void treeland_foreign_toplevel_handle_set_minimized(
    [[maybe_unused]] struct wl_client *client, struct wl_resource *resource)
{
    treeland_toplevel_send_minimized_event(resource, true);
}

static void treeland_foreign_toplevel_handle_unset_minimized(
    [[maybe_unused]] struct wl_client *client, struct wl_resource *resource)
{
    treeland_toplevel_send_minimized_event(resource, false);
}

static void toplevel_send_fullscreen_event(struct wl_resource *resource,
                                           bool state,
                                           struct wl_resource *output_resource)
{
    struct treeland_foreign_toplevel_handle_v1 *toplevel = toplevel_handle_from_resource(resource);
    if (!toplevel) {
        return;
    }

    struct wlr_output *output = NULL;
    if (output_resource) {
        output = wlr_output_from_resource(output_resource);
    }
    struct treeland_foreign_toplevel_handle_v1_fullscreen_event event = {
        .toplevel = toplevel,
        .fullscreen = state,
        .output = output,
    };
    Q_EMIT toplevel->requestFullscreen(&event);
}

static void dock_send_preview_event(struct wl_resource *resource,
                                    struct wl_array *surfaces,
                                    int32_t x,
                                    int32_t y,
                                    int32_t direction)
{
    auto *dock_preview =
        toplevel_dock_preview_context_from_resource(resource);
    if (!dock_preview) {
        return;
    }

    std::vector<uint32_t> s;
    const uint32_t *data = reinterpret_cast<const uint32_t *>(surfaces->data);
    const size_t count = surfaces->size / sizeof(uint32_t);
    for (int i = 0; i != count; ++i) {
        s.push_back(data[i]);
    }

    Q_ASSERT(surfaces->size);

    treeland_dock_preview_context_v1_preview_event event = {
        .toplevel = dock_preview,
        .toplevels = s,
        .x = x,
        .y = y,
        .direction = static_cast<treeland_dock_preview_context_v1_direction>(direction),
    };

    Q_EMIT dock_preview->requestShow(&event);
}

static void treeland_foreign_toplevel_handle_set_fullscreen(
    [[maybe_unused]] struct wl_client *client,
    struct wl_resource *resource,
    struct wl_resource *output)
{
    toplevel_send_fullscreen_event(resource, true, output);
}

static void treeland_foreign_toplevel_handle_unset_fullscreen(
    [[maybe_unused]] struct wl_client *client, struct wl_resource *resource)
{
    toplevel_send_fullscreen_event(resource, false, NULL);
}

static void treeland_foreign_toplevel_handle_activate([[maybe_unused]] struct wl_client *client,
                                                      struct wl_resource *resource,
                                                      struct wl_resource *seat)
{
    auto *toplevel = toplevel_handle_from_resource(resource);
    if (!toplevel) {
        return;
    }
    struct wlr_seat_client *seat_client = wlr_seat_client_from_resource(seat);
    if (!seat_client) {
        return;
    }

    struct treeland_foreign_toplevel_handle_v1_activated_event event = {
        .toplevel = toplevel,
        .seat = seat_client->seat,
    };
    Q_EMIT toplevel->requestActivate(&event);
}

static void treeland_foreign_toplevel_handle_close([[maybe_unused]] struct wl_client *client,
                                                   struct wl_resource *resource)
{
    struct treeland_foreign_toplevel_handle_v1 *toplevel = toplevel_handle_from_resource(resource);
    if (!toplevel) {
        return;
    }
    Q_EMIT toplevel->requestClose();
}

static void treeland_foreign_toplevel_handle_set_rectangle(
    [[maybe_unused]] struct wl_client *client,
    struct wl_resource *resource,
    struct wl_resource *surface,
    int32_t x,
    int32_t y,
    int32_t width,
    int32_t height)
{
    struct treeland_foreign_toplevel_handle_v1 *toplevel = toplevel_handle_from_resource(resource);
    if (!toplevel) {
        return;
    }

    if (width < 0 || height < 0) {
        wl_resource_post_error(resource,
                               ZTREELAND_FOREIGN_TOPLEVEL_HANDLE_V1_ERROR_INVALID_RECTANGLE,
                               "invalid rectangle passed to set_rectangle: width/height < 0");
        return;
    }

    struct treeland_foreign_toplevel_handle_v1_set_rectangle_event event = {
        .toplevel = toplevel,
        .surface = wlr_surface_from_resource(surface),
        .x = x,
        .y = y,
        .width = width,
        .height = height,
    };
    Q_EMIT toplevel->rectangleChanged(&event);
}

static void treeland_foreign_toplevel_handle_destroy([[maybe_unused]] struct wl_client *client,
                                                     struct wl_resource *resource)
{
    wl_resource_destroy(resource);
}

static void toplevel_idle_send_done(void *data)
{
    auto *toplevel = static_cast<treeland_foreign_toplevel_handle_v1 *>(data);
    struct wl_resource *resource;
    wl_resource_for_each(resource, &toplevel->resources)
    {
        ztreeland_foreign_toplevel_handle_v1_send_done(resource);
    }

    toplevel->idle_source = nullptr;
}

void treeland_foreign_toplevel_handle_v1::update_idle_source()
{
    if (idle_source) {
        return;
    }

    idle_source = wl_event_loop_add_idle(manager->event_loop, toplevel_idle_send_done, this);
}

void treeland_foreign_toplevel_handle_v1::set_title(const QString &title)
{
    if (this->title == title)
        return;
    this->title = title;

    struct wl_resource *resource;
    wl_resource_for_each(resource, &this->resources)
    {
        ztreeland_foreign_toplevel_handle_v1_send_title(resource, title.toUtf8());
    }

    update_idle_source();
}

void treeland_foreign_toplevel_handle_v1::set_app_id(const QString &app_id)
{
    if (this->app_id == app_id)
        return;
    this->app_id = app_id;

    struct wl_resource *resource;
    wl_resource_for_each(resource, &this->resources)
    {
        ztreeland_foreign_toplevel_handle_v1_send_app_id(resource, app_id.toLocal8Bit());
    }

    update_idle_source();
}

void treeland_foreign_toplevel_handle_v1::set_pid(const pid_t pid)
{
    this->pid = pid;

    struct wl_resource *resource;
    wl_resource_for_each(resource, &this->resources)
    {
        ztreeland_foreign_toplevel_handle_v1_send_pid(resource, pid);
    }

    update_idle_source();
}

void treeland_foreign_toplevel_handle_v1::set_identifier(uint32_t identifier)
{
    this->identifier = identifier;

    struct wl_resource *resource;
    wl_resource_for_each(resource, &this->resources)
    {
        ztreeland_foreign_toplevel_handle_v1_send_identifier(resource, identifier);
    }

    update_idle_source();
}

static void send_output_to_resource(wl_resource *resource, wlr_output *output, bool enter)
{
    struct wl_client *client = wl_resource_get_client(resource);
    struct wl_resource *output_resource;

    wl_resource_for_each(output_resource, &output->resources)
    {
        if (wl_resource_get_client(output_resource) == client) {
            if (enter) {
                ztreeland_foreign_toplevel_handle_v1_send_output_enter(resource, output_resource);
            } else {
                ztreeland_foreign_toplevel_handle_v1_send_output_leave(resource, output_resource);
            }
        }
    }
}

void treeland_foreign_toplevel_handle_v1::send_output(QWOutput *output, bool enter)
{
    struct wl_resource *resource;
    wl_resource_for_each(resource, &this->resources)
    {
        send_output_to_resource(resource, output->handle(), enter);
    }

    update_idle_source();
}

void treeland_foreign_toplevel_handle_v1::output_enter(QWOutput *output)
{
    if (std::any_of(outputs.begin(),
                    outputs.end(),
                    [output](const treeland_foreign_toplevel_handle_v1_output &toplevel_output) {
                        return toplevel_output.output == output;
                    }))
        return;

    auto toplevel_output =
        treeland_foreign_toplevel_handle_v1_output{ .output = output, .toplevel = this };
    outputs.append(toplevel_output);

    connect(output, &QWOutput::bind, this, [toplevel_output](wlr_output_event_bind *event) {
        struct wl_client *client = wl_resource_get_client(event->resource);

        struct wl_resource *resource;
        wl_resource_for_each(resource, &toplevel_output.toplevel->resources)
        {
            if (wl_resource_get_client(resource) == client) {
                send_output_to_resource(resource, toplevel_output.output->handle(), true);
            }
        }

        toplevel_output.toplevel->update_idle_source();
    });

    connect(output, &QWOutput::beforeDestroy, this, [toplevel_output]() {
        toplevel_output.toplevel->output_leave(toplevel_output.output);
    });
    send_output(output, true);
}

void treeland_foreign_toplevel_handle_v1::output_leave(QWOutput *output)
{
    outputs.removeIf([output](const treeland_foreign_toplevel_handle_v1_output &handle_output) {
        return handle_output.output == output;
    });
    send_output(output, false);
}

static bool fill_array_from_toplevel_state(struct wl_array *array, uint32_t state)
{
    if (state & TREELAND_FOREIGN_TOPLEVEL_HANDLE_V1_STATE_MAXIMIZED) {
        uint32_t *index = static_cast<uint32_t *>(wl_array_add(array, sizeof(uint32_t)));
        if (index == NULL) {
            return false;
        }
        *index = ZTREELAND_FOREIGN_TOPLEVEL_HANDLE_V1_STATE_MAXIMIZED;
    }
    if (state & TREELAND_FOREIGN_TOPLEVEL_HANDLE_V1_STATE_MINIMIZED) {
        uint32_t *index = static_cast<uint32_t *>(wl_array_add(array, sizeof(uint32_t)));
        if (index == NULL) {
            return false;
        }
        *index = ZTREELAND_FOREIGN_TOPLEVEL_HANDLE_V1_STATE_MINIMIZED;
    }
    if (state & TREELAND_FOREIGN_TOPLEVEL_HANDLE_V1_STATE_ACTIVATED) {
        uint32_t *index = static_cast<uint32_t *>(wl_array_add(array, sizeof(uint32_t)));
        if (index == NULL) {
            return false;
        }
        *index = ZTREELAND_FOREIGN_TOPLEVEL_HANDLE_V1_STATE_ACTIVATED;
    }
    if (state & TREELAND_FOREIGN_TOPLEVEL_HANDLE_V1_STATE_FULLSCREEN) {
        uint32_t *index = static_cast<uint32_t *>(wl_array_add(array, sizeof(uint32_t)));
        if (index == NULL) {
            return false;
        }
        *index = ZTREELAND_FOREIGN_TOPLEVEL_HANDLE_V1_STATE_FULLSCREEN;
    }

    return true;
}

void treeland_foreign_toplevel_handle_v1::send_state()
{
    struct wl_array states;
    wl_array_init(&states);
    bool r = fill_array_from_toplevel_state(&states, this->state);
    if (!r) {
        struct wl_resource *resource;
        wl_resource_for_each(resource, &this->resources)
        {
            wl_resource_post_no_memory(resource);
        }

        wl_array_release(&states);
        return;
    }

    struct wl_resource *resource;
    wl_resource_for_each(resource, &this->resources)
    {
        ztreeland_foreign_toplevel_handle_v1_send_state(resource, &states);
    }

    wl_array_release(&states);
    update_idle_source();
}

void treeland_foreign_toplevel_handle_v1::set_maximized(bool maximized)
{
    if (maximized == !!(this->state & TREELAND_FOREIGN_TOPLEVEL_HANDLE_V1_STATE_MAXIMIZED)) {
        return;
    }
    if (maximized) {
        this->state |= TREELAND_FOREIGN_TOPLEVEL_HANDLE_V1_STATE_MAXIMIZED;
    } else {
        this->state &= ~TREELAND_FOREIGN_TOPLEVEL_HANDLE_V1_STATE_MAXIMIZED;
    }
    send_state();
}

void treeland_foreign_toplevel_handle_v1::set_minimized(bool minimized)
{
    if (minimized == !!(this->state & TREELAND_FOREIGN_TOPLEVEL_HANDLE_V1_STATE_MINIMIZED)) {
        return;
    }
    if (minimized) {
        this->state |= TREELAND_FOREIGN_TOPLEVEL_HANDLE_V1_STATE_MINIMIZED;
    } else {
        this->state &= ~TREELAND_FOREIGN_TOPLEVEL_HANDLE_V1_STATE_MINIMIZED;
    }
    send_state();
}

void treeland_foreign_toplevel_handle_v1::set_activated(bool activated)
{
    if (activated == !!(this->state & TREELAND_FOREIGN_TOPLEVEL_HANDLE_V1_STATE_ACTIVATED)) {
        return;
    }
    if (activated) {
        this->state |= TREELAND_FOREIGN_TOPLEVEL_HANDLE_V1_STATE_ACTIVATED;
    } else {
        this->state &= ~TREELAND_FOREIGN_TOPLEVEL_HANDLE_V1_STATE_ACTIVATED;
    }
    send_state();
}

void treeland_foreign_toplevel_handle_v1::set_fullscreen(bool fullscreen)
{
    if (fullscreen == !!(this->state & TREELAND_FOREIGN_TOPLEVEL_HANDLE_V1_STATE_FULLSCREEN)) {
        return;
    }
    if (fullscreen) {
        this->state |= TREELAND_FOREIGN_TOPLEVEL_HANDLE_V1_STATE_FULLSCREEN;
    } else {
        this->state &= ~TREELAND_FOREIGN_TOPLEVEL_HANDLE_V1_STATE_FULLSCREEN;
    }
    send_state();
}

static void toplevel_resource_send_parent(struct wl_resource *toplevel_resource,
                                          struct treeland_foreign_toplevel_handle_v1 *parent)
{
    if (wl_resource_get_version(toplevel_resource)
        < ZTREELAND_FOREIGN_TOPLEVEL_HANDLE_V1_PARENT_SINCE_VERSION) {
        return;
    }
    struct wl_client *client = wl_resource_get_client(toplevel_resource);
    struct wl_resource *parent_resource = NULL;
    if (parent) {
        parent_resource = wl_resource_find_for_client(&parent->resources, client);
        if (!parent_resource) {
            /* don't send an event if this client destroyed the parent handle */
            return;
        }
    }
    ztreeland_foreign_toplevel_handle_v1_send_parent(toplevel_resource, parent_resource);
}

void treeland_foreign_toplevel_handle_v1::set_parent(treeland_foreign_toplevel_handle_v1 *parent)
{
    if (parent == this->parent) {
        /* only send parent event to the clients if there was a change */
        return;
    }
    struct wl_resource *toplevel_resource, *tmp;
    wl_resource_for_each_safe(toplevel_resource, tmp, &this->resources)
    {
        toplevel_resource_send_parent(toplevel_resource, parent);
    }
    this->parent = parent;
    update_idle_source();
}

void treeland_dock_preview_context_v1::enter()
{
    treeland_dock_preview_context_v1_send_enter(resource);
}

void treeland_dock_preview_context_v1::leave()
{
    treeland_dock_preview_context_v1_send_leave(resource);
}

treeland_dock_preview_context_v1::~treeland_dock_preview_context_v1()
{
    Q_EMIT beforeDestroy();

    if (resource) {
        wl_resource_set_user_data(resource, nullptr);
    }
}

treeland_foreign_toplevel_handle_v1::~treeland_foreign_toplevel_handle_v1()
{
    Q_EMIT beforeDestroy();

    struct wl_resource *resource, *tmp;
    wl_resource_for_each_safe(resource, tmp, &this->resources)
    {
        ztreeland_foreign_toplevel_handle_v1_send_closed(resource);
        wl_resource_set_user_data(resource, NULL);
        wl_list_remove(wl_resource_get_link(resource));
        wl_list_init(wl_resource_get_link(resource));
    }

    outputs.clear();

    if (idle_source) {
        wl_event_source_remove(idle_source);
    }

    /* need to ensure no other toplevels hold a pointer to this one as
     * a parent, so that a later call to foreign_toplevel_manager_bind()
     * will not result in a segfault */
    for (auto tl : this->manager->toplevels) {
        if (tl->parent == this) {
            /* Note: we send a parent signal to all clients in this case;
             * the caller should first destroy the child handles if it
             * wishes to avoid this behavior. */
            tl->set_parent(nullptr);
        }
    }
}

static void treeland_foreign_toplevel_resource_destroy(struct wl_resource *resource)
{
    wl_list_remove(wl_resource_get_link(resource));
}

static void treeland_dock_preview_context_handle_show([[maybe_unused]] struct wl_client *client,
                                                      struct wl_resource *resource,
                                                      struct wl_array *surfaces,
                                                      int32_t x,
                                                      int32_t y,
                                                      uint32_t direction)
{
    dock_send_preview_event(resource, surfaces, x, y, direction);
}

static void treeland_dock_preview_context_handle_close([[maybe_unused]] struct wl_client *client,
                                                       struct wl_resource *resource)
{
    auto *dock_preview =
        toplevel_dock_preview_context_from_resource(resource);
    if (!dock_preview) {
        return;
    }

    Q_EMIT dock_preview->requestClose();
}

static void treeland_dock_preview_context_handle_destroy([[maybe_unused]] struct wl_client *client,
                                                         struct wl_resource *resource)
{
    wl_resource_destroy(resource);
}

static struct wl_resource *create_toplevel_resource_for_resource(
    struct treeland_foreign_toplevel_handle_v1 *toplevel, struct wl_resource *manager_resource)
{
    struct wl_client *client = wl_resource_get_client(manager_resource);
    struct wl_resource *resource =
        wl_resource_create(client,
                           &ztreeland_foreign_toplevel_handle_v1_interface,
                           wl_resource_get_version(manager_resource),
                           0);
    if (!resource) {
        wl_client_post_no_memory(client);
        return NULL;
    }

    wl_resource_set_implementation(resource,
                                   &treeland_toplevel_handle_impl,
                                   toplevel,
                                   treeland_foreign_toplevel_resource_destroy);
    wl_resource_set_user_data(resource, toplevel);

    wl_list_insert(&toplevel->resources, wl_resource_get_link(resource));
    ztreeland_foreign_toplevel_manager_v1_send_toplevel(manager_resource, resource);
    return resource;
}

treeland_foreign_toplevel_handle_v1 *treeland_foreign_toplevel_handle_v1::create(
    treeland_foreign_toplevel_manager_v1 *manager)
{
    auto *toplevel = new treeland_foreign_toplevel_handle_v1;
    if (!toplevel) {
        return nullptr;
    }

    manager->toplevels.append(toplevel);
    connect(toplevel,
            &treeland_foreign_toplevel_handle_v1::beforeDestroy,
            manager,
            [manager, toplevel]() {
                manager->toplevels.removeOne(toplevel);
            });

    toplevel->manager = manager;

    wl_list_init(&toplevel->resources);

    struct wl_resource *manager_resource, *tmp;
    wl_resource_for_each_safe(manager_resource, tmp, &manager->resources)
    {
        create_toplevel_resource_for_resource(toplevel, manager_resource);
    }

    return toplevel;
}

static const struct ztreeland_foreign_toplevel_manager_v1_interface
    treeland_foreign_toplevel_manager_impl = {
        .stop = treeland_foreign_toplevel_manager_handle_stop,
        .get_dock_preview_context =
            treeland_foreign_toplevel_manager_handle_get_dock_preview_context,
    };

struct treeland_foreign_toplevel_manager_v1 *foreign_toplevel_manager_from_resource(
    struct wl_resource *resource)
{
    assert(wl_resource_instance_of(resource,
                                   &ztreeland_foreign_toplevel_manager_v1_interface,
                                   &treeland_foreign_toplevel_manager_impl));
    struct treeland_foreign_toplevel_manager_v1 *manager =
        static_cast<treeland_foreign_toplevel_manager_v1 *>(wl_resource_get_user_data(resource));
    assert(manager != nullptr);
    return manager;
}

static void treeland_foreign_toplevel_manager_handle_stop([[maybe_unused]] struct wl_client *client,
                                                          struct wl_resource *resource)
{
    assert(wl_resource_instance_of(resource,
                                   &ztreeland_foreign_toplevel_manager_v1_interface,
                                   &treeland_foreign_toplevel_manager_impl));

    ztreeland_foreign_toplevel_manager_v1_send_finished(resource);
    wl_resource_destroy(resource);
}

static void treeland_dock_preview_context_resource_destroy(struct wl_resource *resource)
{
    auto *context = toplevel_dock_preview_context_from_resource(resource);
    if (!context) {
        return;
    }

    delete context;
}

static void treeland_foreign_toplevel_manager_handle_get_dock_preview_context(
    struct wl_client *client,
    struct wl_resource *manager_resource,
    struct wl_resource *relative_surface,
    uint32_t id)
{
    auto *manager = foreign_toplevel_manager_from_resource(manager_resource);

    auto *context = new treeland_dock_preview_context_v1;
    if (context == nullptr) {
        wl_resource_post_no_memory(manager_resource);
        return;
    }

    uint32_t version = wl_resource_get_version(manager_resource);
    struct wl_resource *resource =
        wl_resource_create(client, &treeland_dock_preview_context_v1_interface, version, id);
    if (resource == nullptr) {
        delete context;
        wl_resource_post_no_memory(manager_resource);
        return;
    }

    wl_resource_set_implementation(resource,
                                   &treeland_dock_preview_context_impl,
                                   context,
                                   treeland_dock_preview_context_resource_destroy);
    wl_resource_set_user_data(resource, context);

    context->manager = manager;
    context->relative_surface = relative_surface;
    context->resource = resource;

    manager->dock_preview.append(context);
    QObject::connect(context, &treeland_dock_preview_context_v1::beforeDestroy, manager, [manager, context]() {
        manager->dock_preview.removeOne(context);
    });

    Q_EMIT context->manager->dockPreviewContextCreated(context);
}

static void treeland_foreign_toplevel_manager_resource_destroy(struct wl_resource *resource)
{
    wl_list_remove(wl_resource_get_link(resource));
}

static void toplevel_send_details_to_toplevel_resource(
    struct treeland_foreign_toplevel_handle_v1 *toplevel, struct wl_resource *resource)
{
    if (!toplevel->title.isEmpty()) {
        ztreeland_foreign_toplevel_handle_v1_send_title(resource, toplevel->title.toUtf8());
    }
    if (!toplevel->app_id.isEmpty()) {
        ztreeland_foreign_toplevel_handle_v1_send_app_id(resource, toplevel->app_id.toLocal8Bit());
    }

    ztreeland_foreign_toplevel_handle_v1_send_pid(resource, toplevel->pid);
    ztreeland_foreign_toplevel_handle_v1_send_identifier(resource, toplevel->identifier);

    for (const auto &output : toplevel->outputs) {
        send_output_to_resource(resource, output.output->handle(), true);
    }

    struct wl_array states;
    wl_array_init(&states);
    bool r = fill_array_from_toplevel_state(&states, toplevel->state);
    if (!r) {
        wl_resource_post_no_memory(resource);
        wl_array_release(&states);
        return;
    }

    ztreeland_foreign_toplevel_handle_v1_send_state(resource, &states);
    wl_array_release(&states);

    toplevel_resource_send_parent(resource, toplevel->parent);

    ztreeland_foreign_toplevel_handle_v1_send_done(resource);
}

static void treeland_foreign_toplevel_manager_bind(struct wl_client *client,
                                                   void *data,
                                                   uint32_t version,
                                                   uint32_t id)
{
    auto *manager = static_cast<struct treeland_foreign_toplevel_manager_v1 *>(data);
    struct wl_resource *resource =
        wl_resource_create(client, &ztreeland_foreign_toplevel_manager_v1_interface, version, id);
    if (!resource) {
        wl_client_post_no_memory(client);
        return;
    }
    wl_resource_set_implementation(resource,
                                   &treeland_foreign_toplevel_manager_impl,
                                   manager,
                                   treeland_foreign_toplevel_manager_resource_destroy);

    wl_list_insert(&manager->resources, wl_resource_get_link(resource));

    /* First loop: create a handle for all toplevels for all clients.
     * Separation into two loops avoid the case where a child handle
     * is created before a parent handle, so the parent relationship
     * could not be sent to a client. */
    for (auto toplevel : manager->toplevels) {
        create_toplevel_resource_for_resource(toplevel, resource);
    }
    /* Second loop: send details about each toplevel. */
    for (auto toplevel : manager->toplevels) {
        struct wl_resource *toplevel_resource =
            wl_resource_find_for_client(&toplevel->resources, client);
        toplevel_send_details_to_toplevel_resource(toplevel, toplevel_resource);
    }
}

treeland_foreign_toplevel_manager_v1::~treeland_foreign_toplevel_manager_v1()
{
    Q_EMIT beforeDestroy();
    if (global)
        wl_global_destroy(global);
}

treeland_foreign_toplevel_manager_v1 *treeland_foreign_toplevel_manager_v1::create(
    QW_NAMESPACE::QWDisplay *display)
{
    auto *manager = new treeland_foreign_toplevel_manager_v1;
    if (!manager) {
        return nullptr;
    }

    manager->event_loop = wl_display_get_event_loop(display->handle());
    manager->global = wl_global_create(display->handle(),
                                       &ztreeland_foreign_toplevel_manager_v1_interface,
                                       FOREIGN_TOPLEVEL_MANAGEMENT_V1_VERSION,
                                       manager,
                                       treeland_foreign_toplevel_manager_bind);
    if (!manager->global) {
        delete manager;
        return nullptr;
    }

    wl_list_init(&manager->resources);

    connect(display, &QWDisplay::beforeDestroy, manager, [manager]() {
        delete manager;
    });

    return manager;
}
