// Copyright 2018 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.chromium file.
//
// Upstream: chromium/src/components/exo/wayland/wayland_keyboard_delegate.h
// Revision: 7acd40ddefff49c9d36144999aafdbb674dbd07a
//
// Adapted for Lingmo: integrates the upstream behavior with native
// compositor types and ownership.

#ifndef LINGMO_WAYLAND_PROTOCOLS_WAYLAND_KEYBOARD_DELEGATE_H_
#define LINGMO_WAYLAND_PROTOCOLS_WAYLAND_KEYBOARD_DELEGATE_H_

#include <wayland-server-core.h>

#include "clipboard/selection.h"
#include "input/keyboard_delegate.h"

namespace Lingmo::Wayland {

class SerialTracker;

// Keyboard delegate that accepts events for surfaces owned by the same client
// as the wl_keyboard resource.
class WaylandKeyboardDelegate : public input::KeyboardDelegate
{
public:
    WaylandKeyboardDelegate(wl_resource* keyboardResource, SerialTracker* serialTracker,
                            clipboard::SelectionController* selection = nullptr);
    WaylandKeyboardDelegate(const WaylandKeyboardDelegate&) = delete;
    WaylandKeyboardDelegate& operator=(const WaylandKeyboardDelegate&) = delete;
    ~WaylandKeyboardDelegate() override;

    // input::KeyboardDelegate:
    bool canAcceptKeyboardEventsForSurface(compositor::Surface* surface) const override;
    void onKeyboardEnter(compositor::Surface* surface,
                         const PressedKeys& pressedKeys) override;
    void onKeyboardLeave(compositor::Surface* surface) override;
    uint32_t onKeyboardKey(input::Timestamp timestamp, uint32_t evdevCode,
                           bool pressed) override;
    void onKeyboardModifiers(const input::KeyboardModifiers& modifiers) override;
    void onKeyRepeatSettingsChanged(bool enabled, std::chrono::milliseconds delay,
                                    std::chrono::milliseconds interval) override;
    void onKeyboardLayoutUpdated(const std::string& keymap) override;

private:
    void sendKeyboardModifiers();

    wl_client* client() const;

    wl_resource* const m_keyboardResource;
    SerialTracker* const m_serialTracker;
    base::WeakPtr<clipboard::SelectionController> m_selection;

    input::KeyboardModifiers m_currentModifiers;
};

// Converts a repeat interval to the rate in Hz that wl_keyboard.repeat_info
// carries. Exposed for testing.
int32_t waylandRepeatRate(bool enabled, std::chrono::milliseconds interval);

} // namespace Lingmo::Wayland

#endif // LINGMO_WAYLAND_PROTOCOLS_WAYLAND_KEYBOARD_DELEGATE_H_
