// Copyright (C) 2023 rewine <luhongxu@lingmo.org>.
// SPDX-License-Identifier: Apache-2.0 OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "output_manager_impl.h"

#include <wayland-server-core.h>
#include <cassert>

using QW_NAMESPACE::QWDisplay;

static treeland_output_manager_v1 *output_manager_from_resource(wl_resource *resource);
static void output_manager_bind(wl_client *client, void *data, uint32_t version, uint32_t id);

// For treeland_output_manager_v1_interface
static void output_manager_interface_handle_destroy(wl_client *, wl_resource *resource)
{
    wl_list_remove(&resource->link);
    wl_resource_destroy(resource);
}

static void output_manager_handle_set_primary_output([[maybe_unused]] wl_client *client,
                                              wl_resource *resource,
                                              const char *output)
{
    auto *manager = output_manager_from_resource(resource);
    Q_EMIT manager->requestSetPrimaryOutput(output);
}

static const struct treeland_output_manager_v1_interface output_manager_impl
{
    .set_primary_output = output_manager_handle_set_primary_output,
    .destroy = output_manager_interface_handle_destroy,
};

// treeland output manager impl
treeland_output_manager_v1::~treeland_output_manager_v1()
{
    Q_EMIT beforeDestroy();
    // TODO: send stop to all clients
    wl_global_destroy(global);
}

treeland_output_manager_v1 *treeland_output_manager_v1::create(QWDisplay *display)
{
    auto *manager = new treeland_output_manager_v1;
    if (!manager) {
        return nullptr;
    }
    manager->global = wl_global_create(display->handle(),
                                       &treeland_output_manager_v1_interface,
                                       TREELAND_OUTPUT_MANAGER_V1_VERSION,
                                       manager,
                                       output_manager_bind);

    wl_list_init(&manager->resources);

    connect(display, &QWDisplay::beforeDestroy, manager, [manager] { delete manager; });

    return manager;
}

void treeland_output_manager_v1::set_primary_output(const char *name)
{
    this->primary_output_name = name;
    wl_resource *resource;
    wl_list_for_each(resource, &this->resources, link)
    {
        treeland_output_manager_v1_send_primary_output(resource, name);
    }
}

// static func
static treeland_output_manager_v1 *output_manager_from_resource(wl_resource *resource)
{
    assert(wl_resource_instance_of(resource,
                                   &treeland_output_manager_v1_interface,
                                   &output_manager_impl));
    auto *manager = static_cast<treeland_output_manager_v1 *>(wl_resource_get_user_data(resource));
    assert(manager != nullptr);
    return manager;
}

static void output_manager_resource_destroy(struct wl_resource *resource)
{
    wl_list_remove(wl_resource_get_link(resource));
}

static void output_manager_bind(wl_client *client, void *data, uint32_t version, uint32_t id)
{
    auto *manager = static_cast<treeland_output_manager_v1 *>(data);

    struct wl_resource *resource =
        wl_resource_create(client, &treeland_output_manager_v1_interface, version, id);
    if (!resource) {
        wl_client_post_no_memory(client);
        return;
    }

    wl_resource_set_implementation(resource,
                                   &output_manager_impl,
                                   manager,
                                   output_manager_resource_destroy);
    wl_list_insert(&manager->resources, wl_resource_get_link(resource));

    treeland_output_manager_v1_send_primary_output(resource, manager->primary_output_name);
}
