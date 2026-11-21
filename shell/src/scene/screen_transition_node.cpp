// Copyright 2015 The Chromium Authors
// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.chromium file.
//
// Upstream: chromium/src/ash/rotator/screen_rotation_animator.cc
//           chromium/src/ash/rotator/screen_rotation_animation.cc
//           chromium/src/ui/gfx/geometry/cubic_bezier.cc
// Revision: 7acd40ddefff49c9d36144999aafdbb674dbd07a

#include "scene/screen_transition_node.h"

#include <algorithm>
#include <chrono>
#include <cmath>
#include "base/geometry.h"
#include "base/transform.h"

namespace Lingmo::Scene {

namespace {

struct CubicBezier {
    double ax, bx, cx, ay, by, cy;

    CubicBezier(double x1, double y1, double x2, double y2)
    {
        cx = 3 * x1;
        bx = 3 * (x2 - x1) - cx;
        ax = 1 - cx - bx;
        cy = 3 * y1;
        by = 3 * (y2 - y1) - cy;
        ay = 1 - cy - by;
    }

    double sampleX(double t) const { return ((ax * t + bx) * t + cx) * t; }

    double sampleY(double t) const { return ((ay * t + by) * t + cy) * t; }

    double sampleDerivativeX(double t) const { return (3 * ax * t + 2 * bx) * t + cx; }

    double solveX(double x) const
    {
        constexpr double epsilon = 1e-7;
        double t = x;
        for (int i = 0; i < 8; ++i) {
            const double error = sampleX(t) - x;
            if (std::abs(error) < epsilon)
                return t;
            const double derivative = sampleDerivativeX(t);
            if (std::abs(derivative) < 1e-6)
                break;
            t -= error / derivative;
        }
        double low = 0, high = 1;
        t = x;
        while (low < high) {
            const double value = sampleX(t);
            if (std::abs(value - x) < epsilon)
                return t;
            (x > value ? low : high) = t;
            t = (low + high) / 2;
            if (high - low < epsilon)
                break;
        }
        return t;
    }
};

} // namespace

ScreenTransitionNode::ScreenTransitionNode(std::function<int64_t()> clock)
    : Node("screen-transition")
    , m_now(clock ? std::move(clock) : [] {
        return std::chrono::duration_cast<std::chrono::milliseconds>(
                       std::chrono::steady_clock::now().time_since_epoch())
                .count();
    })
{
}

double ScreenTransitionNode::fastOutLinearIn(double t)
{
    static const CubicBezier bezier(0.4, 0, 1, 1);
    t = std::clamp(t, 0.0, 1.0);
    return bezier.sampleY(bezier.solveX(t));
}

ScreenTransitionNode::RotationSample ScreenTransitionNode::sampleRotation(int degrees,
                                                                          double progress)
{
    const double t = fastOutLinearIn(progress);
    const int direction = degrees > 0 ? 1 : degrees < 0 ? -1 : 0;
    return {degrees - direction * kRotationDegrees * t, float(1 - t),
            direction * kRotationDegrees * (1 - t)};
}

base::RectF ScreenTransitionNode::coverRect(const base::SizeF &source, int degrees, const base::RectF &viewport)
{
    if (source.isEmpty() || viewport.isEmpty())
        return viewport;
    const base::SizeF turned = std::abs(degrees) == 90 ? source.transposed() : source;
    const double scale =
            std::max(viewport.width() / turned.width(), viewport.height() / turned.height());
    const base::SizeF size = source * scale;
    return base::RectF(viewport.center() - base::PointF(size.width(), size.height()) / 2, size);
}

base::AffineTransform ScreenTransitionNode::aboutCenter(const base::PointF &center, double degrees)
{
    base::AffineTransform transform;
    transform.translate(center.x(), center.y());
    transform.rotate(degrees);
    transform.translate(-center.x(), -center.y());
    return transform;
}

void ScreenTransitionNode::start(Outputs outputs, Paint paintIncoming, std::function<bool()> ready,
                                 int timeoutMs)
{
    m_outputs.clear();
    for (auto &[id, output] : outputs)
        m_outputs[id].output = std::move(output);
    m_paintIncoming = std::move(paintIncoming);
    m_ready = std::move(ready);
    m_deadline = m_now() + timeoutMs;
    m_animationStarted = -1;
    notifyChanged();
}

void ScreenTransitionNode::stopAnimating()
{
    if (isAnimating())
        finish();
}

void ScreenTransitionNode::finish() const
{
    m_outputs.clear();
    m_paintIncoming = {};
    m_ready = {};
    m_animationStarted = -1;
    // Every output repaints without the layers, not only the one that ended them.
    notifyChanged();
}

void ScreenTransitionNode::render(render::Renderer &renderer) const
{
    if (!isActive() || !isVisible())
        return;
    const int64_t now = m_now();
    // A capture only records what is on screen; it must not advance anything.
    if (!isAnimating() && !renderer.capturing && (now >= m_deadline || !m_ready || m_ready()))
        m_animationStarted = now;
    const double progress =
            isAnimating() ? double(now - m_animationStarted) / kRotationDurationMs : 0;
    if (progress >= 1) {
        finish();
        return;
    }
    const auto &context = renderer.outputContext();
    const auto state = m_outputs.find(context.outputId);
    if (state != m_outputs.end() && state->second.output.outgoing) {
        const base::Rect viewport =
                context.logicalViewport.isEmpty() ? absoluteGeometry() : context.logicalViewport;
        auto &layers = state->second;
        if (isAnimating() && !layers.incoming && !layers.incomingFailed && !renderer.capturing
            && m_paintIncoming) {
            layers.incomingFailed = !renderer.renderToTexture(layers.incoming, viewport, [&] {
                m_paintIncoming(renderer);
            });
        }
        const auto &output = layers.output;
        const RotationSample sample = sampleRotation(output.degrees, progress);
        const base::PointF center = base::RectF(viewport).center();
        // The mask hides the live content around a turned copy.
        if (!isAnimating() || layers.incoming)
            renderer.fillRect(viewport, {0, 0, 0, 1});
        if (layers.incoming) {
            renderer.drawTextureTransformed(layers.incoming.get(), viewport,
                                            aboutCenter(center, sample.newDegrees));
        }
        renderer.drawTextureTransformed(
                output.outgoing.get(), coverRect(output.outgoing->size(), output.degrees, viewport),
                aboutCenter(center, sample.oldDegrees), sample.oldOpacity);
    }
    if (isAnimating())
        notifyChanged();
}

void ScreenTransitionNode::releaseRenderResources()
{
    m_outputs.clear();
    m_paintIncoming = {};
    m_ready = {};
    m_animationStarted = -1;
    Node::releaseRenderResources();
}

} // namespace Lingmo::Scene
