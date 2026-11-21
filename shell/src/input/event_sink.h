// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// The replacement for ui::EventHandler: something that sees physical input
// before the seat dispatches it to clients, and may consume it.

#ifndef LINGMO_INPUT_EVENT_SINK_H_
#define LINGMO_INPUT_EVENT_SINK_H_

#include <cstdint>


#include "input/types.h"
#include "base/geometry.h"

namespace Lingmo::Input {

class EventSink
{
public:
    virtual ~EventSink() = default;

    // Each returns true if the sink consumed the event, in which case the seat
    // does not dispatch it to any client. `location` is in output coordinates.
    virtual bool onPointerMotion(Timestamp timestamp, const base::PointF& location) { return false; }
    virtual bool onPointerButton(Timestamp timestamp, const base::PointF& location,
                                 uint32_t evdevButton, bool pressed)
    {
        return false;
    }
    virtual bool onPointerAxis(Timestamp, const base::PointF&, const base::PointF&, bool) { return false; }
    virtual bool onPointerAxisStop(Timestamp, const base::PointF&) { return false; }
    virtual bool onKey(Timestamp timestamp, uint32_t evdevCode, bool pressed) { return false; }
    // A key on its way to the lock screen instead of onKey(). True keeps it
    // from the lock screen: the volume and brightness keys work there too.
    virtual bool onSecureKey(Timestamp timestamp, uint32_t evdevCode, bool pressed)
    {
        return false;
    }

    // A touch point landing at `location`. Returning true takes the whole
    // point: its motion, release or cancellation come here and none of it
    // reaches a client. A point a client has can still be taken later, by
    // returning true from its motion or release, as a window drag does once
    // the client asked to move with it.
    virtual bool onTouchDown(Timestamp, int32_t, const base::PointF&) { return false; }
    virtual bool onTouchMotion(Timestamp, int32_t, const base::PointF&) { return false; }
    virtual bool onTouchUp(Timestamp, int32_t) { return false; }
    // Every point is gone, taken or not.
    virtual void onTouchCancel(Timestamp) { }
};

} // namespace Lingmo::Input

#endif // LINGMO_INPUT_EVENT_SINK_H_
