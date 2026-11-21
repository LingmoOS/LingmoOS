// Copyright 2019 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.chromium file.
//
// Upstream: chromium/src/components/exo/wayland/zwp_relative_pointer_manager.cc
// Revision: 7acd40ddefff49c9d36144999aafdbb674dbd07a
//
// Adapted for Lingmo: the timestamp is sent in microseconds, as the protocol
// asks, where Exo sends milliseconds; and the motion is in the client's units.

#include "wayland/protocols/zwp_relative_pointer_manager.h"

#include <relative-pointer-unstable-v1-server-protocol.h>

#include "input/pointer.h"
#include "input/relative_pointer_delegate.h"
#include "wayland/protocols/wl_seat.h"
#include "wayland/server_util.h"

namespace Lingmo::Wayland {

namespace {

class WaylandRelativePointerDelegate : public input::RelativePointerDelegate
{
public:
    WaylandRelativePointerDelegate(wl_resource *resource, input::Pointer *pointer)
        : m_resource(resource)
        , m_pointer(pointer)
    {
        if (m_pointer)
            m_pointer->addRelativePointerDelegate(this);
    }

    WaylandRelativePointerDelegate(const WaylandRelativePointerDelegate &) = delete;
    WaylandRelativePointerDelegate &operator=(const WaylandRelativePointerDelegate &) = delete;

    ~WaylandRelativePointerDelegate() override
    {
        if (m_pointer)
            m_pointer->removeRelativePointerDelegate(this);
    }

    void onPointerDestroying(input::Pointer *) override { m_pointer = nullptr; }

    void onPointerRelativeMotion(input::Timestamp timestamp,
                                 const input::RelativeMotion &motion) override
    {
        const auto usec = std::chrono::duration_cast<std::chrono::microseconds>(
                                  timestamp.time_since_epoch())
                                  .count();
        // Unaccelerated motion is in device units, which no client scale applies to.
        const base::PointF delta =
                logicalToClient(motion.delta, clientScale(wl_resource_get_client(m_resource)));
        zwp_relative_pointer_v1_send_relative_motion(
                m_resource, uint32_t(uint64_t(usec) >> 32), uint32_t(usec),
                wl_fixed_from_double(delta.x()), wl_fixed_from_double(delta.y()),
                wl_fixed_from_double(motion.unaccelerated.x()),
                wl_fixed_from_double(motion.unaccelerated.y()));
    }

private:
    wl_resource *const m_resource;
    input::Pointer *m_pointer;
};

void relative_pointer_destroy(wl_client *, wl_resource *resource)
{
    wl_resource_destroy(resource);
}

const struct zwp_relative_pointer_v1_interface relative_pointer_implementation = {
        relative_pointer_destroy};

void relative_pointer_manager_destroy(wl_client *, wl_resource *resource)
{
    wl_resource_destroy(resource);
}

void relative_pointer_manager_get_relative_pointer(wl_client *client, wl_resource *resource,
                                                   uint32_t id, wl_resource *pointerResource)
{
    wl_resource *relativePointerResource = wl_resource_create(
            client, &zwp_relative_pointer_v1_interface, wl_resource_get_version(resource), id);
    if (!relativePointerResource) {
        wl_client_post_no_memory(client);
        return;
    }
    setImplementation(relativePointerResource, &relative_pointer_implementation,
                      std::make_unique<WaylandRelativePointerDelegate>(
                              relativePointerResource, pointerFromResource(pointerResource)));
}

const struct zwp_relative_pointer_manager_v1_interface relative_pointer_manager_implementation = {
        relative_pointer_manager_destroy, relative_pointer_manager_get_relative_pointer};

void bind_relative_pointer_manager(wl_client *client, void *data, uint32_t version, uint32_t id)
{
    wl_resource *resource =
            wl_resource_create(client, &zwp_relative_pointer_manager_v1_interface, version, id);
    if (!resource) {
        wl_client_post_no_memory(client);
        return;
    }
    wl_resource_set_implementation(resource, &relative_pointer_manager_implementation, data,
                                   nullptr);
}

} // namespace

wl_global *createRelativePointerManagerGlobal(wl_display *display)
{
    return wl_global_create(display, &zwp_relative_pointer_manager_v1_interface, 1, nullptr,
                            bind_relative_pointer_manager);
}

} // namespace Lingmo::Wayland
