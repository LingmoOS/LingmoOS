// Copyright 2015 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.chromium file.
//
// Upstream: chromium/src/components/exo/touch_delegate.h
// Revision: 7acd40ddefff49c9d36144999aafdbb674dbd07a
//
// Adapted for Lingmo: gfx::PointF becomes base::PointF. libinput reports no
// contact size, so the shape callback is not carried over.

#ifndef LINGMO_INPUT_TOUCH_DELEGATE_H_
#define LINGMO_INPUT_TOUCH_DELEGATE_H_

#include <cstdint>


#include "input/types.h"
#include "base/geometry.h"

namespace Lingmo::Compositor {
class Surface;
}

namespace Lingmo::Input {

class Touch;

// Handles events on touch devices in context-specific ways.
class TouchDelegate
{
public:
    // Called at the top of the touch device's destructor, to give the
    // delegate a chance to remove itself.
    virtual void onTouchDestroying(Touch* touch) = 0;

    // Whether `surface` is a valid target for this touch device, e.g. because
    // it is owned by the same client.
    virtual bool canAcceptTouchEventsForSurface(compositor::Surface* surface) const = 0;

    // A new touch point appeared on `surface`, at `location` relative to its
    // origin. Every later event of this point carries the same `id`.
    virtual void onTouchDown(compositor::Surface* surface, Timestamp timestamp, int32_t id,
                             const base::PointF& location) = 0;

    // The touch point is gone; nothing more is sent for `id`.
    virtual void onTouchUp(Timestamp timestamp, int32_t id) = 0;

    // The touch point moved, to `location` relative to the surface it went
    // down on.
    virtual void onTouchMotion(Timestamp timestamp, int32_t id, const base::PointF& location) = 0;

    // The client should apply every touch update since the last frame.
    virtual void onTouchFrame() = 0;

    // Every touch point currently down is cancelled.
    virtual void onTouchCancel() = 0;

protected:
    virtual ~TouchDelegate() = default;
};

} // namespace Lingmo::Input

#endif // LINGMO_INPUT_TOUCH_DELEGATE_H_
