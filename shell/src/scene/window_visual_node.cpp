// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "scene/window_visual_node.h"

#include <algorithm>
#include <cmath>
#include <format>
#include <utility>

#include "render/shape_mask.h"
#include "base/geometry.h"
#include "base/transform.h"
#include "ui/geometry_conversions.h"

namespace Lingmo::Scene {

namespace {

// ash/wm/window_dimmer.cc kDefaultDimAnimationDurationMs.
constexpr int64_t kDimDurationMs = 200;

} // namespace

void paintWindowOutline(render::Renderer &renderer, const render::RoundedShape &shape,
                        const WindowVisualState &state, float opacity)
{
    const double stroke = state.outlineWidthPx / renderer.rasterScale();
    auto inner = shape;
    // On whole pixels, so no edge splits its stroke over two.
    inner.bounds = renderer.pixelsInside(shape.bounds);
    auto outer = inner;
    outer.bounds = outer.bounds.adjusted(-stroke, -stroke, stroke, stroke);
    if (shape.radius > 0)
        outer.radius += stroke / std::max(shape.radiusScale.width(), shape.radiusScale.height());
    if (!renderer.drawsInto(outer.bounds.toEnclosingRect().adjusted(-1, -1, 1, 1)))
        return;
    auto outline = state.outlineColor;
    outline.a *= opacity;
    renderer.drawRoundedOutline(outer, outline, state.outlineWidthPx);
    if (state.highlightColor.a <= 0)
        return;
    auto highlight = state.highlightColor;
    highlight.a *= opacity;
    renderer.drawRoundedOutline(inner, highlight, state.outlineWidthPx);
}

void paintShapeOutline(render::Renderer &renderer, const WindowVisualState &state,
                       const base::AffineTransform &transform, float opacity)
{
    const auto raster =
            render::ShapeRaster::covering(ui::toBase(state.outlinePath.boundingRect()), renderer.rasterScale(),
                                          int(std::ceil(state.outlineWidthPx)) + 1);
    const auto color = [](const render::Color &c) {
        return std::format(":{},{},{},{}", c.r, c.g, c.b, c.a);
    };
    const std::string key = "shape-outline:" + render::shapeKey(state.outlinePath) + raster.key()
            + color(state.outlineColor) + color(state.highlightColor)
            + std::format("{}", state.outlineWidthPx);
    const auto &textures = renderer.cachedTextureSet(key, [&] {
        const ui::Bitmap image =
                render::rasterizeShapeOutline(state.outlinePath, raster, state.outlineWidthPx,
                                              state.outlineColor, state.highlightColor);
        render::Renderer::TextureSet set;
        set.push_back(renderer.createTexture(image.size(), image.constBits(), int(image.stride()),
                                             render::PixelFormat::Rgba8888));
        return set;
    });
    // Looked up even when culled, so the cache keeps it as long as before.
    const base::RectF target = transform.mapRect(raster.target());
    if (textures.front() && renderer.drawsInto(target.toEnclosingRect()))
        renderer.drawTextureF(textures.front().get(), target, opacity);
}

base::PointF clientBodySnap(const render::Renderer &renderer, const WindowVisualState &state,
                       const base::PointF &position)
{
    const base::PointF surface = position + state.surfaceOrigin;
    return renderer.pixelAligned(surface) - surface;
}

float WindowVisualNode::presentedDim(const render::Renderer &renderer) const
{
    if (!m_dimFading)
        return m_dimPresented;
    // Outside an output frame there is nothing to pace the fade against.
    if (renderer.frameTime < 0) {
        m_dimFading = false;
        return m_dimPresented = m_state.dimOpacity;
    }
    if (m_dimStarted < 0)
        m_dimStarted = renderer.frameTime;
    const double progress =
            std::clamp((renderer.frameTime - m_dimStarted) / kDimDurationMs, 0.0, 1.0);
    m_dimPresented = m_dimFrom + (m_state.dimOpacity - m_dimFrom) * float(progress);
    if (progress >= 1)
        m_dimFading = false;
    else
        notifyChanged();
    return m_dimPresented;
}

void WindowVisualNode::render(render::Renderer &renderer) const
{
    if (!isVisible())
        return;
    const base::PointF position = absolutePosition();
    const base::PointF placed = position + clientBodySnap(renderer, m_state, position);
    auto shape = m_state.shape;
    shape.bounds = renderer.pixelsInside(shape.bounds.translated(placed));
    // Taken here, so a window nested below keeps its own corners.
    const auto corners = std::exchange(renderer.windowCorners, std::nullopt);
    if (corners) {
        shape.radius = corners->radius;
        shape.radiusScale = corners->radiusScale;
    }
    const bool clip = m_state.clipContent || corners;
    if (clip)
        renderer.pushRoundedClip(shape);
    Node::render(renderer);
    if (const float dim = presentedDim(renderer);
        dim > 0 && renderer.drawsInto(shape.bounds.toEnclosingRect()))
        renderer.fillRectF(shape.bounds, {0, 0, 0, dim});
    if (clip)
        renderer.popRoundedClip();
    if (m_state.drawOutline && !renderer.suppressWindowOutlines && !m_state.outlinePath.isEmpty()) {
        // Rasterized where it lands, so it stays on physical pixels at any scale.
        auto state = m_state;
        state.outlinePath.translate(placed.x(), placed.y());
        paintShapeOutline(renderer, state, base::AffineTransform());
    } else if (m_state.drawOutline && !renderer.suppressWindowOutlines) {
        auto outline = m_state.outlined();
        outline.bounds.translate(placed);
        paintWindowOutline(renderer, outline, m_state);
    }
}

} // namespace Lingmo::Scene
