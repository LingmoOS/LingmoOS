// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <string>
#include <string_view>
#include <vector>

#include "ui/geometry.h"

namespace fm {

// The widgets the desktop can hold. A widget's kind picks its content and the
// size its card; the placement is the card's top-left in the widget layer.
enum class WidgetKind {
    Clock = 0,
    Calendar,
    System,
    Battery,
    NowPlaying,
    Notes,
    Weather,
    WorldClock,
    Countdown,
    NetworkSpeed,
    Todo,
    Launcher,
};

enum class WidgetSize {
    Small = 0,
    Medium,
    Large,
};

struct WidgetPlacement
{
    WidgetKind kind = WidgetKind::Clock;
    WidgetSize size = WidgetSize::Medium;
    double x = 0;
    double y = 0;
    // Stable per-card identity; empty in layouts saved before widgets had one.
    std::string id;
};

// Between two cards, and the corner of a card.
inline constexpr double kWidgetGap = 16;
inline constexpr double kWidgetRadius = 24;

// The keys the desktop's JSON stores; unknown ones are dropped when read.
const char* widgetKindKey(WidgetKind kind);
const char* widgetSizeKey(WidgetSize size);
bool widgetKindFromKey(std::string_view key, WidgetKind& kind);
bool widgetSizeFromKey(std::string_view key, WidgetSize& size);

// The card's size in logical pixels. Medium is a wide card, Large a square.
ui::SizeF widgetSizeSpec(WidgetSize size);

// A JSON array of placements, as the desktop's settings store it.
std::string serializeWidgets(const std::vector<WidgetPlacement>& placements);
// Every placement in the array that names a known kind and size; empty when
// the text is not a JSON array.
std::vector<WidgetPlacement> parseWidgets(std::string_view json);

} // namespace fm
