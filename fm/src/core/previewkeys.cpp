// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "core/previewkeys.h"

#include "ui/event.h"
#include "ui/keys.h"

namespace fm::previewkeys {

static constexpr int kEscape = 0x01000000;
static constexpr int kLeft = 0x01000012;
static constexpr int kUp = 0x01000013;
static constexpr int kRight = 0x01000014;
static constexpr int kDown = 0x01000015;
static constexpr int kSpace = 0x20;

static constexpr int kShift = 0x02000000;
static constexpr int kControl = 0x04000000;
static constexpr int kAlt = 0x08000000;
static constexpr int kMeta = 0x10000000;

int toWireKey(uint32_t keysym)
{
    using namespace Lingmo::ui;
    switch (keysym) {
    case key::Escape:
        return kEscape;
    case key::Left:
        return kLeft;
    case key::Up:
        return kUp;
    case key::Right:
        return kRight;
    case key::Down:
        return kDown;
    case key::Space:
        return kSpace;
    }
    // Letters travel as their upper case code.
    if (keysym >= 'a' && keysym <= 'z')
        return int(keysym - 'a' + 'A');
    if (keysym < 0x80)
        return int(keysym);
    return 0;
}

uint32_t fromWireKey(int key)
{
    using namespace Lingmo::ui;
    switch (key) {
    case kEscape:
        return key::Escape;
    case kLeft:
        return key::Left;
    case kUp:
        return key::Up;
    case kRight:
        return key::Right;
    case kDown:
        return key::Down;
    }
    if (key >= 'A' && key <= 'Z')
        return uint32_t(key - 'A' + 'a');
    if (key > 0 && key < 0x80)
        return uint32_t(key);
    return 0;
}

int toWireModifiers(uint32_t modifiers)
{
    using namespace Lingmo::ui;
    return (modifiers & ShiftModifier ? kShift : 0) | (modifiers & ControlModifier ? kControl : 0)
            | (modifiers & AltModifier ? kAlt : 0) | (modifiers & SuperModifier ? kMeta : 0);
}

uint32_t fromWireModifiers(int modifiers)
{
    using namespace Lingmo::ui;
    return (modifiers & kShift ? ShiftModifier : 0) | (modifiers & kControl ? ControlModifier : 0)
            | (modifiers & kAlt ? AltModifier : 0) | (modifiers & kMeta ? SuperModifier : 0);
}

} // namespace fm::previewkeys
