// Copyright 2012 The Chromium Authors
// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.chromium file.
//
// Upstream: chromium/src/ui/gfx/shadow_value.cc
//           chromium/src/ui/compositor_extra/{shadow,decoration_util}.cc
// Revision: 7acd40ddefff49c9d36144999aafdbb674dbd07a
//
// What is taken: the shadow geometry and cache concepts.

#ifndef LINGMO_DECORATION_SHADOW_H_
#define LINGMO_DECORATION_SHADOW_H_

#include <array>
#include <memory>
#include <string>

#include "base/geometry.h"
#include "decoration/raster.h"
#include "ui/path.h"

namespace Lingmo::Decoration {

struct ShadowLayer {
    float sigma = 0; // Gaussian standard deviation, logical px, NOT Chromium blur().
    base::Point offset; // Logical px, positive down/right.
    float opacity = 0;
};

using ShadowStyle = std::array<ShadowLayer, 3>;

// Raster geometry for a supplied silhouette; window ownership is decided by
// WindowVisualController. No surface or input geometry is changed.
class ShadowGeometry
{
public:
    // Larger corners make no shadow: the corner assets are kept small. The
    // switcher's 36 is the largest corner that casts one; a smaller shadow
    // corner shows past a glass corner as a dark crescent.
    static constexpr int kMaxRadius = 36;

    // A non-empty `silhouette`, window-local, replaces the rounded rectangle.
    // Its longest straight run on each axis is stretched the way the
    // nine-patch stretches the sides of a rounded rectangle.
    ShadowGeometry(const base::Size &windowSize, int radius, const ShadowStyle &style,
                   int rasterScale = 1, const Corners &corners = Corners::all(),
                   const ui::Path &silhouette = {});

    base::Rect paintBounds() const;

    const std::array<base::Rect, 9> &targets() const { return m_targets; }

    const std::array<base::Rect, 9> &sources() const { return m_sources; }

    base::Size rasterSize() const { return m_rasterSize; }

    base::Size canonicalSize() const { return m_canonicalSize; }

    base::Insets margins() const { return m_margins; }

    std::string key() const;
    // The nine-patch source, drawn by `renderer` at rasterSize(); null where
    // it cannot draw one.
    std::unique_ptr<render::Texture> render(render::Renderer &renderer) const;

private:
    base::Size m_windowSize;
    base::Size m_canonicalSize;
    base::Size m_rasterSize;
    int m_radius = 0;
    int m_scale = 1;
    Corners m_corners;
    ShadowStyle m_style;
    ui::Path m_silhouette;
    // Window-local start and end of the stretched run on each axis.
    base::Point m_stretchBegin;
    base::Point m_stretchEnd;
    base::Insets m_margins;
    std::array<base::Rect, 9> m_targets;
    std::array<base::Rect, 9> m_sources;
};

void paintShadow(render::Renderer &renderer, const ShadowGeometry &geometry,
                 const base::Point &position, float opacity = 1, const base::PointF &snap = {});

// `geometry` stretched over `window`, a window near its size: one texture
// stands in for many sizes in a frame of motion.
void paintStretchedShadow(render::Renderer &renderer, const ShadowGeometry &geometry,
                          const base::Rect &window, float opacity = 1);

} // namespace Lingmo::Decoration

#endif // LINGMO_DECORATION_SHADOW_H_
