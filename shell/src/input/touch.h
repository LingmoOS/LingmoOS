// Copyright 2015 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.chromium file.
//
// Upstream: chromium/src/components/exo/touch.h
// Revision: 7acd40ddefff49c9d36144999aafdbb674dbd07a
//
// Adapted for Lingmo: the seat resolves each new touch point's target once
// and hands it to every client's Touch, where aura targets the event before
// its pre-target handlers see it. A frame follows each libinput frame rather
// than each event, which is the end-of-sequence signal upstream's TODO asks
// ui::TouchEvent for. The stylus delegate is not carried over.

#ifndef LINGMO_INPUT_TOUCH_H_
#define LINGMO_INPUT_TOUCH_H_

#include <map>


#include "compositor/surface_observer.h"
#include "base/geometry.h"
#include "input/types.h"

namespace Lingmo::Input {

class Seat;
class TouchDelegate;

// One client's view of the seat's touch devices.
class Touch : public compositor::SurfaceObserver
{
public:
    // The delegate outlives the touch device, or destroys itself from
    // onTouchDestroying(), as the Wayland delegate does upstream.
    Touch(TouchDelegate* delegate, Seat* seat);
    Touch(const Touch&) = delete;
    Touch& operator=(const Touch&) = delete;
    ~Touch() override;

    TouchDelegate* delegate() const { return m_delegate; }

    // Called by the Seat. `target` is the surface the point went down on, or
    // null when it landed on nothing a client owns; `rootLocation` is in
    // output coordinates.
    void onTouchDown(compositor::Surface* target, Timestamp timestamp, int32_t id,
                     const base::PointF& rootLocation);
    void onTouchMotion(Timestamp timestamp, int32_t id, const base::PointF& rootLocation);
    void onTouchUp(Timestamp timestamp, int32_t id);
    void onTouchCancel();
    void onTouchFrame();

    // Whether `id` is a point this client is being sent.
    bool tracks(int32_t id) const { return m_points.contains(id); }

    // compositor::SurfaceObserver:
    void onSurfaceDestroying(compositor::Surface* surface) override;

private:
    void cancelAllTouches();

    TouchDelegate* m_delegate = nullptr;
    Seat* m_seat = nullptr;
    // Each point's surface, and how many points each surface holds, so a
    // surface is observed exactly while it has a point on it.
    std::map<int32_t, compositor::Surface*> m_points;
    std::map<compositor::Surface*, int> m_surfaceCounts;
    // Something was sent since the last frame.
    bool m_framePending = false;
};

} // namespace Lingmo::Input

#endif // LINGMO_INPUT_TOUCH_H_
