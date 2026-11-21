// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "decoration/window_visual_style.h"

#include <algorithm>
#include <iterator>

namespace Lingmo::Decoration {

namespace {

constexpr int kRadius = 14; // Logical pixels, all four corners.
constexpr double kOutlineWidthPx = 1.0;

constexpr render::Color kOutlineColor {0, 0, 0, 0.05f};
// Drawn just inside the outline; only shows on dark content.
constexpr render::Color kHighlightColor {1, 1, 1, 0.15f};
// Dark content needs a real dark edge to separate from the shadow, and a
// brighter rim inside it so the window still reads as raised.
constexpr render::Color kDarkOutlineColor {0, 0, 0, 0.7f};
constexpr render::Color kDarkHighlightColor {1, 1, 1, 0.2f};
constexpr render::Color kShellOutlineColor {0, 0, 0, 0.14f};
// The dock floats over the wallpaper, so it leans on the highlight instead.
constexpr render::Color kDockOutlineColor {0, 0, 0, 0.1f};
constexpr render::Color kDockHighlightColor {1, 1, 1, 0.2f};
constexpr render::Color kDarkDockOutlineColor {0, 0, 0, 0.3f};
constexpr render::Color kDarkDockHighlightColor {1, 1, 1, 0.2f};
// Light enough that the blocked window still reads; ash's 0.5 is for whole
// containers behind system modals.
constexpr float kDimOpacity = 0.15f;
constexpr float kDarkDimOpacity = 0.25f;

// FishUI's ThemeManager accent palette, in the Appearance service's index order.
constexpr render::Color kAccentColors[] = {
    {51 / 255.f, 133 / 255.f, 255 / 255.f, 1}, // Blue
    {255 / 255.f, 92 / 255.f, 109 / 255.f, 1}, // Red
    {53 / 255.f, 191 / 255.f, 86 / 255.f, 1}, // Green
    {130 / 255.f, 102 / 255.f, 255 / 255.f, 1}, // Purple
    {202 / 255.f, 100 / 255.f, 172 / 255.f, 1}, // Pink
    {254 / 255.f, 160 / 255.f, 66 / 255.f, 1}, // Orange
    {79 / 255.f, 89 / 255.f, 107 / 255.f, 1}, // Grey
};

// Ash's phantom window: kColorAshPhantomWindowBackgroundColor is the primary
// colour at kAlpha25, edged by views::HighlightBorder's kHighlightBorderNoShadow
// (cros.sys.system-highlight-border outside, cros.sys.system-highlight inside).
constexpr float kSnapPreviewFillAlpha = 0x40 / 255.f;
constexpr render::Color kSnapPreviewOutline {0, 0, 0, 0x14 / 255.f};
constexpr render::Color kSnapPreviewHighlight {1, 1, 1, 0x28 / 255.f};
constexpr render::Color kDarkSnapPreviewHighlight {1, 1, 1, 0x0F / 255.f};

render::Color accentColor(int index)
{
    if (index < 0 || index >= int(std::size(kAccentColors)))
        index = 0;
    return kAccentColors[index];
}

} // namespace

WindowVisualStyle::WindowVisualStyle(bool dark, int accentIndex)
    : radius(kRadius)
    , outlineWidthPx(kOutlineWidthPx)
    , activeOutline(dark ? kDarkOutlineColor : kOutlineColor)
    , inactiveOutline(dark ? kDarkOutlineColor : kOutlineColor)
    , highlight(dark ? kDarkHighlightColor : kHighlightColor)
    , shellOutline(dark ? kDarkOutlineColor : kShellOutlineColor)
    , dockOutline(dark ? kDarkDockOutlineColor : kDockOutlineColor)
    , dockHighlight(dark ? kDarkDockHighlightColor : kDockHighlightColor)
    , dimOpacity(dark ? kDarkDimOpacity : kDimOpacity)
    , snapPreviewOutline(kSnapPreviewOutline)
    , snapPreviewHighlight(dark ? kDarkSnapPreviewHighlight : kSnapPreviewHighlight)
{
    snapPreviewFill = accentColor(accentIndex);
    snapPreviewFill.a = kSnapPreviewFillAlpha;
}

int WindowVisualStyle::radiusFor(const base::Size &size)
{
    return std::max(0, std::min({kRadius, size.width() / 2, size.height() / 2}));
}

} // namespace Lingmo::Decoration
