// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "core/common.h"

#include "ui/geometry.h"

#include <cstdint>
#include <string>

// FishUI values the file manager uses, in one place.
namespace fm::style {

// FishUI.Units.fontMetrics.height: the theme font's line height.
double fontHeight();
// The theme font's advance width of the text, as FontMetrics.advanceWidth.
double advanceWidth(const std::string& text);

// Text drawn on the highlight colour: dark on a light accent.
ui::Color selectedTextColor();
ui::Color withAlpha(ui::Color color, double alpha);
// The theme's text colour at the given alpha.
ui::Color textAlpha(double alpha);

inline constexpr ui::Color kDanger = ui::Color::rgb(0xd94b4b);

// Keysyms ui/keys.h leaves out.
namespace key {
inline constexpr uint32_t F2 = 0xffbf;
inline constexpr uint32_t F5 = 0xffc2;
inline constexpr uint32_t ShiftL = 0xffe1;
inline constexpr uint32_t ShiftR = 0xffe2;
inline constexpr uint32_t ControlL = 0xffe3;
inline constexpr uint32_t ControlR = 0xffe4;
inline constexpr uint32_t Backtab = 0xfe20;
inline constexpr uint32_t Back = 0x1008ff26;
inline constexpr uint32_t Forward = 0x1008ff27;
} // namespace key

// A cubic bezier easing through (x1, y1) and (x2, y2), at progress t.
double bezier(double x1, double y1, double x2, double y2, double t);

} // namespace fm::style
