// Copyright 2021 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.chromium file.
//
// Upstream: chromium/src/components/exo/wayland/zwp_idle_inhibit_manager.cc
// Revision: 7acd40ddefff49c9d36144999aafdbb674dbd07a

#include "wayland/protocols/zwp_idle_inhibit_manager.h"
#include "compositor/surface.h"
#include "compositor/surface_observer.h"
#include "scene/surface_node.h"
#include "wayland/idle_inhibition.h"
#include "wayland/server_util.h"
#include <idle-inhibit-unstable-v1-server-protocol.h>

namespace Lingmo::Wayland {
namespace {
class IdleInhibitor : public compositor::SurfaceObserver
{
public:
    IdleInhibitor(IdleInhibition *inhibition, compositor::Surface *surface)
        : m_inhibition(inhibition)
        , m_surface(surface)
    {
        m_surface->addSurfaceObserver(this);
        m_sceneChanged = inhibition->sceneChanged.connect([this] {
            update();
        });
        update();
    }

    ~IdleInhibitor() override
    {
        m_sceneChanged.reset();
        if (m_surface) {
            m_surface->removeSurfaceObserver(this);
        }
        if (m_active)
            m_inhibition->release();
    }

    void onSurfaceDestroying(compositor::Surface *surface) override
    {
        surface->removeSurfaceObserver(this);
        m_surface = nullptr;
        update();
    }

    void onSurfaceCommit(compositor::Surface *) override { update(); }

private:
    void update()
    {
        const bool active = m_surface && m_surface->isMapped() && !m_surface->isHidden()
                && m_surface->node()->isEffectivelyVisible();
        if (active == m_active)
            return;
        m_active = active;
        if (active)
            m_inhibition->acquire();
        else
            m_inhibition->release();
    }

    bool m_active = false;
    base::Subscription m_sceneChanged;
    IdleInhibition *m_inhibition;
    compositor::Surface *m_surface;
};

void destroy(wl_client *, wl_resource *resource)
{
    wl_resource_destroy(resource);
}
const struct zwp_idle_inhibitor_v1_interface inhibitorImplementation = {destroy};

void createInhibitor(wl_client *client, wl_resource *manager, uint32_t id, wl_resource *surface)
{
    auto *resource = wl_resource_create(client, &zwp_idle_inhibitor_v1_interface, 1, id);
    if (!resource) {
        wl_client_post_no_memory(client);
        return;
    }
    setImplementation(resource, &inhibitorImplementation,
                      std::make_unique<IdleInhibitor>(getUserDataAs<IdleInhibition>(manager),
                                                      surfaceFromResource(surface)));
}
const struct zwp_idle_inhibit_manager_v1_interface managerImplementation = {destroy,
                                                                            createInhibitor};

void bind(wl_client *client, void *data, uint32_t, uint32_t id)
{
    auto *resource = wl_resource_create(client, &zwp_idle_inhibit_manager_v1_interface, 1, id);
    if (!resource) {
        wl_client_post_no_memory(client);
        return;
    }
    wl_resource_set_implementation(resource, &managerImplementation, data, nullptr);
}
} // namespace

wl_global *createIdleInhibitManagerGlobal(wl_display *display, IdleInhibition *inhibition)
{
    return wl_global_create(display, &zwp_idle_inhibit_manager_v1_interface, 1, inhibition, bind);
}
} // namespace Lingmo::Wayland
