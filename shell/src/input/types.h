// Copyright 2015 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.chromium file.
//
// Upstream: chromium/src/components/exo/key_state.h
//           chromium/src/components/exo/keyboard_modifiers.h
// Revision: 7acd40ddefff49c9d36144999aafdbb674dbd07a
//
// Adapted for Lingmo: integrates the upstream behavior with native
// compositor types and ownership.
//
// Lingmo-native replacements for ui::Event / ui::EventFlags / ui::DomCode.
// Keys are evdev codes throughout, which is what both libinput and the Wayland
// protocol speak, so there is no DomCode round trip to get wrong.

#ifndef LINGMO_INPUT_TYPES_H_
#define LINGMO_INPUT_TYPES_H_

#include <chrono>
#include <cstdint>
#include <tuple>

#include "base/geometry.h"

namespace Lingmo::Input {

// libinput timestamps come from CLOCK_MONOTONIC, which is what steady_clock
// reads on Linux, so the two are directly comparable.
using Timestamp = std::chrono::steady_clock::time_point;

// One wheel click in InputManager::onPointerAxis offsets, which are Wayland
// axis units.
constexpr double kAxisUnitsPerClick = 10.0;

// Stands in for ui::EventFlags' mouse-button bits. Exo's delegate passes a
// bitmask of changed buttons, and a mask of pressed buttons on enter.
enum ButtonFlag : uint32_t {
    NoButton = 0,
    LeftButton = 1u << 0,
    RightButton = 1u << 1,
    MiddleButton = 1u << 2,
    ForwardButton = 1u << 3,
    BackButton = 1u << 4,
};

using ButtonFlags = uint32_t;

// Maps an evdev BTN_* code to its flag, or NoButton for a button the pointer
// protocol has no place for.
ButtonFlags buttonFlagFromEvdev(uint32_t code);

// A keyboard's lock lights. The values are libinput_led's.
namespace KeyboardLed {
constexpr uint32_t NumLock = 1 << 0;
constexpr uint32_t CapsLock = 1 << 1;
constexpr uint32_t ScrollLock = 1 << 2;
} // namespace KeyboardLed

// The serialized xkb modifier state, sent verbatim as wl_keyboard.modifiers.
struct KeyboardModifiers {
    uint32_t depressed = 0;
    uint32_t locked = 0;
    uint32_t latched = 0;
    uint32_t group = 0;
};

inline bool operator==(const KeyboardModifiers& lhs, const KeyboardModifiers& rhs)
{
    return std::tie(lhs.depressed, lhs.locked, lhs.latched, lhs.group)
        == std::tie(rhs.depressed, rhs.locked, rhs.latched, rhs.group);
}

// Exo tracks a set of rewritten codes per physical key because ChromeOS's IME
// and remapping layers can turn one physical press into another code. Lingmo
// rewrites ahead of the seat (KeyTranslator), so each code here is already a
// delivered code; the field keeps the release path honest about what it reports.
struct KeyState {
    uint32_t code = 0; // evdev code delivered to the client
};

// Which modifier a keyboard puts next to the space bar: Alt on PC keyboards,
// Command (reported as Meta) on Apple ones, with Option (Alt) beside it.
enum class KeyboardModifierLayout {
    Pc,
    Apple,
};

// How the modifiers left of the space bar behave. Pc is the usual Linux
// arrangement. Command makes the key next to the space bar the primary
// shortcut modifier, the key beside it Option and the outer one Control, on
// every keyboard alike; see KeyTranslator.
enum class ShortcutConvention {
    Pc,
    Command,
};

// Key repeat defaults. Wayland sends a rate in Hz and a delay in milliseconds.
constexpr int kDefaultKeyRepeatDelayMs = 500;
constexpr int kDefaultKeyRepeatIntervalMs = 40;

// ash::KeyRepeatSettings.
struct KeyRepeatSettings {
    bool enabled = true;
    std::chrono::milliseconds delay {kDefaultKeyRepeatDelayMs};
    std::chrono::milliseconds interval {kDefaultKeyRepeatIntervalMs};
};

inline bool operator==(const KeyRepeatSettings& lhs, const KeyRepeatSettings& rhs)
{
    return std::tie(lhs.enabled, lhs.delay, lhs.interval)
        == std::tie(rhs.enabled, rhs.delay, rhs.interval);
}

// Equal up to rounding error, as input locations have always been compared.
inline bool fuzzyEqual(const base::PointF &a, const base::PointF &b)
{
    const auto equal = [](double x, double y) {
        return x == 0 || y == 0 ? base::fuzzyIsNull(x - y) : base::fuzzyCompare(x, y);
    };
    return equal(a.x(), b.x()) && equal(a.y(), b.y());
}

} // namespace Lingmo::Input

#endif // LINGMO_INPUT_TYPES_H_
