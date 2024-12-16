// Copyright (C) 2023 Dingyuan Zhang <lxz@mkacg.com>.
// SPDX-License-Identifier: Apache-2.0 OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "shortcut_manager_impl.h"

#include "server-protocol.h"

#include <QMetaObject>

#define SHORTCUT_MANAGEMENT_V1_VERSION 1

using QW_NAMESPACE::QWDisplay;

static void treeland_shortcut_context_v1_destroy(struct wl_resource *resource);
static treeland_shortcut_manager_v1 *shortcut_manager_from_resource(struct wl_resource *resource);
static treeland_shortcut_context_v1 *shortcut_context_from_resource(struct wl_resource *resource);

static void treeland_shortcut_context_destroy([[maybe_unused]] struct wl_client *client,
                                              struct wl_resource *resource)
{
    wl_resource_destroy(resource);
}

static const struct treeland_shortcut_context_v1_interface shortcut_context_impl
{
    .destroy = treeland_shortcut_context_destroy,
};

void create_shortcut_context_listener(struct wl_client *client,
                                      struct wl_resource *manager_resource,
                                      const char *key,
                                      uint32_t id)
{
    auto *manager = shortcut_manager_from_resource(manager_resource);

    struct wl_resource *resource =
        wl_resource_create(client,
                           &treeland_shortcut_context_v1_interface,
                           TREELAND_SHORTCUT_CONTEXT_V1_SHORTCUT_SINCE_VERSION,
                           id);
    if (resource == nullptr) {
        wl_resource_post_no_memory(manager_resource);
        return;
    }

    auto *context = new treeland_shortcut_context_v1;
    if (context == nullptr) {
        wl_resource_post_no_memory(manager_resource);
        return;
    }

    wl_resource_set_implementation(resource,
                                   &shortcut_context_impl,
                                   context,
                                   treeland_shortcut_context_v1_destroy);

    wl_resource_set_user_data(resource, context);

    context->manager = manager;
    context->key = strdup(key);
    context->resource = resource;

    manager->contexts.append(context);
    QObject::connect(context,
                     &treeland_shortcut_context_v1::beforeDestroy,
                     manager,
                     [context, manager]() {
                         manager->contexts.removeOne(context);
                     });

    uid_t uid;
    wl_client_get_credentials(client, nullptr, &uid, nullptr);
    Q_EMIT manager->newContext(uid, context);
}

static const struct treeland_shortcut_manager_v1_interface shortcut_manager_impl
{
    .register_shortcut_context = create_shortcut_context_listener,
};

static void shortcut_manager_resource_destroy(struct wl_resource *resource)
{
    auto *manager = shortcut_manager_from_resource(resource);
    manager->clients.removeOne(resource);
}

static void treeland_shortcut_context_v1_destroy(struct wl_resource *resource)
{
    auto *context =
        static_cast<treeland_shortcut_context_v1 *>(wl_resource_get_user_data(resource));
    if (!context) {
        return;
    }
    delete context;
}

treeland_shortcut_context_v1::~treeland_shortcut_context_v1()
{
    Q_EMIT beforeDestroy();
}

void treeland_shortcut_context_v1::send_shortcut()
{
    treeland_shortcut_context_v1_send_shortcut(resource);
}

void treeland_shortcut_context_v1::send_register_failed()
{
    wl_resource_post_error(resource,
                           TREELAND_SHORTCUT_CONTEXT_V1_ERROR_REGISTER_FAILED,
                           "register shortcut failed.");
}

static treeland_shortcut_manager_v1 *shortcut_manager_from_resource(struct wl_resource *resource)
{
    assert(wl_resource_instance_of(resource,
                                   &treeland_shortcut_manager_v1_interface,
                                   &shortcut_manager_impl));
    auto *manager =
        static_cast<treeland_shortcut_manager_v1 *>(wl_resource_get_user_data(resource));
    assert(manager != nullptr);
    return manager;
}

static treeland_shortcut_context_v1 *shortcut_context_from_resource(struct wl_resource *resource)
{
    assert(wl_resource_instance_of(resource,
                                   &treeland_shortcut_context_v1_interface,
                                   &shortcut_context_impl));
    auto *context =
        static_cast<treeland_shortcut_context_v1 *>(wl_resource_get_user_data(resource));
    assert(context != nullptr);
    return context;
}

treeland_shortcut_manager_v1::~treeland_shortcut_manager_v1()
{
    Q_EMIT beforeDestroy();
    if (global)
        wl_global_destroy(global);
}

static void treeland_shortcut_manager_bind(struct wl_client *client,
                                           void *data,
                                           uint32_t version,
                                           uint32_t id)
{
    auto *manager = static_cast<treeland_shortcut_manager_v1 *>(data);
    struct wl_resource *resource =
        wl_resource_create(client, &treeland_shortcut_manager_v1_interface, version, id);
    if (!resource) {
        wl_client_post_no_memory(client);
        return;
    }
    wl_resource_set_implementation(resource,
                                   &shortcut_manager_impl,
                                   manager,
                                   shortcut_manager_resource_destroy);

    manager->clients.append(resource);
}

treeland_shortcut_manager_v1 *treeland_shortcut_manager_v1::create(QWDisplay *display)
{
    auto *manager = new treeland_shortcut_manager_v1;
    if (!manager) {
        return nullptr;
    }

    manager->event_loop = wl_display_get_event_loop(display->handle());
    manager->global = wl_global_create(display->handle(),
                                       &treeland_shortcut_manager_v1_interface,
                                       SHORTCUT_MANAGEMENT_V1_VERSION,
                                       manager,
                                       treeland_shortcut_manager_bind);
    if (!manager->global) {
        delete manager;
        return nullptr;
    }

    connect(display, &QWDisplay::beforeDestroy, manager, [manager]() {
        delete manager;
    });

    return manager;
}
