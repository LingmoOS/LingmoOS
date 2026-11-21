// Copyright 2012 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.chromium file.
//
// Upstream: chromium/src/ui/base/accelerators/accelerator.cc
// Revision: 8abbac30b9cf2fe976422c58c000c94685648b65
//
// Adapted for Lingmo: key names come from xkbcommon rather than the
// localized VKEY tables.

#include "accelerators/accelerator.h"
#include "base/strings.h"

#include <linux/input-event-codes.h>
#include <xkbcommon/xkbcommon.h>
#include <string_view>


namespace Lingmo::Accelerators {

std::string Accelerator::shortcutText() const
{
    std::string text;
    const auto append = [&text](std::string_view part) {
        if (!text.empty())
            text += '+';
        text += part;
    };
    if (isControlDown())
        append("Ctrl");
    if (isAltDown())
        append("Alt");
    if (isShiftDown())
        append("Shift");
    if (isMetaDown())
        append("Meta");
    if (isAltGrDown())
        append("AltGr");
    const std::string key = keysymText(m_keysym);
    append(key.empty() ? keysymName(m_keysym) : key);
    return text;
}

std::string keysymName(uint32_t keysym)
{
    char buffer[64];
    const int length = xkb_keysym_get_name(keysym, buffer, sizeof(buffer));
    if (length <= 0)
        return {};
    return std::string(buffer, size_t(length));
}

uint32_t keysymFromName(const std::string &name)
{
    const xkb_keysym_t keysym = xkb_keysym_from_name(name.c_str(), XKB_KEYSYM_NO_FLAGS);
    return keysym == XKB_KEY_NoSymbol ? 0 : keysym;
}

std::string keysymText(uint32_t keysym)
{
    // Space and the control characters have names; only visible glyphs are text.
    if (keysym == XKB_KEY_space || xkb_keysym_to_utf32(keysym) < 0x21)
        return {};
    char buffer[8];
    const int length = xkb_keysym_to_utf8(keysym, buffer, sizeof(buffer));
    if (length <= 1)
        return {};
    return base::strings::toUpper(std::string_view(buffer, size_t(length - 1)));
}

namespace {

bool isAlphanumericKeysym(uint32_t keysym)
{
    const uint32_t lower = xkb_keysym_to_lower(keysym);
    return (lower >= XKB_KEY_a && lower <= XKB_KEY_z) || (lower >= XKB_KEY_0 && lower <= XKB_KEY_9);
}

} // namespace

uint32_t positionalUsKeysym(uint32_t evdevCode, uint32_t layoutKeysym)
{
    switch (evdevCode) {
    // ui::ShouldPositionallyRemapKey: these positions hold letters on many
    // layouts (the US semicolon is "m" on a French keyboard), so they keep the
    // letter's meaning instead of the position.
    case KEY_SEMICOLON:
    case KEY_APOSTROPHE:
    case KEY_BACKSLASH:
    case KEY_GRAVE:
    case KEY_102ND:
        if (isAlphanumericKeysym(layoutKeysym))
            return 0;
        break;
    case KEY_LEFTBRACE:
    case KEY_RIGHTBRACE:
    case KEY_MINUS:
    case KEY_EQUAL:
    case KEY_COMMA:
    case KEY_DOT:
    case KEY_SLASH:
        break;
    default:
        return 0;
    }

    switch (evdevCode) {
    case KEY_MINUS:
        return XKB_KEY_minus;
    case KEY_EQUAL:
        return XKB_KEY_equal;
    case KEY_SLASH:
        return XKB_KEY_slash;
    case KEY_LEFTBRACE:
        return XKB_KEY_bracketleft;
    case KEY_RIGHTBRACE:
        return XKB_KEY_bracketright;
    case KEY_COMMA:
        return XKB_KEY_comma;
    case KEY_DOT:
        return XKB_KEY_period;
    case KEY_SEMICOLON:
        return XKB_KEY_semicolon;
    case KEY_APOSTROPHE:
        return XKB_KEY_apostrophe;
    case KEY_GRAVE:
        return XKB_KEY_grave;
    case KEY_BACKSLASH:
        return XKB_KEY_backslash;
    case KEY_102ND:
        return XKB_KEY_less;
    default:
        return 0;
    }
}

bool isModifierKeysym(uint32_t keysym)
{
    switch (keysym) {
    case XKB_KEY_Shift_L:
    case XKB_KEY_Shift_R:
    case XKB_KEY_Control_L:
    case XKB_KEY_Control_R:
    case XKB_KEY_Alt_L:
    case XKB_KEY_Alt_R:
    case XKB_KEY_Meta_L:
    case XKB_KEY_Meta_R:
    case XKB_KEY_Super_L:
    case XKB_KEY_Super_R:
    case XKB_KEY_Hyper_L:
    case XKB_KEY_Hyper_R:
    case XKB_KEY_ISO_Level3_Shift:
    case XKB_KEY_ISO_Level5_Shift:
    case XKB_KEY_Caps_Lock:
    case XKB_KEY_Num_Lock:
    case XKB_KEY_Scroll_Lock:
        return true;
    default:
        return false;
    }
}

bool isFunctionKeysym(uint32_t keysym)
{
    return keysym >= XKB_KEY_F1 && keysym <= XKB_KEY_F24;
}

} // namespace Lingmo::Accelerators
