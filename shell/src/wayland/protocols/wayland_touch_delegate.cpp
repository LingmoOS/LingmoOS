// Copyright 2018 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.chromium file.
//
// Upstream: chromium/src/components/exo/wayland/wayland_touch_delegate.cc
// Revision: 7acd40ddefff49c9d36144999aafdbb674dbd07a
//
// Adapted for Lingmo: see wayland_touch_delegate.h.

#include "wayland/protocols/wayland_touch_delegate.h"

#include <wayland-server-protocol.h>

#include "wayland/serial_tracker.h"
#include "wayland/server_util.h"

namespace Lingmo::Wayland {

WaylandTouchDelegate::WaylandTouchDelegate(wl_resource *touchResource,
                                           SerialTracker *serialTracker,
                                           clipboard::SelectionController *selection)
    : m_touchResource(touchResource)
    , m_serialTracker(serialTracker)
    , m_selection(selection ? selection->weakPtr() : nullptr)
{
}

void WaylandTouchDelegate::onTouchDestroying(input::Touch *)
{
    delete this;
}

bool WaylandTouchDelegate::canAcceptTouchEventsForSurface(compositor::Surface *surface) const
{
    wl_resource *surfaceResource = getSurfaceResource(surface);
    return surfaceResource && wl_resource_get_client(surfaceResource) == client();
}

void WaylandTouchDelegate::onTouchDown(compositor::Surface *surface, input::Timestamp timestamp,
                                       int32_t id, const base::PointF &location)
{
    wl_resource *surfaceResource = getSurfaceResource(surface);
    if (!surfaceResource)
        return;
    const uint32_t serial = m_serialTracker->nextSerial(SerialTracker::TOUCH_DOWN);
    // A touch is a user action, as a click is: the client may cite it to
    // activate a window.
    m_serialTracker->recordActivationSerial(serial, m_touchResource);
    if (m_selection)
        m_selection->recordSerial(client(), serial);
    const base::PointF local = logicalToClient(location, clientScale(client()));
    wl_touch_send_down(m_touchResource, serial, timePointToMilliseconds(timestamp),
                       surfaceResource, id, wl_fixed_from_double(local.x()),
                       wl_fixed_from_double(local.y()));
}

void WaylandTouchDelegate::onTouchUp(input::Timestamp timestamp, int32_t id)
{
    const uint32_t serial = m_serialTracker->nextSerial(SerialTracker::TOUCH_UP);
    if (m_selection)
        m_selection->recordSerial(client(), serial);
    wl_touch_send_up(m_touchResource, serial, timePointToMilliseconds(timestamp), id);
}

void WaylandTouchDelegate::onTouchMotion(input::Timestamp timestamp, int32_t id,
                                         const base::PointF &location)
{
    const base::PointF local = logicalToClient(location, clientScale(client()));
    wl_touch_send_motion(m_touchResource, timePointToMilliseconds(timestamp), id,
                         wl_fixed_from_double(local.x()), wl_fixed_from_double(local.y()));
}

void WaylandTouchDelegate::onTouchFrame()
{
    wl_touch_send_frame(m_touchResource);
    wl_client_flush(client());
}

void WaylandTouchDelegate::onTouchCancel()
{
    wl_touch_send_cancel(m_touchResource);
    wl_client_flush(client());
}

wl_client *WaylandTouchDelegate::client() const
{
    return wl_resource_get_client(m_touchResource);
}

} // namespace Lingmo::Wayland
