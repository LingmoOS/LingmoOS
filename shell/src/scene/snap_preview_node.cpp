// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "scene/snap_preview_node.h"

#include <algorithm>
#include <chrono>

#include "scene/motion.h"
#include "base/geometry.h"

namespace Lingmo::Scene {

namespace {

// Upstream kScrimStartBoundsRatio: the plate starts at this fraction of its
// size, centred on where it ends.
constexpr double kStartScale = 0.75;

// gfx::Tween::ACCEL_20_DECEL_100.
double accel20Decel100(double t)
{
    return motion::cubicBezier(.2, 0, 0, 1, t);
}

render::Color faded(render::Color color, float opacity)
{
    color.a *= opacity;
    return color;
}

} // namespace

SnapPreviewNode::SnapPreviewNode(std::function<double()> clock)
    : Node("snap-preview")
    , m_now(clock ? std::move(clock) : [] {
        return std::chrono::duration<double, std::milli>(
                       std::chrono::steady_clock::now().time_since_epoch())
                .count();
    })
{
}

void SnapPreviewNode::show(const base::Rect &windowBounds)
{
    const base::Rect target = windowBounds.adjusted(kInset, kInset, -kInset, -kInset);
    if (target == geometry())
        return;
    m_started = -1;
    setGeometry(target);
    notifyChanged();
}

void SnapPreviewNode::setColors(const render::Color &fill, const render::Color &outline,
                                const render::Color &highlight)
{
    m_fill = fill;
    m_outline = outline;
    m_highlight = highlight;
    notifyChanged();
}

base::RectF SnapPreviewNode::entranceBounds(const base::RectF &target, double elapsedMs)
{
    const double duration = kSizeDurationMs * motion::durationScale();
    const double t = accel20Decel100(elapsedMs / duration);
    const double scale = kStartScale + (1 - kStartScale) * t;
    const base::SizeF size = target.size() * scale;
    return base::RectF(target.center() - base::PointF(size.width(), size.height()) / 2, size);
}

float SnapPreviewNode::entranceOpacity(double elapsedMs)
{
    const double duration = kOpacityDurationMs * motion::durationScale();
    return float(std::clamp(elapsedMs / duration, 0.0, 1.0));
}

void SnapPreviewNode::paint(render::Renderer &renderer, const base::Rect &absoluteGeometry) const
{
    if (absoluteGeometry.isEmpty())
        return;

    const double now = renderer.frameTime >= 0 ? renderer.frameTime : m_now();
    // A capture only records what is on screen; it must not start anything.
    if (m_started < 0 && !renderer.capturing)
        m_started = now;
    const double elapsed = m_started < 0 ? 0 : now - m_started;
    const base::RectF bounds = entranceBounds(base::RectF(absoluteGeometry), elapsed);
    const float opacity = entranceOpacity(elapsed);

    if (opacity > 0) {
        const render::RoundedShape shape {bounds, kRadius, {1, 1}};
        // fillRectF is a rectangle; the rounded clip is what gives it its corners.
        renderer.pushRoundedClip(shape);
        renderer.fillRectF(bounds, faded(m_fill, opacity));
        renderer.popRoundedClip();

        // views::HighlightBorder: a physical-pixel hairline, and a second one
        // just inside it on the same radius.
        renderer.drawRoundedOutline(shape, faded(m_outline, opacity), 1);
        const double pixel = 1 / renderer.rasterScale();
        const render::RoundedShape inner {bounds.adjusted(pixel, pixel, -pixel, -pixel), kRadius,
                                          {1, 1}};
        renderer.drawRoundedOutline(inner, faded(m_highlight, opacity), 1);
    }

    if (elapsed < kSizeDurationMs * motion::durationScale())
        notifyChanged();
}

} // namespace Lingmo::Scene
