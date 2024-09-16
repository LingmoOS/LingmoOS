// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Resource.h"

WL_ADDONS_BASE_USE_NAMESPACE

Resource::Resource(struct wl_client *client,
                   const struct wl_interface *interface,
                   int version,
                   uint32_t id)
    : handle(wl_resource_create(client, interface, version, id))
{
    wl_resource_set_user_data(handle, this);
}

void Resource::setImplementation(const void *implementation,
                                 void *object,
                                 wl_resource_destroy_func_t onDestroy)
{
    object_ = object;
    wl_resource_set_implementation(handle, implementation, this, onDestroy);
}

struct wl_client *Resource::client()
{
    return wl_resource_get_client(handle);
}

void Resource::destroy()
{
    wl_resource_destroy(handle);
}
