// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "widgets/colors.h"

#include "ui/theme.h"

using namespace Lingmo::ui;

namespace settings {

static constexpr Color kWhite = Color::rgb(0xFFFFFF);
static constexpr Color kBlack = Color::rgb(0x000000);

void addSettingsColors(ColorMixer& mixer, const Theme& theme)
{
    const bool dark = theme.darkMode;

    mixer[SettingsColor::SidebarBackground] =
        withAlpha(dark ? lighter(ColorId::Background, 1.5) : darker(ColorId::Background, 1.05), 0.7);
    mixer[SettingsColor::SidebarItemCurrent] = withAlpha(ColorId::Highlight, 1.0);
    mixer[SettingsColor::SidebarIconTile] = dark ? Color::rgb(0x596273) : Color::rgb(0x8B95A7);
    mixer[SettingsColor::SidebarItemHovered] = withAlpha(ColorId::Text, dark ? 0.065 : 0.05);
    // A flat panel over the sidebar; the shadow gives it depth without a
    // border or a gradient.
    mixer[SettingsColor::SidebarPanelFill] =
        withAlpha(dark ? lighter(ColorId::SecondBackground, 1.35) : lighter(ColorId::SecondBackground, 1.03),
                  dark ? 0.72 : 0.62);
    mixer[SettingsColor::SidebarPanelShadow] = kBlack.withAlpha(dark ? 0.42 : 0.1);

    mixer[SettingsColor::RowHighlight] = withAlpha(ColorId::Text, dark ? 0.05 : 0.15);
    mixer[SettingsColor::RowComboBoxHovered] = withAlpha(ColorId::Text, dark ? 0.08 : 0.06);
    mixer[SettingsColor::RowComboBoxOpen] = withAlpha(ColorId::Text, dark ? 0.12 : 0.1);
    mixer[SettingsColor::StandardButtonBackground] = dark ? Color::rgb(0x363636) : kWhite;
    mixer[SettingsColor::KeyCap] = dark ? lighter(ColorId::SecondBackground, 1.6)
                                        : darker(ColorId::SecondBackground, 1.08);
    mixer[SettingsColor::PreferredLanguageTag] = withAlpha(ColorId::Highlight, dark ? 0.2 : 0.12);
    mixer[SettingsColor::WallpaperPlaceholder] = dark ? kWhite : Color::rgb(0xC8CAD5);

    mixer[SettingsColor::TimeZoneMapPill] = dark ? kBlack.withAlpha(0.6) : kWhite.withAlpha(0.9);

    mixer[SettingsColor::BatteryChipBackground] = dark ? ColorId::AlternateBackground : ColorId::Background;
    mixer[SettingsColor::BatteryChipBorder] = withAlpha(ColorId::Text, dark ? 0.1 : 0.08);
    mixer[SettingsColor::GraphGrid] = dark ? kWhite.withAlpha(0.08) : kBlack.withAlpha(0.06);
    mixer[SettingsColor::GraphBaseline] = dark ? kWhite.withAlpha(0.18) : kBlack.withAlpha(0.14);
    mixer[SettingsColor::GraphProbe] = dark ? kWhite.withAlpha(0.28) : kBlack.withAlpha(0.2);
}

} // namespace settings
