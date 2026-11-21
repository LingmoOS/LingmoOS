// Copyright 2020 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.chromium file.
//
// Upstream: chromium/src/components/exo/xkb_tracker.h
// Revision: 7acd40ddefff49c9d36144999aafdbb674dbd07a
//
// Adapted for Lingmo: integrates the upstream behavior with native
// compositor types and ownership.
//
// Adaptation: Exo derives xkb modifier state from ui::EventFlags through
// ui::XkbModifierConverter. Lingmo is the one processing the physical key
// stream, so it drives xkb_state_update_key directly and lets xkbcommon own
// latch, lock and group bookkeeping.

#ifndef LINGMO_INPUT_XKB_TRACKER_H_
#define LINGMO_INPUT_XKB_TRACKER_H_

#include <memory>
#include <optional>
#include <string>
#include <vector>

#include <xkbcommon/xkbcommon.h>

#include "input/types.h"

namespace Lingmo::Input {

// The evdev key that types a keysym, with the modifier keys to hold for it.
struct KeyStroke {
    uint32_t key = 0;
    std::vector<uint32_t> modifiers;
};

// Owns the xkb context, keymap and state used by every wl_keyboard.
class XkbTracker
{
public:
    XkbTracker();
    XkbTracker(const XkbTracker &) = delete;
    XkbTracker &operator=(const XkbTracker &) = delete;
    ~XkbTracker();

    // Recompiles the keymap for the given layout, e.g. "us" or "de-neo".
    // Everything after the first '-' is the variant, matching
    // ui::XkbKeyboardLayoutEngine::ParseLayoutName; an empty name is the
    // built-in default. Returns false and keeps the previous keymap if the new
    // one does not compile. The new state starts with nothing pressed, and only
    // Caps Lock and Num Lock carry over.
    bool updateKeyboardLayout(const std::string &name);

    // Feeds one physical key transition into the xkb state machine.
    void updateKey(uint32_t evdevCode, bool pressed);

    // The keysym for an evdev code under the current keymap and state.
    uint32_t keysym(uint32_t evdevCode) const;

    // The keysym at the base shift level of the active layout, lower-cased:
    // what ui::KeyboardCode is to Ash accelerators. Shift+1 is "1", not "!".
    uint32_t baseKeysym(uint32_t evdevCode) const;

    // How to type `keysym` under the current keymap, layout and locks, using
    // Shift and the layout's AltGr at most; fewest modifiers first. Nothing
    // when the layout has no key for it.
    std::optional<KeyStroke> keyStroke(uint32_t keysym) const;

    // Whether a modifier is depressed or latched, ignoring locks: Caps Lock
    // does not make Ctrl+A a different accelerator.
    bool modifierPressed(const char *name) const;

    // The layout name updateKeyboardLayout() was last given; empty for the
    // built-in default.
    const std::string &layoutName() const { return m_layoutName; }
    bool keyRepeats(uint32_t evdevCode) const;

    // The keymap in XKB_KEYMAP_FORMAT_TEXT_V1, which is what
    // wl_keyboard.keymap carries.
    std::string keymap() const;

    KeyboardModifiers modifiers() const;
    bool modifierActive(const char *name) const;

    // The lock lights the state has on, as KeyboardLed bits.
    uint32_t leds() const;

    // Takes the locked modifiers and layout of `source`, which must run the
    // same keymap. Returns whether anything changed.
    bool copyLocks(const XkbTracker &source);

    // Evdev codes are offset by 8 from xkb keycodes; the shift comes from X11's
    // keycode range and every Linux keymap is written against it.
    static constexpr uint32_t kEvdevToXkbOffset = 8;

private:
    struct ContextDeleter {
        void operator()(xkb_context *context) const { xkb_context_unref(context); }
    };

    struct KeymapDeleter {
        void operator()(xkb_keymap *keymap) const { xkb_keymap_unref(keymap); }
    };

    struct StateDeleter {
        void operator()(xkb_state *state) const { xkb_state_unref(state); }
    };

    bool updateKeyboardLayoutInternal(const xkb_rule_names *names);

    std::unique_ptr<xkb_context, ContextDeleter> m_context;
    std::unique_ptr<xkb_keymap, KeymapDeleter> m_keymap;
    std::unique_ptr<xkb_state, StateDeleter> m_state;
    std::string m_layoutName;
};

} // namespace Lingmo::Input

#endif // LINGMO_INPUT_XKB_TRACKER_H_
