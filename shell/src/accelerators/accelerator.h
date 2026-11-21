// Copyright 2012 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.chromium file.
//
// Upstream: chromium/src/ui/base/accelerators/accelerator.h
// Revision: 8abbac30b9cf2fe976422c58c000c94685648b65
//
// Adapted for Lingmo: ui::KeyboardCode becomes the xkb keysym at the base
// shift level of the current layout, ui::EventFlags becomes an xkb-derived
// modifier mask, and ui::DomCode becomes the evdev code. Time stamps, source
// device ids and the mouse-interruption flag are not needed.
#pragma once


#include <cstddef>
#include <cstdint>
#include <functional>
#include <tuple>

#include <string>
#include <string_view>

namespace Lingmo::Accelerators {

// ui::EventFlags, reduced to the modifiers an accelerator can carry.
enum Modifier : uint32_t {
    NoModifier = 0,
    ShiftDown = 1u << 0,
    ControlDown = 1u << 1,
    AltDown = 1u << 2,
    MetaDown = 1u << 3,
    AltGrDown = 1u << 4,
};

constexpr uint32_t kModifierMask = ShiftDown | ControlDown | AltDown | MetaDown | AltGrDown;

class Accelerator
{
public:
    enum class KeyState {
        Pressed,
        Released,
    };

    constexpr Accelerator() = default;

    constexpr Accelerator(uint32_t keysym, uint32_t modifiers,
                          KeyState keyState = KeyState::Pressed)
        : m_keysym(keysym)
        , m_modifiers(modifiers & kModifierMask)
        , m_keyState(keyState)
    {
    }

    // ui::Accelerator(KeyboardCode, DomCode, ...): the evdev code pins the key
    // position for the positional lookup in AcceleratorMap.
    constexpr Accelerator(uint32_t keysym, uint32_t evdevCode, uint32_t modifiers,
                          KeyState keyState)
        : m_keysym(keysym)
        , m_evdevCode(evdevCode)
        , m_modifiers(modifiers & kModifierMask)
        , m_keyState(keyState)
    {
    }

    constexpr uint32_t keysym() const { return m_keysym; }

    constexpr uint32_t evdevCode() const { return m_evdevCode; }

    constexpr void resetEvdevCode() { m_evdevCode = 0; }

    constexpr uint32_t modifiers() const { return m_modifiers; }

    constexpr KeyState keyState() const { return m_keyState; }

    constexpr void setKeyState(KeyState state) { m_keyState = state; }

    constexpr bool isShiftDown() const { return m_modifiers & ShiftDown; }

    constexpr bool isControlDown() const { return m_modifiers & ControlDown; }

    constexpr bool isAltDown() const { return m_modifiers & AltDown; }

    constexpr bool isMetaDown() const { return m_modifiers & MetaDown; }

    constexpr bool isAltGrDown() const { return m_modifiers & AltGrDown; }

    // The evdev code is not part of the identity: it only selects a keysym.
    constexpr bool operator==(const Accelerator &rhs) const
    {
        return m_keysym == rhs.m_keysym && m_modifiers == rhs.m_modifiers
                && m_keyState == rhs.m_keyState;
    }

    constexpr bool operator!=(const Accelerator &rhs) const { return !(*this == rhs); }

    constexpr bool operator<(const Accelerator &rhs) const
    {
        return std::tie(m_keysym, m_modifiers, m_keyState)
                < std::tie(rhs.m_keysym, rhs.m_modifiers, rhs.m_keyState);
    }

    size_t hash() const
    {
        return static_cast<size_t>(m_keyState) | (static_cast<size_t>(m_modifiers) << 1)
                | (static_cast<size_t>(m_keysym) << 8);
    }

    // "Ctrl+Shift+Q", for logs and D-Bus results. Modifier order follows
    // ui::Accelerator::GetShortcutText.
    std::string shortcutText() const;

private:
    uint32_t m_keysym = 0;
    uint32_t m_evdevCode = 0;
    uint32_t m_modifiers = NoModifier;
    KeyState m_keyState = KeyState::Pressed;
};

// The keysym name xkbcommon knows the key by ("Up", "XF86AudioRaiseVolume").
std::string keysymName(uint32_t keysym);

// Parses what keysymName() produced; 0 when the name is unknown.
uint32_t keysymFromName(const std::string &name);

// The printable character for a keysym, or an empty string. Letters come back
// upper-cased so "Ctrl+Q" reads as it does on the key cap.
std::string keysymText(uint32_t keysym);

// The key at `evdevCode` on a US layout, for the subset of punctuation keys
// ChromeOS pins by position. Port of
// ui::KeycodeConverter::MapPositionalDomCodeToUSShortcutKey: returns 0 when
// the code is not a positional key, or when the layout puts a letter or a
// digit there, since that key then means the letter and not the position.
uint32_t positionalUsKeysym(uint32_t evdevCode, uint32_t layoutKeysym);

bool isModifierKeysym(uint32_t keysym);
bool isFunctionKeysym(uint32_t keysym);

} // namespace Lingmo::Accelerators

namespace std {
template<>
struct hash<Lingmo::Accelerators::Accelerator> {
    size_t operator()(const Lingmo::Accelerators::Accelerator &accelerator) const
    {
        return accelerator.hash();
    }
};
} // namespace std

