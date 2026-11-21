// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "scene/wallpaper_node.h"

#include <algorithm>
#include <cmath>
#include <vector>

#include "render/texture.h"

namespace Lingmo::Scene {

// The status bar gradient's stops along a smoothstep, linear between them.
static double gradientStrength(double position)
{
    static constexpr double kStops[] = {1.0, 0.896, 0.648, 0.352, 0.104, 0.0};
    static constexpr int kLast = 5;
    const double at = std::clamp(position, 0.0, 1.0) * kLast;
    const int index = std::min(int(at), kLast - 1);
    const double t = at - index;
    return kStops[index] + (kStops[index + 1] - kStops[index]) * t;
}

WallpaperNode::WallpaperNode()
    : Node("wallpaper")
{
}

WallpaperNode::~WallpaperNode() = default;

void WallpaperNode::setPicture(const ui::Bitmap &picture)
{
    if (picture.isNull() && m_picture.isNull())
        return;
    m_picture = picture;
    m_pictureStale = true;
    notifyChanged();
}

void WallpaperNode::setColor(const render::Color &color)
{
    if (color.r == m_color.r && color.g == m_color.g && color.b == m_color.b && color.a == m_color.a)
        return;
    m_color = color;
    if (m_picture.isNull())
        notifyChanged();
}

void WallpaperNode::setGradient(std::optional<Gradient> gradient)
{
    if (gradient == m_gradient)
        return;
    m_gradient = gradient;
    notifyChanged();
}

void WallpaperNode::releaseRenderResources()
{
    m_pictureTexture.reset();
    m_pictureRenderer = nullptr;
    m_pictureStale = true;
    m_gradientTexture.reset();
    m_gradientRenderer = nullptr;
    m_gradientDrawn.reset();
}

render::Texture *WallpaperNode::pictureTexture(render::Renderer &renderer) const
{
    if (m_picture.isNull())
        return nullptr;
    if (m_pictureTexture && m_pictureRenderer == &renderer && !m_pictureStale)
        return m_pictureTexture.get();
    // Premultiplied 0xAARRGGBB is BGRA in memory; drawn as XRGB, the picture
    // is opaque.
    const bool reused = m_pictureTexture && m_pictureRenderer == &renderer
            && m_pictureTexture->size() == m_picture.size()
            && renderer.updateTexture(m_pictureTexture.get(), m_picture.constBits(), int(m_picture.stride()),
                                      render::PixelFormat::Bgrx8888);
    if (!reused) {
        m_pictureTexture = renderer.createTexture(m_picture.size(), m_picture.constBits(), int(m_picture.stride()),
                                                  render::PixelFormat::Bgrx8888);
        m_pictureRenderer = m_pictureTexture ? &renderer : nullptr;
    }
    m_pictureStale = !m_pictureTexture;
    return m_pictureTexture.get();
}

render::Texture *WallpaperNode::gradientTexture(render::Renderer &renderer) const
{
    if (!m_gradient || m_gradient->height <= 0)
        return nullptr;
    const int rows = std::max(1, int(std::lround(m_gradient->height * renderer.rasterScale())));
    if (m_gradientTexture && m_gradientRenderer == &renderer && m_gradientDrawn == m_gradient
        && m_gradientRows == rows)
        return m_gradientTexture.get();
    // One texel per device row, sampled at the row's centre as the GPU
    // interpolates a gradient's vertices; stretched across.
    std::vector<uint8_t> pixels(size_t(rows) * 4);
    const ui::Color color = m_gradient->color;
    for (int row = 0; row < rows; ++row) {
        const double alpha = color.a / 255.0 * gradientStrength((row + 0.5) / rows);
        uint8_t *texel = pixels.data() + size_t(row) * 4;
        texel[0] = uint8_t(std::lround(color.r * alpha));
        texel[1] = uint8_t(std::lround(color.g * alpha));
        texel[2] = uint8_t(std::lround(color.b * alpha));
        texel[3] = uint8_t(std::lround(255 * alpha));
    }
    m_gradientTexture = renderer.createTexture(base::Size(1, rows), pixels.data(), 4, render::PixelFormat::Rgba8888);
    m_gradientRenderer = m_gradientTexture ? &renderer : nullptr;
    m_gradientDrawn = m_gradient;
    m_gradientRows = rows;
    return m_gradientTexture.get();
}

void WallpaperNode::paintInto(render::Renderer &renderer, const base::RectF &target, bool mapped) const
{
    const float scale = renderer.opacityScale;
    if (render::Texture *texture = pictureTexture(renderer)) {
        renderer.drawTextureF(texture, target, scale);
    } else {
        render::Color color = m_color;
        if (renderer.overwriteOpaque && color.a >= 1) {
            // Darkened in place, still opaque: nothing under it is blended in.
            color.r *= scale;
            color.g *= scale;
            color.b *= scale;
        } else {
            color.a *= scale;
        }
        renderer.fillRectF(target, color);
    }

    if (render::Texture *texture = gradientTexture(renderer)) {
        const double height = mapped ? m_gradient->height * target.height() / std::max(1, geometry().height())
                                     : m_gradient->height;
        // Over a backdrop the overview darkens, a black gradient keeps its
        // strength: the result is the desktop, gradient included, darkened
        // as one.
        const bool black = m_gradient->color.r == 0 && m_gradient->color.g == 0 && m_gradient->color.b == 0;
        renderer.drawTextureF(texture, base::RectF(target.x(), target.y(), target.width(), height),
                              black ? 1.0f : scale);
    }
}

void WallpaperNode::paint(render::Renderer &renderer, const base::Rect &absoluteGeometry) const
{
    if (absoluteGeometry.isEmpty() || !renderer.drawsInto(absoluteGeometry))
        return;
    paintInto(renderer, base::RectF(absoluteGeometry), false);
}

void WallpaperNode::renderMapped(render::Renderer &renderer, const base::RectF &target) const
{
    if (!isVisible() || target.isEmpty())
        return;
    paintInto(renderer, target, true);
}

} // namespace Lingmo::Scene
