// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef LINGMO_INPUT_GESTURE_FILTER_H_
#define LINGMO_INPUT_GESTURE_FILTER_H_

#include <cstdint>

#include "input/pointer_gesture_delegate.h"

namespace Lingmo::Input {

// Offered every touchpad gesture before any client. A gesture claimed at its
// begin is the compositor's to the end: no client sees any of it.
class GestureFilter
{
public:
    virtual ~GestureFilter() = default;

    virtual bool claimGesture(Timestamp timestamp, GestureType type, uint32_t fingers) = 0;
    virtual void onGestureUpdate(Timestamp timestamp, const GestureUpdate &update) = 0;
    virtual void onGestureEnd(Timestamp timestamp, bool cancelled) = 0;
};

} // namespace Lingmo::Input

#endif // LINGMO_INPUT_GESTURE_FILTER_H_
