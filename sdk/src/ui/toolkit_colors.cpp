// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ui/toolkit_colors.h"

#include "ui/theme.h"

namespace Lingmo::ui {

static constexpr Color kWhite = Color::rgb(0xFFFFFF);
static constexpr Color kBlack = Color::rgb(0x000000);
static constexpr Color kTransparent = Color{0, 0, 0, 0};

// Classic style – the existing colour recipes, unchanged.
static void addClassicColors(ColorMixer& mixer, const Theme& theme)
{
    const bool dark = theme.darkMode;

    mixer[ColorId::Background] = theme.background;
    mixer[ColorId::SecondBackground] = theme.secondBackground;
    mixer[ColorId::AlternateBackground] = theme.alternateBackground;
    mixer[ColorId::Text] = theme.text;
    mixer[ColorId::DisabledText] = theme.disabledText;
    mixer[ColorId::Highlight] = theme.highlight;
    mixer[ColorId::HighlightedText] = theme.highlightedText;

    mixer[ColorId::DestructiveAction] = dark ? Color::rgb(0xFF453A) : Color::rgb(0xD92D20);
    mixer[ColorId::Separator] = withAlpha(ColorId::DisabledText, dark ? 0.3 : 0.1);
    mixer[ColorId::HeaderSeparator] = withAlpha(ColorId::Text, dark ? 0.12 : 0.08);
    mixer[ColorId::ItemPressed] = withAlpha(ColorId::Text, dark ? 0.05 : 0.1);
    mixer[ColorId::TranslucentWindowBackground] = withAlpha(ColorId::Background, dark ? 0.85 : 0.8);
    mixer[ColorId::TranslucentWindowBackgroundInactive] = withAlpha(ColorId::Background, dark ? 0.92 : 0.9);

    // Classic mode: use pure white for buttons so they stand out on the
    // window background (#F2F4F5 → #FFFFFF contrast).  A 1px border gives
    // the edge definition that the fill alone cannot supply.
    mixer[ColorId::ButtonBackground] = dark ? mixer[ColorId::AlternateBackground] : kWhite;
    mixer[ColorId::ButtonBorder]     = dark ? kTransparent : kBlack.withAlpha(0.14);
    mixer[ColorId::ButtonBackgroundHovered] = dark ? lighter(ColorId::ButtonBackground, 1.2)
                                                   : Color::rgb(0xF5F5F7);
    mixer[ColorId::ButtonBackgroundPressed] = dark ? lighter(ColorId::ButtonBackground, 1.1)
                                                   : Color::rgb(0xE8E8EA);
    mixer[ColorId::ButtonFocusRing] = withAlpha(ColorId::Highlight, 0.5);

    mixer[ColorId::ComboBoxBackground] = ColorId::AlternateBackground;
    mixer[ColorId::ComboBoxBackgroundPressed] = dark ? lighter(ColorId::ComboBoxBackground, 1.1)
                                                     : darker(ColorId::ComboBoxBackground, 1.2);
    mixer[ColorId::ComboBoxFocusBorder] = ColorId::Highlight;

    mixer[ColorId::ToggleBackground] = ColorId::SecondBackground;
    mixer[ColorId::ToggleBackgroundChecked] = ColorId::Highlight;
    mixer[ColorId::ToggleBorder] = withAlpha(ColorId::Text, dark ? 0.28 : 0.2);
    mixer[ColorId::ToggleBorderChecked] = darker(ColorId::Highlight, 1.12);
    mixer[ColorId::ToggleFocusRing] = withAlpha(ColorId::Highlight, 0.35);
    mixer[ColorId::CheckBoxInnerEdge] = kWhite.withAlpha(0.06);
    mixer[ColorId::CheckBoxInnerEdgeChecked] = kWhite.withAlpha(0.18);
    mixer[ColorId::ToggleMark] = ColorId::HighlightedText;

    mixer[ColorId::SwitchTrack] = withAlpha(ColorId::Text, 0.2);
    mixer[ColorId::SwitchTrackOn] = ColorId::Highlight;
    mixer[ColorId::SwitchHandle] = kWhite;

    mixer[ColorId::SliderTrack] = dark ? kWhite.withAlpha(0.3) : kBlack.withAlpha(0.1);
    mixer[ColorId::SliderTrackFilled] = ColorId::Highlight;
    mixer[ColorId::SliderTickMark] = dark ? kWhite.withAlpha(0.55) : kBlack.withAlpha(0.35);
    mixer[ColorId::SliderHandle] = kWhite;
    mixer[ColorId::SliderHandlePressed] = Color::rgb(0xFAFAFA);
    mixer[ColorId::SliderHandleBorder] = kBlack.withAlpha(0.04);
    mixer[ColorId::SliderHandleShadow] = kBlack.withAlpha(0.1);
    mixer[ColorId::SliderHandleShadowPressed] = kBlack.withAlpha(0.15);

    mixer[ColorId::SegmentedControlTrack] = dark ? kWhite.withAlpha(0.06) : kBlack.withAlpha(0.05);
    mixer[ColorId::SegmentedControlPlate] = dark ? lighter(ColorId::SecondBackground, 2)
                                                 : kWhite;
    mixer[ColorId::SegmentedControlPlateShadow] = kBlack.withAlpha(dark ? 0.18 : 0.12);
    mixer[ColorId::SegmentedControlTextPressed] = withAlpha(ColorId::Text, 0.5);
    mixer[ColorId::SegmentedTabBackground] = ColorId::AlternateBackground;
    mixer[ColorId::SegmentedTabPlateShadow] = dark ? kBlack.withAlpha(0.11) : kTransparent;

    mixer[ColorId::ScrollBarThumb] = dark ? kWhite.withAlpha(0.5) : kBlack.withAlpha(0.4);
    mixer[ColorId::ScrollBarThumbDragging] = dark ? kWhite.withAlpha(0.4) : kBlack.withAlpha(0.5);

    mixer[ColorId::TextFieldPlaceholder] = withAlpha(ColorId::Text, 0.4);
    mixer[ColorId::TextFieldFocusRing] = withAlpha(ColorId::Highlight, 0.25);

    mixer[ColorId::RoundButtonHovered] = withAlpha(ColorId::Text, dark ? 0.12 : 0.11);
    mixer[ColorId::RoundButtonPressed] = withAlpha(ColorId::Text, dark ? 0.06 : 0.21);

    mixer[ColorId::HeaderButtonTint] = dark ? kWhite : Color::rgb(0x323238);

    mixer[ColorId::DialogButtonBackground] = ColorId::AlternateBackground;
    mixer[ColorId::DialogButtonBackgroundHovered] = dark ? lighter(ColorId::DialogButtonBackground, 1.05)
                                                         : darker(ColorId::DialogButtonBackground, 1.04);
    mixer[ColorId::DialogButtonBorder] = dark ? Color::rgb(0x545455) : Color::rgb(0xD9DADD);
    mixer[ColorId::DialogButtonInnerEdge] = dark ? kTransparent : kWhite.withAlpha(0.08);
    mixer[ColorId::DialogButtonProminentText] = kWhite;
    mixer[ColorId::DialogButtonProminentInnerEdge] = kWhite.withAlpha(0.2);
    mixer[ColorId::DialogButtonFocusRing] = withAlpha(ColorId::Highlight, 0.3);

    mixer[ColorId::MenuHighlight] = withAlpha(ColorId::Highlight, dark ? 0.82 : 0.9);
    mixer[ColorId::MenuSeparator] = dark ? kWhite.withAlpha(0.14) : kBlack.withAlpha(0.1);
    mixer[ColorId::MenuDisabledText] = withAlpha(ColorId::Text, 0.4);
    mixer[ColorId::MenuShortcutText] = withAlpha(ColorId::Text, 0.5);
    mixer[ColorId::MenuArrow] = dark ? ColorId::Text : ColorId::DisabledText;
    mixer[ColorId::MenuArrowDisabled] = dark ? ColorId::MenuDisabledText : ColorId::DisabledText;
    mixer[ColorId::MenuScrollIndicator] = dark ? kWhite.withAlpha(0.35) : kBlack.withAlpha(0.25);

    mixer[ColorId::WindowTabBarTrack] = dark ? kWhite.withAlpha(0.06) : kBlack.withAlpha(0.045);
    mixer[ColorId::WindowTabBarSelected] = kWhite.withAlpha(dark ? 0.13 : 0.8);
    mixer[ColorId::WindowTabBarSelectedEdge] = dark ? kTransparent : Color::fromArgb(0x0F000000);

    mixer[ColorId::ToolTipBackground] = withAlpha(ColorId::SecondBackground, 0.95);
    mixer[ColorId::ToolTipShadow] = kBlack.withAlpha(0.15);
    mixer[ColorId::ToastShadow] = kBlack.withAlpha(0.14);
    mixer[ColorId::AboutDialogBackground] = withAlpha(ColorId::Background, 0.8);
    // RowComboBox: subtle fill on hover/open, no fill at rest.
    mixer[ColorId::RowComboBoxHovered] = withAlpha(ColorId::Text, dark ? 0.08 : 0.06);
    mixer[ColorId::RowComboBoxOpen]    = withAlpha(ColorId::Text, dark ? 0.12 : 0.10);
}

// Liquid Glass style – frosted-glass material colours.
//
// Design principles:
//   • Controls are mostly transparent; they let the blurred backdrop show
//     through rather than painting a solid fill.
//   • A thin white/near-white edge highlight on the top/leading edge and a
//     subtler shadow on the bottom/trailing edge give the impression of a
//     convex glass surface.
//   • Pressed and hover states shift the glass opacity and add a specular flash,
//     not a colour change.
//   • Text always reads: white on dark glass, near-black on light glass.
//   • Accent colour is used only for filled elements (switch-on track, sliders,
//     checkboxes when checked) where identification of state matters most.
static void addLiquidGlassColors(ColorMixer& mixer, const Theme& theme)
{
    const bool dark = theme.darkMode;

    // Semantic colours are the same as Classic (they come from the user's accent
    // and system palette), only control-surface colours change.
    mixer[ColorId::Background] = theme.background;
    mixer[ColorId::SecondBackground] = theme.secondBackground;
    mixer[ColorId::AlternateBackground] = theme.alternateBackground;
    mixer[ColorId::Text] = theme.text;
    mixer[ColorId::DisabledText] = theme.disabledText;
    mixer[ColorId::Highlight] = theme.highlight;
    mixer[ColorId::HighlightedText] = theme.highlightedText;

    mixer[ColorId::DestructiveAction] = dark ? Color::rgb(0xFF453A) : Color::rgb(0xD92D20);
    mixer[ColorId::Separator] = dark ? kWhite.withAlpha(0.1) : kBlack.withAlpha(0.07);
    mixer[ColorId::HeaderSeparator] = dark ? kWhite.withAlpha(0.08) : kBlack.withAlpha(0.06);
    mixer[ColorId::ItemPressed] = dark ? kWhite.withAlpha(0.08) : kBlack.withAlpha(0.06);
    // Window fills: a subtly translucent glass surface.
    mixer[ColorId::TranslucentWindowBackground] = dark ? Color::fromArgb(0xCC1C1C1E)
                                                       : Color::fromArgb(0xCCF2F4F6);
    mixer[ColorId::TranslucentWindowBackgroundInactive] = dark ? Color::fromArgb(0xDD1C1C1E)
                                                               : Color::fromArgb(0xDDF2F4F6);

    // --- Buttons ---
    // Light mode: very subtle dark tint (3%) — clearly visible via shadow+rim,
    // not grey enough to look "coloured" on a white background.
    // Liquid Glass: ButtonBorder not used (shadow+rim provide the edge).
    mixer[ColorId::ButtonBackground] = dark ? kWhite.withAlpha(0.10) : kBlack.withAlpha(0.03);
    mixer[ColorId::ButtonBorder] = kTransparent;
    mixer[ColorId::ButtonBackgroundHovered] = dark ? kWhite.withAlpha(0.20) : kBlack.withAlpha(0.06);
    mixer[ColorId::ButtonBackgroundPressed] = dark ? kWhite.withAlpha(0.08) : kBlack.withAlpha(0.09);
    mixer[ColorId::ButtonFocusRing] = withAlpha(ColorId::Highlight, 0.6);

    // --- ComboBox (Dropdown) ---
    mixer[ColorId::ComboBoxBackground] = dark ? kWhite.withAlpha(0.12) : kBlack.withAlpha(0.03);
    mixer[ColorId::ComboBoxBackgroundPressed] = dark ? kWhite.withAlpha(0.08) : kBlack.withAlpha(0.09);
    mixer[ColorId::ComboBoxFocusBorder] = ColorId::Highlight;

    // --- Checkbox / Radio ---
    mixer[ColorId::ToggleBackground] = dark ? kWhite.withAlpha(0.14) : kBlack.withAlpha(0.04);
    mixer[ColorId::ToggleBackgroundChecked] = ColorId::Highlight;
    mixer[ColorId::ToggleBorder] = dark ? kWhite.withAlpha(0.30) : kBlack.withAlpha(0.14);
    mixer[ColorId::ToggleBorderChecked] = darker(ColorId::Highlight, 1.1);
    mixer[ColorId::ToggleFocusRing] = withAlpha(ColorId::Highlight, 0.45);
    // Inner edge highlight simulates the top-lit rim of a glass surface.
    mixer[ColorId::CheckBoxInnerEdge] = kWhite.withAlpha(0.14);
    mixer[ColorId::CheckBoxInnerEdgeChecked] = kWhite.withAlpha(0.28);
    mixer[ColorId::ToggleMark] = kWhite;

    // --- Switch ---
    // Track: glassy pill, clearly distinct off/on.
    mixer[ColorId::SwitchTrack] = dark ? kWhite.withAlpha(0.18) : kBlack.withAlpha(0.10);
    mixer[ColorId::SwitchTrackOn] = ColorId::Highlight;
    // Handle: a bright glass bead with a soft shadow.
    mixer[ColorId::SwitchHandle] = kWhite;

    // --- Slider ---
    // Track: a thin glass groove.
    mixer[ColorId::SliderTrack] = dark ? kWhite.withAlpha(0.22) : kBlack.withAlpha(0.09);
    mixer[ColorId::SliderTrackFilled] = ColorId::Highlight;
    mixer[ColorId::SliderTickMark] = dark ? kWhite.withAlpha(0.45) : kBlack.withAlpha(0.28);
    // Handle: glossy white bead.
    mixer[ColorId::SliderHandle] = kWhite;
    mixer[ColorId::SliderHandlePressed] = Color::rgb(0xF4F4F4);
    mixer[ColorId::SliderHandleBorder] = kBlack.withAlpha(0.06);
    mixer[ColorId::SliderHandleShadow] = kBlack.withAlpha(0.14);
    mixer[ColorId::SliderHandleShadowPressed] = kBlack.withAlpha(0.20);

    // --- Segmented Control ---
    // Track: a subtle glass tray, slightly more visible in light mode.
    mixer[ColorId::SegmentedControlTrack] = dark ? kWhite.withAlpha(0.09) : kBlack.withAlpha(0.04);
    // Selected plate: a bright glass pill inside the tray.
    mixer[ColorId::SegmentedControlPlate] = dark ? kWhite.withAlpha(0.18) : kWhite.withAlpha(0.80);
    mixer[ColorId::SegmentedControlPlateShadow] = kBlack.withAlpha(dark ? 0.22 : 0.10);
    mixer[ColorId::SegmentedControlTextPressed] = withAlpha(ColorId::Text, 0.45);
    mixer[ColorId::SegmentedTabBackground] = dark ? kWhite.withAlpha(0.07) : kWhite.withAlpha(0.50);
    mixer[ColorId::SegmentedTabPlateShadow] = dark ? kBlack.withAlpha(0.14) : kTransparent;

    // --- Scroll bars ---
    mixer[ColorId::ScrollBarThumb] = dark ? kWhite.withAlpha(0.42) : kBlack.withAlpha(0.32);
    mixer[ColorId::ScrollBarThumbDragging] = dark ? kWhite.withAlpha(0.56) : kBlack.withAlpha(0.44);

    // --- Text Field ---
    mixer[ColorId::TextFieldPlaceholder] = withAlpha(ColorId::Text, 0.38);
    mixer[ColorId::TextFieldFocusRing] = withAlpha(ColorId::Highlight, 0.35);

    // --- Round (window) buttons ---
    mixer[ColorId::RoundButtonHovered] = dark ? kWhite.withAlpha(0.16) : kWhite.withAlpha(0.70);
    mixer[ColorId::RoundButtonPressed] = dark ? kWhite.withAlpha(0.08) : kWhite.withAlpha(0.50);

    mixer[ColorId::HeaderButtonTint] = dark ? kWhite : Color::rgb(0x2A2A2F);

    // --- Dialog buttons ---
    mixer[ColorId::DialogButtonBackground] = dark ? kWhite.withAlpha(0.11) : kBlack.withAlpha(0.03);
    mixer[ColorId::DialogButtonBackgroundHovered] = dark ? kWhite.withAlpha(0.19) : kBlack.withAlpha(0.06);
    // The edge highlight is the glass rim; replaces the flat border.
    mixer[ColorId::DialogButtonBorder] = dark ? kWhite.withAlpha(0.22) : kBlack.withAlpha(0.08);
    mixer[ColorId::DialogButtonInnerEdge] = kWhite.withAlpha(dark ? 0.14 : 0.20);
    mixer[ColorId::DialogButtonProminentText] = kWhite;
    mixer[ColorId::DialogButtonProminentInnerEdge] = kWhite.withAlpha(0.28);
    mixer[ColorId::DialogButtonFocusRing] = withAlpha(ColorId::Highlight, 0.40);

    // --- Menus ---
    // Menu items: a highlight pill that feels like a glass selection.
    mixer[ColorId::MenuHighlight] = withAlpha(ColorId::Highlight, dark ? 0.80 : 0.88);
    mixer[ColorId::MenuSeparator] = dark ? kWhite.withAlpha(0.11) : kBlack.withAlpha(0.08);
    mixer[ColorId::MenuDisabledText] = withAlpha(ColorId::Text, 0.36);
    mixer[ColorId::MenuShortcutText] = withAlpha(ColorId::Text, 0.46);
    mixer[ColorId::MenuArrow] = dark ? ColorId::Text : ColorId::DisabledText;
    mixer[ColorId::MenuArrowDisabled] = dark ? ColorId::MenuDisabledText : ColorId::DisabledText;
    mixer[ColorId::MenuScrollIndicator] = dark ? kWhite.withAlpha(0.32) : kBlack.withAlpha(0.22);

    // --- Window tab bar ---
    mixer[ColorId::WindowTabBarTrack] = dark ? kWhite.withAlpha(0.08) : kBlack.withAlpha(0.04);
    mixer[ColorId::WindowTabBarSelected] = dark ? kWhite.withAlpha(0.17) : kWhite.withAlpha(0.84);
    mixer[ColorId::WindowTabBarSelectedEdge] = dark ? kWhite.withAlpha(0.06) : Color::fromArgb(0x0F000000);

    // --- Tooltips ---
    // A frosted glass tooltip with a barely opaque tinted background.
    mixer[ColorId::ToolTipBackground] = dark ? Color::fromArgb(0xDD2C2C2E) : Color::fromArgb(0xDDFFFFFF);
    mixer[ColorId::ToolTipShadow] = kBlack.withAlpha(0.18);
    mixer[ColorId::ToastShadow] = kBlack.withAlpha(0.16);
    mixer[ColorId::AboutDialogBackground] = dark ? Color::fromArgb(0xBB1C1C1E)
                                                 : Color::fromArgb(0xBBF5F6FA);
    // RowComboBox: subtle fill on hover/open, no fill at rest.
    mixer[ColorId::RowComboBoxHovered] = withAlpha(ColorId::Text, dark ? 0.08 : 0.06);
    mixer[ColorId::RowComboBoxOpen]    = withAlpha(ColorId::Text, dark ? 0.12 : 0.10);
}

void addToolkitColors(ColorMixer& mixer, const Theme& theme)
{
    if (theme.controlStyle == 1)
        addLiquidGlassColors(mixer, theme);
    else
        addClassicColors(mixer, theme);
}

} // namespace Lingmo::ui
