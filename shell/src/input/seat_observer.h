// Copyright 2017 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.chromium file.
//
// Upstream: chromium/src/components/exo/seat_observer.h
// Revision: 7acd40ddefff49c9d36144999aafdbb674dbd07a
//
// Adapted for Lingmo: integrates the upstream behavior with native
// compositor types and ownership.

#ifndef LINGMO_INPUT_SEAT_OBSERVER_H_
#define LINGMO_INPUT_SEAT_OBSERVER_H_

namespace Lingmo::Compositor {
class Surface;
}

namespace Lingmo::Input {

class SeatObserver
{
public:
    virtual ~SeatObserver() = default;

    // Called when a new surface receives keyboard focus.
    virtual void onSurfaceFocused(compositor::Surface* gainedFocus,
                                  compositor::Surface* lostFocus) = 0;

    // Cursor image ownership changed; independent of keyboard/selection focus.
    virtual void onCursorChanged() { }

    virtual void onCursorPositionChanged() { }

    // Called after the seat's xkb modifier state has been updated.
    virtual void onKeyboardModifierUpdated() {}

    // Called after the seat's keymap was replaced; the modifier state was
    // carried over into the new keymap.
    virtual void onKeyboardLayoutUpdated() {}

    virtual void onKeyRepeatSettingsChanged() {}
};

} // namespace Lingmo::Input

#endif // LINGMO_INPUT_SEAT_OBSERVER_H_
