// Copyright 2012 The Chromium Authors
// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.chromium file.
//
// Upstream: chromium/src/ui/compositor_extra/shadow_unittest.cc
//           chromium/src/ui/gfx/shadow_value_unittest.cc
// Revision: 7acd40ddefff49c9d36144999aafdbb674dbd07a
//
// What is taken: the margin, bounds, small-content and cache cases.

#include <algorithm>
#include <cstdint>

#include <gtest/gtest.h>

#include "decoration/decoration_theme.h"
#include "decoration/shadow.h"
#include "fake_renderer.h"
#include "ui/path.h"
#include "wm/shadow_types.h"

using namespace Lingmo;
using namespace Lingmo::Decoration;

namespace {

base::Size times(const base::Size &size, int factor)
{
    return base::Size(size.width() * factor, size.height() * factor);
}

TEST(ShadowTest, OffsetMarginsAndUnits)
{
    const ShadowStyle style {{{2, {1, 2}, 0.4f}, {1, {-2, -3}, 0.2f}}};
    ShadowGeometry geometry({300, 200}, 11, style);
    EXPECT_EQ(geometry.margins(), base::Insets(5, 6, 7, 8));
    EXPECT_EQ(geometry.paintBounds(), base::Rect(-5, -6, 312, 214));
    ShadowGeometry scaled({300, 200}, 11, style, 2);
    EXPECT_EQ(scaled.paintBounds(), geometry.paintBounds());
    EXPECT_EQ(scaled.rasterSize(), times(geometry.rasterSize(), 2));
    EXPECT_NE(scaled.key(), geometry.key());
    ShadowGeometry scale4({300, 200}, 11, style, 4);
    EXPECT_EQ(scale4.paintBounds(), geometry.paintBounds());
    EXPECT_EQ(scale4.rasterSize(), times(geometry.rasterSize(), 4));
    EXPECT_NE(scale4.key(), scaled.key());
    EXPECT_TRUE(ShadowGeometry({0, 2}, 0, style).paintBounds().isEmpty());
    EXPECT_TRUE(ShadowGeometry({2, 2}, 0, style, 0).paintBounds().isEmpty());
}

TEST(ShadowTest, TinyAndLargeSlicesAreBoundedAndDoNotOverlap)
{
    Metrics metrics;
    const ShadowGeometry largest(base::Size(1 << 16, 1 << 16), metrics.cornerRadius,
                                 metrics.shadow(true));
    for (const base::Size size : {base::Size(1, 1), base::Size(2, 2), base::Size(3, 40), base::Size(40, 3),
                                  base::Size(21, 31), base::Size(100, 400), base::Size(100000, 100000)}) {
        ShadowGeometry geometry(size, WindowVisualStyle::radiusFor(size), metrics.shadow(true));
        int64_t area = 0;
        for (size_t i = 0; i < 9; ++i) {
            const auto rect = geometry.targets()[i];
            EXPECT_GE(rect.width(), 0);
            EXPECT_GE(rect.height(), 0);
            area += int64_t(rect.width()) * rect.height();
            for (size_t j = 0; j < i; ++j)
                EXPECT_TRUE(rect.intersected(geometry.targets()[j]).isEmpty());
        }
        EXPECT_EQ(area, int64_t(geometry.paintBounds().width()) * geometry.paintBounds().height());
        // However big the window, the raster never exceeds the canonical
        // nine-patch source: everything past it is the stretched aperture.
        EXPECT_LE(geometry.rasterSize().width(), largest.rasterSize().width());
        EXPECT_LE(geometry.rasterSize().height(), largest.rasterSize().height());
    }
}

// A tall popup with an arrow under its bottom edge.
ui::Path arrowedPopup(const base::Size &size)
{
    const double body = size.height() - 9;
    ui::Path path;
    path.addRoundedRect(ui::RectF {0, 0, double(size.width()), body}, 10, 10);
    ui::Path arrow;
    arrow.moveTo(size.width() / 2.0 - 10, body - 1);
    arrow.lineTo(size.width() / 2.0 + 10, body - 1);
    arrow.lineTo(size.width() / 2.0, size.height());
    arrow.close();
    return path.united(arrow);
}

TEST(ShadowTest, SilhouetteStretchesOnlyItsStraightRuns)
{
    Metrics metrics;
    const base::Size size(160, 700);
    const auto style = metrics.shadowFor(wm::ShadowElevation::MenuOrTooltip);
    for (int scale : {1, 2}) {
        ShadowGeometry geometry(size, 0, style, scale, Corners::all(), arrowedPopup(size));
        ASSERT_FALSE(geometry.paintBounds().isEmpty());
        // Only the height has a run long enough to stretch; the arrow breaks the width.
        EXPECT_LT(geometry.canonicalSize().height(), 200);
        EXPECT_EQ(geometry.canonicalSize().width(), size.width());
        EXPECT_EQ(geometry.sources()[4].height(), scale);
        int64_t area = 0;
        for (size_t i = 0; i < 9; ++i)
            area += int64_t(geometry.targets()[i].width()) * geometry.targets()[i].height();
        EXPECT_EQ(area, int64_t(geometry.paintBounds().width()) * geometry.paintBounds().height());
    }
}

// The renderer is handed the style in the pixels of the image it draws.
TEST(ShadowTest, LobesReachTheRendererInImagePixels)
{
    const ShadowStyle style {{{2.5f, {1, 3}, 0.4f}, {1, {0, 0}, 0}, {4, {-2, 0}, 0.1f}}};
    testsupport::FakeRenderer renderer;
    const auto texture = ShadowGeometry({300, 200}, 11, style, 2).render(renderer);
    ASSERT_TRUE(texture);
    EXPECT_EQ(texture->size(), ShadowGeometry({300, 200}, 11, style, 2).rasterSize());
    // A lobe with no opacity is left out.
    ASSERT_EQ(renderer.lastShadowLobes.size(), 2u);
    EXPECT_FLOAT_EQ(renderer.lastShadowLobes[0].sigma, 5);
    EXPECT_EQ(renderer.lastShadowLobes[0].offset, base::Point(2, 6));
    EXPECT_FLOAT_EQ(renderer.lastShadowLobes[0].opacity, 0.4f);
    EXPECT_FLOAT_EQ(renderer.lastShadowLobes[1].sigma, 8);
    EXPECT_EQ(renderer.lastShadowLobes[1].offset, base::Point(-4, 0));
    const ShadowStyle invisible {{{2, {}, 0}}};
    EXPECT_FALSE(ShadowGeometry({300, 200}, 11, invisible).render(renderer));
}

TEST(ShadowTest, MovesAndOrdinaryResizesReuseAndSmallVariantsRetire)
{
    Metrics metrics;
    testsupport::FakeRenderer renderer;
    for (int i = 0; i < 100; ++i) {
        ShadowGeometry geometry({500 + i, 400 + i}, 11, metrics.shadow(true));
        paintShadow(renderer, geometry, {i, i});
        renderer.calls.clear();
    }
    EXPECT_EQ(renderer.textureCacheMisses(), 1u);
    EXPECT_EQ(renderer.renderedShadows, 1);
    paintShadow(renderer, ShadowGeometry({500, 400}, 11, metrics.shadow(false)), {});
    EXPECT_EQ(renderer.textureCacheMisses(), 2u);
    const ShadowStyle small {{{1, {}, 0.2f}, {0, {}, 0.1f}}};
    for (int width = 1; width <= 40; ++width) {
        paintShadow(renderer, ShadowGeometry({width, 10}, std::min(width / 2, 10), small), {});
        renderer.calls.clear();
        EXPECT_LE(renderer.textureCacheSize(), render::Renderer::textureCacheLimit);
    }
    EXPECT_EQ(renderer.textureCacheSize(), render::Renderer::textureCacheLimit);
    renderer.clearTextureCache();
    EXPECT_EQ(renderer.textureCacheSize(), 0u);
    testsupport::FakeRenderer replacement;
    paintShadow(replacement, ShadowGeometry({500, 400}, 11, metrics.shadow(true)), {});
    EXPECT_EQ(replacement.renderedShadows, 1);
    // One image, drawn as the eight slices around the window.
    EXPECT_EQ(replacement.kinds(), std::vector<std::string>(8, "texture"));
}

} // namespace
