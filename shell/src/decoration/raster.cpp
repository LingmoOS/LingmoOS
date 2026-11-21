// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "decoration/raster.h"

#include <algorithm>
#include <cmath>

#include "render/rounded_shape.h"

namespace Lingmo::Decoration {

namespace {

constexpr int kSamples = 4;

uint8_t toByte(float value)
{
    return static_cast<uint8_t>(std::lround(std::clamp(value, 0.0f, 1.0f) * 255.0f));
}

bool insideRoundedRect(const base::RectF& rect, double radius, const Corners& corners, double x, double y,
                       bool continuous = false)
{
    if (x < rect.left() || x > rect.right() || y < rect.top() || y > rect.bottom())
        return false;
    if (radius <= 0.0)
        return true;

    const double rx = continuous ? render::continuousCornerExtent(radius, rect.width() / 2)
                               : std::min(radius, std::min(rect.width(), rect.height()) / 2);
    const double ry = continuous ? render::continuousCornerExtent(radius, rect.height() / 2) : rx;
    if (rx <= 0 || ry <= 0)
        return true;
    const double left = rect.left() + rx;
    const double right = rect.right() - rx;
    const double top = rect.top() + ry;
    const double bottom = rect.bottom() - ry;

    const bool rounded = (x < left && y < top && corners.topLeft)
            || (x > right && y < top && corners.topRight)
            || (x < left && y > bottom && corners.bottomLeft)
            || (x > right && y > bottom && corners.bottomRight);
    if (!rounded)
        return true;

    if (continuous)
        return render::RoundedShape {rect, radius}.contains({x, y});

    const double dx = std::max(std::abs(x - rect.center().x()) - rect.width() / 2 + rx, double(0));
    const double dy = std::max(std::abs(y - rect.center().y()) - rect.height() / 2 + ry, double(0));
    return dx * dx + dy * dy <= rx * rx;
}

// Runs `test` over a 4x4 grid inside pixel (x, y) and returns the fraction that
// passed.
template<typename F>
float coverage(int x, int y, F&& test)
{
    int hits = 0;
    for (int sy = 0; sy < kSamples; ++sy) {
        for (int sx = 0; sx < kSamples; ++sx) {
            const double px = x + (sx + 0.5) / kSamples;
            const double py = y + (sy + 0.5) / kSamples;
            if (test(px, py))
                ++hits;
        }
    }
    return static_cast<float>(hits) / (kSamples * kSamples);
}

// The pixels a shape bounded by `rect` can touch, the far edges' included.
base::Rect pixelBounds(const Image& image, const base::RectF& rect, double margin = 1.0)
{
    const base::Rect bounds = base::Rect::fromEdges(
        static_cast<int>(std::floor(rect.left() - margin)),
        static_cast<int>(std::floor(rect.top() - margin)),
        static_cast<int>(std::ceil(rect.right() + margin)) + 1,
        static_cast<int>(std::ceil(rect.bottom() + margin)) + 1);
    return bounds.intersected(base::Rect(image.size()));
}

// Whole pixels no corner reaches, which every sample of a fill would hit.
// Skipping their supersampling keeps a window-sized shadow mask cheap.
struct SolidPixels {
    SolidPixels(const base::RectF& rect, double radius, bool continuous)
        : rect(rect)
    {
        if (radius > 0.0) {
            rx = continuous ? render::continuousCornerExtent(radius, rect.width() / 2)
                            : std::min(radius, std::min(rect.width(), rect.height()) / 2);
            ry = continuous ? render::continuousCornerExtent(radius, rect.height() / 2) : rx;
        }
    }

    bool contains(int x, int y) const
    {
        if (x < rect.left() || x + 1 > rect.right() || y < rect.top() || y + 1 > rect.bottom())
            return false;
        return (x >= rect.left() + rx && x + 1 <= rect.right() - rx)
            || (y >= rect.top() + ry && y + 1 <= rect.bottom() - ry);
    }

    base::RectF rect;
    double rx = 0.0;
    double ry = 0.0;
};

template<typename F>
void fill(Image& image, const base::RectF& area, const SolidPixels& solid, F&& test,
          const render::Color& color)
{
    const base::Rect bounds = pixelBounds(image, area);
    for (int y = bounds.top(); y < bounds.bottom(); ++y) {
        for (int x = bounds.left(); x < bounds.right(); ++x) {
            const float alpha = solid.contains(x, y) ? 1.0f : coverage(x, y, test);
            if (alpha > 0.0f)
                image.blend(x, y, color, alpha);
        }
    }
}

} // namespace

Image::Image(const base::Size& size)
    : m_size(size)
    , m_pixels(static_cast<size_t>(std::max(0, m_size.width()))
                   * static_cast<size_t>(std::max(0, m_size.height())) * 4,
               0)
{
}

void Image::blend(int x, int y, const render::Color& color, float coverage)
{
    if (x < 0 || y < 0 || x >= m_size.width() || y >= m_size.height())
        return;

    const float alpha = color.a * std::clamp(coverage, 0.0f, 1.0f);
    if (alpha <= 0.0f)
        return;

    uint8_t* pixel = &m_pixels[(static_cast<size_t>(y) * m_size.width() + x) * 4];
    const float inverse = 1.0f - alpha;
    // Premultiplied source over premultiplied destination.
    pixel[0] = toByte(color.r * alpha + pixel[0] / 255.0f * inverse);
    pixel[1] = toByte(color.g * alpha + pixel[1] / 255.0f * inverse);
    pixel[2] = toByte(color.b * alpha + pixel[2] / 255.0f * inverse);
    pixel[3] = toByte(alpha + pixel[3] / 255.0f * inverse);
}

void fillRoundedRect(Image& image, const base::RectF& rect, double radius, const render::Color& color,
                     const Corners& corners)
{
    fill(
        image, rect, SolidPixels(rect, radius, false),
        [&](double x, double y) {
            return insideRoundedRect(rect, radius, corners, x, y);
        },
        color);
}

void fillContinuousRoundedRect(Image& image, const base::RectF& rect, double radius,
                               const render::Color& color, const Corners& corners)
{
    fill(
        image, rect, SolidPixels(rect, radius, true),
        [&](double x, double y) {
            return insideRoundedRect(rect, radius, corners, x, y, true);
        },
        color);
}

} // namespace Lingmo::Decoration
