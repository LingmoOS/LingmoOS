// Copyright 2018 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.chromium file.
//
// Upstream: chromium/src/components/exo/wayland/server_util.cc
// Revision: 7acd40ddefff49c9d36144999aafdbb674dbd07a
//
// Adapted for Lingmo: integrates the upstream behavior with native
// compositor types and ownership.

#include "server_util.h"

#include <algorithm>
#include <cmath>
#include <unordered_map>
#include <utility>

#include <wayland-server-protocol.h>

#include "server.h"

namespace Lingmo::Wayland {

namespace {

std::unordered_map<compositor::Surface*, wl_resource*>& surfaceResources()
{
    static std::unordered_map<compositor::Surface*, wl_resource*> map;
    return map;
}

} // namespace

compositor::Surface* surfaceFromResource(wl_resource* resource)
{
    return resource ? getUserDataAs<compositor::Surface>(resource) : nullptr;
}

wl_resource* getSurfaceResource(compositor::Surface* surface)
{
    const auto& map = surfaceResources();
    const auto it = map.find(surface);
    return it == map.end() ? nullptr : it->second;
}

void setSurfaceResource(compositor::Surface* surface, wl_resource* resource)
{
    if (resource)
        surfaceResources()[surface] = resource;
    else
        surfaceResources().erase(surface);
}

void setImplementation(wl_resource* resource, const void* implementation)
{
    wl_resource_set_implementation(resource, implementation, nullptr, nullptr);
}

uint32_t timePointToMilliseconds(std::chrono::steady_clock::time_point tp)
{
    using namespace std::chrono;
    return static_cast<uint32_t>(duration_cast<milliseconds>(tp.time_since_epoch()).count());
}

uint32_t nowInMilliseconds()
{
    return timePointToMilliseconds(std::chrono::steady_clock::now());
}

double clientScale(wl_client* client)
{
    Server* server = client ? Server::serverForDisplay(wl_client_get_display(client)) : nullptr;
    return server ? server->clientScale(client) : 1;
}

base::Rect clientRect(int32_t x, int32_t y, int32_t width, int32_t height)
{
    static constexpr int64_t kLimit = 1 << 24;
    const auto span = [](int32_t origin, int32_t size) {
        const int64_t start = std::clamp<int64_t>(origin, -kLimit, kLimit);
        const int64_t end =
                std::clamp<int64_t>(int64_t(origin) + std::max(size, 0), start, kLimit);
        return std::pair<int, int>(int(start), int(end - start));
    };
    const auto [left, w] = span(x, width);
    const auto [top, h] = span(y, height);
    return base::Rect(left, top, w, h);
}

base::Rect clientToLogical(const base::Rect& rect, double scale)
{
    if (scale == 1 || rect.isEmpty())
        return rect;
    const int left = int(std::floor(rect.x() / scale));
    const int top = int(std::floor(rect.y() / scale));
    const int right = int(std::ceil((rect.x() + rect.width()) / scale));
    const int bottom = int(std::ceil((rect.y() + rect.height()) / scale));
    return base::Rect::fromEdges(left, top, right, bottom);
}

base::Point clientToLogical(const base::Point& point, double scale)
{
    return scale == 1 ? point
                      : base::Point(base::roundToInt(point.x() / scale),
                                    base::roundToInt(point.y() / scale));
}

base::PointF logicalToClient(const base::PointF& point, double scale)
{
    return point * scale;
}

bool isClientDestroyed(wl_client* client)
{
    // A client always has a display, and the display always outlives it.
    wl_display* display = wl_client_get_display(client);
    Server* server = Server::serverForDisplay(display);
    return server ? server->isClientDestroyed(client) : true;
}

} // namespace Lingmo::Wayland
