// Copyright 2018 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.chromium file.
//
// Upstream: chromium/src/components/exo/wayland/server_util.h
// Revision: 7acd40ddefff49c9d36144999aafdbb674dbd07a
//
// Adapted for Lingmo: integrates the upstream behavior with native
// compositor types and ownership.

#ifndef LINGMO_WAYLAND_SERVER_UTIL_H_
#define LINGMO_WAYLAND_SERVER_UTIL_H_

#include <wayland-server-core.h>

#include <chrono>
#include <cstdint>
#include <memory>

#include "base/geometry.h"

namespace Lingmo::Compositor {
class Surface;
}

namespace Lingmo::Wayland {

// Exo keeps this association in an aura window property; a side table serves
// the same purpose here. The protocol layer registers a surface when its
// wl_surface is created and clears it when the resource dies, so the input
// delegates can answer "which client owns this surface".
compositor::Surface* surfaceFromResource(wl_resource* resource);
wl_resource* getSurfaceResource(compositor::Surface* surface);
void setSurfaceResource(compositor::Surface* surface, wl_resource* resource);

template<class T>
T* getUserDataAs(wl_resource* resource)
{
    return static_cast<T*>(wl_resource_get_user_data(resource));
}

template<class T>
std::unique_ptr<T> takeUserDataAs(wl_resource* resource)
{
    std::unique_ptr<T> userData(getUserDataAs<T>(resource));
    wl_resource_set_user_data(resource, nullptr);
    return userData;
}

template<class T>
void destroyUserData(wl_resource* resource)
{
    takeUserDataAs<T>(resource);
}

template<class T>
void setImplementation(wl_resource* resource, const void* implementation,
                       std::unique_ptr<T> userData)
{
    wl_resource_set_implementation(resource, implementation, userData.release(),
                                   destroyUserData<T>);
}

void setImplementation(wl_resource* resource, const void* implementation);

// Convert a steady-clock time point to a time value that can be used when
// interfacing with wayland. Note that the value is truncated to uint32_t,
// which can potentially overflow.
uint32_t timePointToMilliseconds(std::chrono::steady_clock::time_point tp);
uint32_t nowInMilliseconds();

// Returns whether a client has initiated destruction. After destruction begins
// resources associated with the client start to be freed and there is a risk of
// UAFs in querying for client resources (see crbug.com/1433187).
bool isClientDestroyed(wl_client* client);

// Server::clientScale for the client's display: 1 for every client but Xwayland.
double clientScale(wl_client* client);

// A request's x, y, width and height as a rectangle. Exo saturates them
// (SkIRect::MakeXYWH, gfx::Rect); here every edge stays within 2^24 of zero,
// past any surface a client can attach, so no sum the compositor makes of them
// overflows. A negative size is empty.
base::Rect clientRect(int32_t x, int32_t y, int32_t width, int32_t height);

// Surface coordinates from a scaled client, in logical units. Rectangles grow
// outwards, so damage and regions never lose a partly covered pixel.
base::Rect clientToLogical(const base::Rect& rect, double scale);
base::Point clientToLogical(const base::Point& point, double scale);

// Logical surface coordinates in a scaled client's units.
base::PointF logicalToClient(const base::PointF& point, double scale);

} // namespace Lingmo::Wayland

#endif // LINGMO_WAYLAND_SERVER_UTIL_H_
