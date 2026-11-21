// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <cstdint>

// The xkb keysyms KeyEvent::key carries, for the keys controls handle.
// Letters and digits are their ASCII codes, lower case for letters.
namespace Lingmo::ui::key {

inline constexpr uint32_t Space = 0x0020;
inline constexpr uint32_t BackSpace = 0xff08;
inline constexpr uint32_t Tab = 0xff09;
inline constexpr uint32_t Return = 0xff0d;
inline constexpr uint32_t Escape = 0xff1b;
inline constexpr uint32_t Home = 0xff50;
inline constexpr uint32_t Left = 0xff51;
inline constexpr uint32_t Up = 0xff52;
inline constexpr uint32_t Right = 0xff53;
inline constexpr uint32_t Down = 0xff54;
inline constexpr uint32_t PageUp = 0xff55;
inline constexpr uint32_t PageDown = 0xff56;
inline constexpr uint32_t End = 0xff57;
inline constexpr uint32_t Insert = 0xff63;
inline constexpr uint32_t Menu = 0xff67;
inline constexpr uint32_t KeypadEnter = 0xff8d;
inline constexpr uint32_t F1 = 0xffbe;
inline constexpr uint32_t F4 = 0xffc1;
inline constexpr uint32_t Delete = 0xffff;

constexpr uint32_t letter(char c)
{
    return uint32_t(c >= 'A' && c <= 'Z' ? c - 'A' + 'a' : c);
}

} // namespace Lingmo::ui::key
