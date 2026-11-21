// Copyright 2015 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.chromium file.
//
// Upstream: chromium/src/components/exo/keyboard.h
// Revision: 7acd40ddefff49c9d36144999aafdbb674dbd07a
//
// Adapted for Lingmo: integrates the upstream behavior with native
// compositor types and ownership.
//
// Adaptation: the key-ack extension, IME interaction and the virtual-keyboard
// type reporting are dropped. Accelerators are not processed here: without a
// key-ack there is no post-target pass, so accelerators::AcceleratorFilter
// handles them all before the seat dispatches to any keyboard. What remains
// is focus follow, the pressed-key set delivered on enter, modifier and layout
// resend, and repeat settings.

#ifndef LINGMO_INPUT_KEYBOARD_H_
#define LINGMO_INPUT_KEYBOARD_H_

#include <memory>
#include <set>

#include "compositor/surface_observer.h"
#include "input/keyboard_delegate.h"
#include "input/seat_observer.h"
#include "input/types.h"

namespace Lingmo::Input {

class Seat;

// A client keyboard, representing one or more physical keyboards.
class Keyboard : public compositor::SurfaceObserver, public SeatObserver
{
public:
    Keyboard(std::unique_ptr<KeyboardDelegate> delegate, Seat* seat);
    Keyboard(const Keyboard&) = delete;
    Keyboard& operator=(const Keyboard&) = delete;
    ~Keyboard() override;

    KeyboardDelegate* delegate() const { return m_delegate.get(); }

    compositor::Surface* focusedSurface() const { return m_focus; }

    // Called by the Seat for every physical key transition.
    void onKey(Timestamp timestamp, uint32_t evdevCode, bool pressed);

    // compositor::SurfaceObserver:
    void onSurfaceDestroying(compositor::Surface* surface) override;

    // SeatObserver:
    void onSurfaceFocused(compositor::Surface* gainedFocus,
                          compositor::Surface* lostFocus) override;
    void onKeyboardModifierUpdated() override;
    void onKeyboardLayoutUpdated() override;
    void onKeyRepeatSettingsChanged() override;

private:
    void setFocus(compositor::Surface* surface);

    std::unique_ptr<KeyboardDelegate> m_delegate;
    Seat* m_seat = nullptr;

    compositor::Surface* m_focus = nullptr;
    // Another client borrows the keys; `m_focus` keeps focus without a leave.
    bool m_lent = false;
    // Keys held when the borrow ended, pressed while it lasted: the borrower
    // had their press, which may be what ended it, so neither the press nor
    // the release reaches this client.
    std::set<uint32_t> m_borrowedKeys;

    // Keys pressed as of the moment focus was taken, plus everything pressed
    // since. Exo keeps this separate from the seat's set so that a key pressed
    // before this surface was focused is still released to it.
    KeyboardDelegate::PressedKeys m_pressedKeys;
};

} // namespace Lingmo::Input

#endif // LINGMO_INPUT_KEYBOARD_H_
