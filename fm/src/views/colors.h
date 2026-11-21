// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <cstdint>

#include "ui/color_provider.h"

namespace fm {

// The file manager's own colours, after the toolkit's; colors.cpp gives
// them for each theme.
enum class FmColor : uint16_t {
    ChooserSidebarBackground = uint16_t(Lingmo::ui::ColorId::ApplicationFirst),
    AlternateRow,
    RubberBandFill,
    RubberBandBorder,
    PathEditorText,
    PathEditorBackground,
    SearchScopeSelected,
};

void addFmColors(Lingmo::ui::ColorMixer& mixer, const Lingmo::ui::Theme& theme);

// Added before main by every program that names one of the colours.
[[maybe_unused]] inline const bool fmColorsAdded = (Lingmo::ui::ColorProvider::addMixer(addFmColors), true);

} // namespace fm
