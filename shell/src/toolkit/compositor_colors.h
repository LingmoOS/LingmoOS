// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef LINGMO_TOOLKIT_COMPOSITOR_COLORS_H_
#define LINGMO_TOOLKIT_COMPOSITOR_COLORS_H_

#include <cstdint>

#include "ui/color_provider.h"

namespace Lingmo::Toolkit {

// The colours of the panels the compositor draws, after the toolkit's;
// compositor_colors.cpp gives them for each theme.
enum class CompositorColor : uint16_t {
    SwitcherName = uint16_t(ui::ColorId::ApplicationFirst),

    DockDivider,

    LevelTrackBackground,
    LevelTrackOutline,
    LevelTrackFill,
    LevelTrackKnob,
    LevelTrackKnobShadow,

    ClipboardSurface,
    ClipboardHairline,

    NotificationSecondaryText,
    NotificationBannerSurface,
    NotificationCentreCard,
    NotificationCentreEmptyText,
};

void addCompositorColors(ui::ColorMixer &mixer, const ui::Theme &theme);

// Added before main by every program that names one of the colours.
[[maybe_unused]] inline const bool compositorColorsAdded = (ui::ColorProvider::addMixer(addCompositorColors), true);

} // namespace Lingmo::Toolkit

#endif // LINGMO_TOOLKIT_COMPOSITOR_COLORS_H_
