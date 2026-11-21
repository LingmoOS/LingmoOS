// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include <wayland-server-core.h>

namespace Lingmo::Wayland {

class ResourceReference
{
public:
    ResourceReference() { wl_list_init(&m_listener.link); }

    ~ResourceReference() { wl_list_remove(&m_listener.link); }

    ResourceReference(const ResourceReference &) = delete;
    ResourceReference &operator=(const ResourceReference &) = delete;

    void reset(wl_resource *resource)
    {
        wl_list_remove(&m_listener.link);
        wl_list_init(&m_listener.link);
        m_resource = resource;
        m_listener.notify = [](wl_listener *listener, void *) {
            auto *self = reinterpret_cast<ResourceReference *>(listener);
            self->reset(nullptr);
        };
        if (resource)
            wl_resource_add_destroy_listener(resource, &m_listener);
    }

    wl_resource *get() const { return m_resource; }

private:
    wl_listener m_listener {};
    wl_resource *m_resource = nullptr;
};

} // namespace Lingmo::Wayland
