// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include "base/geometry.h"

#include "render/renderer.h"

namespace Lingmo::Decoration {

// Interface only: every tunable lives in window_visual_style.cpp so that
// changing one relinks instead of rebuilding the tree.
struct WindowVisualStyle {
    // Dark mode deepens the outer outline and brightens the inner highlight.
    // `accentIndex` is the Appearance service's AccentColor setting.
    explicit WindowVisualStyle(bool dark = false, int accentIndex = 0);

    // Clamped so the corners never overlap on a small window.
    static int radiusFor(const base::Size &size);

    int radius; // Logical pixels, all four corners.
    double outlineWidthPx;
    render::Color activeOutline;
    render::Color inactiveOutline;
    render::Color highlight;
    // Popups, menus and panels have no titlebar to set them apart.
    render::Color shellOutline;
    render::Color dockOutline;
    render::Color dockHighlight;
    // Black laid over a window while a modal transient blocks it.
    float dimOpacity;
    // Where a dragged window will snap or maximize: an accent-tinted plate
    // edged by a dark hairline with a light one just inside it.
    render::Color snapPreviewFill;
    render::Color snapPreviewOutline;
    render::Color snapPreviewHighlight;
};

} // namespace Lingmo::Decoration
