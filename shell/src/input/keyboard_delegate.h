// Copyright 2015 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.chromium file.
//
// Upstream: chromium/src/components/exo/keyboard_delegate.h
// Revision: 7acd40ddefff49c9d36144999aafdbb674dbd07a
//
// Adapted for Lingmo: integrates the upstream behavior with native
// compositor types and ownership.

#ifndef LINGMO_INPUT_KEYBOARD_DELEGATE_H_
#define LINGMO_INPUT_KEYBOARD_DELEGATE_H_

#include <chrono>
#include <map>
#include <string>

#include "input/types.h"

namespace Lingmo::Compositor {
class Surface;
}

namespace Lingmo::Input {

// Handles events on keyboards in context-specific ways.
class KeyboardDelegate
{
public:
    virtual ~KeyboardDelegate() = default;

    // Whether `surface` is a valid target for this keyboard, e.g. because it is
    // owned by the same client.
    virtual bool canAcceptKeyboardEventsForSurface(compositor::Surface* surface) const = 0;

    // Keyed by physical evdev code.
    using PressedKeys = std::map<uint32_t, KeyState>;

    virtual void onKeyboardEnter(compositor::Surface* surface,
                                 const PressedKeys& pressedKeys) = 0;
    virtual void onKeyboardLeave(compositor::Surface* surface) = 0;

    // Returns the serial the client will use to acknowledge the change.
    virtual uint32_t onKeyboardKey(Timestamp timestamp, uint32_t evdevCode, bool pressed) = 0;

    virtual void onKeyboardModifiers(const KeyboardModifiers& modifiers) = 0;

    virtual void onKeyRepeatSettingsChanged(bool enabled, std::chrono::milliseconds delay,
                                            std::chrono::milliseconds interval) = 0;

    virtual void onKeyboardLayoutUpdated(const std::string& keymap) = 0;
};

} // namespace Lingmo::Input

#endif // LINGMO_INPUT_KEYBOARD_DELEGATE_H_
