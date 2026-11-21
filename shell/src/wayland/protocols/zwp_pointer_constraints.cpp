// Copyright 2019 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.chromium file.
//
// Upstream: chromium/src/components/exo/wayland/zwp_pointer_constraints.cc
// Revision: 7acd40ddefff49c9d36144999aafdbb674dbd07a
//
// Adapted for Lingmo: confinement, set_region and set_cursor_position_hint,
// which Exo leaves unsupported, are implemented; Xwayland confines the pointer
// for grabs and locks it to emulate XWarpPointer. Exo asks its security
// delegate whether a window may lock; any client may here.

#include "wayland/protocols/zwp_pointer_constraints.h"

#include <optional>
#include <pointer-constraints-unstable-v1-server-protocol.h>
#include <wayland-server-protocol.h>

#include "base/region.h"
#include "compositor/surface.h"
#include "compositor/surface_observer.h"
#include "input/pointer_constraint_delegate.h"
#include "input/pointer_constraints.h"
#include "input/seat.h"
#include "wayland/protocols/wl_seat.h"
#include "wayland/server_util.h"

namespace Lingmo::Wayland {

namespace {

class WaylandPointerConstraintDelegate : public input::PointerConstraintDelegate,
                                         public compositor::SurfaceObserver
{
public:
    WaylandPointerConstraintDelegate(wl_resource *resource, Kind kind, input::Seat *seat,
                                     compositor::Surface *surface,
                                     std::optional<base::Region> region, bool persistent)
        : m_resource(resource)
        , m_kind(kind)
        , m_seat(seat)
        , m_surface(surface)
        , m_region(std::move(region))
        , m_persistent(persistent)
    {
        m_surface->addSurfaceObserver(this);
    }

    ~WaylandPointerConstraintDelegate() override
    {
        m_seat->pointerConstraints()->remove(this);
        if (m_surface)
            m_surface->removeSurfaceObserver(this);
    }

    void setPendingRegion(std::optional<base::Region> region)
    {
        m_pendingRegion = std::move(region);
    }

    void setPendingHint(const base::PointF &hint) { m_pendingHint = hint; }

    // input::PointerConstraintDelegate:
    Kind kind() const override { return m_kind; }

    void onConstraintActivated() override
    {
        if (m_kind == Kind::Lock)
            zwp_locked_pointer_v1_send_locked(m_resource);
        else
            zwp_confined_pointer_v1_send_confined(m_resource);
    }

    // The manager posts the error; this object is never activated.
    void onAlreadyConstrained() override { }

    void onConstraintBroken() override
    {
        if (m_kind == Kind::Lock)
            zwp_locked_pointer_v1_send_unlocked(m_resource);
        else
            zwp_confined_pointer_v1_send_unconfined(m_resource);
    }

    void onDefunct() override
    {
        if (m_surface)
            m_surface->removeSurfaceObserver(this);
        m_surface = nullptr;
    }

    bool isPersistent() const override { return m_persistent; }

    compositor::Surface *constrainedSurface() const override { return m_surface; }

    std::optional<base::Region> region() const override
    {
        return m_region;
    }

    std::optional<base::PointF> cursorPositionHint() const override
    {
        return m_hint;
    }

    // compositor::SurfaceObserver:
    void onSurfaceDestroying(compositor::Surface *surface) override
    {
        surface->removeSurfaceObserver(this);
        m_surface = nullptr;
    }

    // Both are double-buffered state of the surface.
    std::function<void()> onSurfaceCommitRequested(compositor::Surface *) override
    {
        if (!m_pendingRegion && !m_pendingHint)
            return {};
        return [this, region = std::exchange(m_pendingRegion, std::nullopt),
                hint = std::exchange(m_pendingHint, std::nullopt)] {
            if (region)
                m_region = *region;
            if (hint)
                m_hint = hint;
        };
    }

    void onSurfaceCommit(compositor::Surface *) override
    {
        m_seat->pointerConstraints()->update();
    }

private:
    wl_resource *const m_resource;
    const Kind m_kind;
    input::Seat *const m_seat;
    compositor::Surface *m_surface;
    std::optional<base::Region> m_region;
    std::optional<base::PointF> m_hint;
    // Set means a region, possibly null, was requested since the last commit.
    std::optional<std::optional<base::Region>> m_pendingRegion;
    std::optional<base::PointF> m_pendingHint;
    const bool m_persistent;
};

std::optional<base::Region> regionFromResource(wl_client *client, wl_resource *region)
{
    if (!region)
        return std::nullopt;
    const double scale = clientScale(client);
    base::Region result;
    for (const base::Rect &rect : *getUserDataAs<base::Region>(region))
        result += clientToLogical(rect, scale);
    return result;
}

////////////////////////////////////////////////////////////////////////////////
// zwp_locked_pointer_v1_interface:

void locked_pointer_destroy(wl_client *, wl_resource *resource)
{
    wl_resource_destroy(resource);
}

void locked_pointer_set_cursor_position_hint(wl_client *client, wl_resource *resource,
                                             wl_fixed_t surfaceX, wl_fixed_t surfaceY)
{
    const double scale = clientScale(client);
    getUserDataAs<WaylandPointerConstraintDelegate>(resource)->setPendingHint(
            base::PointF(wl_fixed_to_double(surfaceX), wl_fixed_to_double(surfaceY)) / scale);
}

void locked_pointer_set_region(wl_client *client, wl_resource *resource,
                               wl_resource *regionResource)
{
    getUserDataAs<WaylandPointerConstraintDelegate>(resource)->setPendingRegion(
            regionFromResource(client, regionResource));
}

const struct zwp_locked_pointer_v1_interface locked_pointer_implementation = {
        locked_pointer_destroy, locked_pointer_set_cursor_position_hint,
        locked_pointer_set_region};

////////////////////////////////////////////////////////////////////////////////
// zwp_confined_pointer_v1_interface:

void confined_pointer_destroy(wl_client *, wl_resource *resource)
{
    wl_resource_destroy(resource);
}

void confined_pointer_set_region(wl_client *client, wl_resource *resource,
                                 wl_resource *regionResource)
{
    getUserDataAs<WaylandPointerConstraintDelegate>(resource)->setPendingRegion(
            regionFromResource(client, regionResource));
}

const struct zwp_confined_pointer_v1_interface confined_pointer_implementation = {
        confined_pointer_destroy,
        confined_pointer_set_region,
};

////////////////////////////////////////////////////////////////////////////////
// zwp_pointer_constraints_v1_interface:

void pointer_constraints_destroy(wl_client *, wl_resource *resource)
{
    wl_resource_destroy(resource);
}

void constrain(wl_client *client, wl_resource *resource, uint32_t id,
               wl_resource *surfaceResource, wl_resource *regionResource, uint32_t lifetime,
               input::PointerConstraintDelegate::Kind kind)
{
    auto *seat = getUserDataAs<WaylandSeat>(resource)->seat;
    compositor::Surface *surface = surfaceFromResource(surfaceResource);
    if (lifetime != ZWP_POINTER_CONSTRAINTS_V1_LIFETIME_ONESHOT
        && lifetime != ZWP_POINTER_CONSTRAINTS_V1_LIFETIME_PERSISTENT) {
        wl_resource_post_error(resource, WL_DISPLAY_ERROR_INVALID_METHOD, "invalid lifetime %u",
                               lifetime);
        return;
    }

    const bool lock = kind == input::PointerConstraintDelegate::Kind::Lock;
    wl_resource *constraintResource =
            wl_resource_create(client,
                               lock ? &zwp_locked_pointer_v1_interface
                                    : &zwp_confined_pointer_v1_interface,
                               wl_resource_get_version(resource), id);
    if (!constraintResource) {
        wl_client_post_no_memory(client);
        return;
    }
    auto delegate = std::make_unique<WaylandPointerConstraintDelegate>(
            constraintResource, kind, seat, surface, regionFromResource(client, regionResource),
            lifetime == ZWP_POINTER_CONSTRAINTS_V1_LIFETIME_PERSISTENT);
    auto *constraint = delegate.get();
    setImplementation(constraintResource,
                      lock ? static_cast<const void *>(&locked_pointer_implementation)
                           : static_cast<const void *>(&confined_pointer_implementation),
                      std::move(delegate));
    if (!seat->pointerConstraints()->add(constraint)) {
        wl_resource_post_error(resource, ZWP_POINTER_CONSTRAINTS_V1_ERROR_ALREADY_CONSTRAINED,
                               "the surface already has a pointer constraint");
    }
}

void pointer_constraints_lock_pointer(wl_client *client, wl_resource *resource, uint32_t id,
                                      wl_resource *surfaceResource, wl_resource *,
                                      wl_resource *regionResource, uint32_t lifetime)
{
    constrain(client, resource, id, surfaceResource, regionResource, lifetime,
              input::PointerConstraintDelegate::Kind::Lock);
}

void pointer_constraints_confine_pointer(wl_client *client, wl_resource *resource, uint32_t id,
                                         wl_resource *surfaceResource, wl_resource *,
                                         wl_resource *regionResource, uint32_t lifetime)
{
    constrain(client, resource, id, surfaceResource, regionResource, lifetime,
              input::PointerConstraintDelegate::Kind::Confine);
}

const struct zwp_pointer_constraints_v1_interface pointer_constraints_implementation = {
        pointer_constraints_destroy, pointer_constraints_lock_pointer,
        pointer_constraints_confine_pointer};

void bind_pointer_constraints(wl_client *client, void *data, uint32_t version, uint32_t id)
{
    wl_resource *resource =
            wl_resource_create(client, &zwp_pointer_constraints_v1_interface, version, id);
    if (!resource) {
        wl_client_post_no_memory(client);
        return;
    }
    wl_resource_set_implementation(resource, &pointer_constraints_implementation, data, nullptr);
}

} // namespace

wl_global *createPointerConstraintsGlobal(wl_display *display, WaylandSeat *seat)
{
    return wl_global_create(display, &zwp_pointer_constraints_v1_interface, 1, seat,
                            bind_pointer_constraints);
}

} // namespace Lingmo::Wayland
