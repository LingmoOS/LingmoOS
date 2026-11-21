// Copyright 2018 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.chromium file.
//
// Upstream: chromium/src/components/exo/wayland/wayland_touch_delegate.h
// Revision: 7acd40ddefff49c9d36144999aafdbb674dbd07a
//
// Adapted for Lingmo: locations are converted to the client's units, for
// Xwayland's scaled surfaces. There is no contact size to send as a shape.

#ifndef LINGMO_WAYLAND_PROTOCOLS_WAYLAND_TOUCH_DELEGATE_H_
#define LINGMO_WAYLAND_PROTOCOLS_WAYLAND_TOUCH_DELEGATE_H_

#include <wayland-server-core.h>

#include "clipboard/selection.h"
#include "input/touch_delegate.h"

namespace Lingmo::Wayland {

class SerialTracker;

// Touch delegate that accepts events for surfaces owned by the same client as
// the wl_touch resource.
class WaylandTouchDelegate : public input::TouchDelegate
{
public:
    WaylandTouchDelegate(wl_resource* touchResource, SerialTracker* serialTracker,
                         clipboard::SelectionController* selection = nullptr);
    WaylandTouchDelegate(const WaylandTouchDelegate&) = delete;
    WaylandTouchDelegate& operator=(const WaylandTouchDelegate&) = delete;

    // input::TouchDelegate:
    void onTouchDestroying(input::Touch* touch) override;
    bool canAcceptTouchEventsForSurface(compositor::Surface* surface) const override;
    void onTouchDown(compositor::Surface* surface, input::Timestamp timestamp, int32_t id,
                     const base::PointF& location) override;
    void onTouchUp(input::Timestamp timestamp, int32_t id) override;
    void onTouchMotion(input::Timestamp timestamp, int32_t id, const base::PointF& location) override;
    void onTouchFrame() override;
    void onTouchCancel() override;

private:
    wl_client* client() const;

    wl_resource* const m_touchResource;
    SerialTracker* const m_serialTracker;
    base::WeakPtr<clipboard::SelectionController> m_selection;
};

} // namespace Lingmo::Wayland

#endif // LINGMO_WAYLAND_PROTOCOLS_WAYLAND_TOUCH_DELEGATE_H_
