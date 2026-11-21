// Copyright 2015 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.chromium file.
//
// Upstream: chromium/src/components/exo/key_state.h
//           chromium/src/components/exo/keyboard_modifiers.h
// Revision: 7acd40ddefff49c9d36144999aafdbb674dbd07a

#include "input/types.h"

#include <linux/input-event-codes.h>

namespace Lingmo::Input {

ButtonFlags buttonFlagFromEvdev(uint32_t code)
{
    // Mirrors the button table in Exo's WaylandPointerDelegate::OnPointerButton.
    switch (code) {
    case BTN_LEFT:
        return LeftButton;
    case BTN_RIGHT:
        return RightButton;
    case BTN_MIDDLE:
        return MiddleButton;
    case BTN_EXTRA:
        return ForwardButton;
    case BTN_SIDE:
        return BackButton;
    default:
        return NoButton;
    }
}

} // namespace Lingmo::Input
