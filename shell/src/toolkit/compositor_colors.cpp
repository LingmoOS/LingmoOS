// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "toolkit/compositor_colors.h"

#include "ui/theme.h"

namespace Lingmo::Toolkit {

namespace {

constexpr ui::Color kWhite = ui::Color::rgb(0xFFFFFF);
constexpr ui::Color kBlack = ui::Color::rgb(0x000000);

} // namespace

void addCompositorColors(ui::ColorMixer &mixer, const ui::Theme &theme)
{
    const bool dark = theme.darkMode;

    mixer[CompositorColor::SwitcherName] = dark ? kWhite.withAlpha(0.92) : kBlack.withAlpha(0.85);

    // Opaque: the dock sets the alpha.
    mixer[CompositorColor::DockDivider] = dark ? kWhite : kBlack;

    mixer[CompositorColor::LevelTrackBackground] = kBlack.withAlpha(dark ? 0.2 : 0.04);
    mixer[CompositorColor::LevelTrackOutline] = kBlack.withAlpha(dark ? 0.3 : 0.08);
    mixer[CompositorColor::LevelTrackFill] = dark ? ui::Color::rgb(0x6C6C72) : kWhite;
    mixer[CompositorColor::LevelTrackKnob] = dark ? ui::Color::rgb(0x7C7C82) : kWhite;
    mixer[CompositorColor::LevelTrackKnobShadow] = kBlack.withAlpha(dark ? 0.35 : 0.18);

    mixer[CompositorColor::ClipboardSurface] = dark ? ui::Color::rgb(0x242529) : ui::Color::rgb(0xFAFAFC);
    mixer[CompositorColor::ClipboardHairline] = (dark ? kWhite : kBlack).withAlpha(0.1);

    mixer[CompositorColor::NotificationSecondaryText] = dark ? kWhite.withAlpha(0.55) : kBlack.withAlpha(0.5);
    mixer[CompositorColor::NotificationBannerSurface] = dark ? ui::Color::rgb(0x4D4D4D).withAlpha(0.65)
                                                             : ui::Color::rgb(0xF0F0F0).withAlpha(0.82);
    mixer[CompositorColor::NotificationCentreCard] = kWhite.withAlpha(dark ? 0.1 : 0.8);
    mixer[CompositorColor::NotificationCentreEmptyText] = dark ? kWhite.withAlpha(0.5) : kBlack.withAlpha(0.45);
}

} // namespace Lingmo::Toolkit
