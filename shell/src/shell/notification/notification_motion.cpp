// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "shell/notification/notification_motion.h"

#include <algorithm>
#include <cmath>

namespace Lingmo::Shell {

// A cubic bezier from (0, 0) to (1, 1) through (x1, y1) and (x2, y2): x solved
// for t, then y at t.
static double cubicBezier(double x1, double y1, double x2, double y2, double x)
{
    const auto at = [](double a, double b, double t) {
        const double u = 1 - t;
        return 3 * u * u * t * a + 3 * u * t * t * b + t * t * t;
    };
    const auto slope = [](double a, double b, double t) {
        const double u = 1 - t;
        return 3 * u * u * a + 6 * u * t * (b - a) + 3 * t * t * (1 - b);
    };
    double t = x;
    for (int i = 0; i < 8; ++i) {
        const double error = at(x1, x2, t) - x;
        if (std::abs(error) < 1e-7)
            return at(y1, y2, t);
        const double d = slope(x1, x2, t);
        if (std::abs(d) < 1e-6)
            break;
        t -= error / d;
    }
    // Bisection where Newton's steps stall.
    double low = 0;
    double high = 1;
    t = x;
    for (int i = 0; i < 40; ++i) {
        const double value = at(x1, x2, t);
        if (std::abs(value - x) < 1e-7)
            break;
        if (value < x)
            low = t;
        else
            high = t;
        t = (low + high) / 2;
    }
    return at(y1, y2, t);
}

double motionEasing(double progress)
{
    if (progress <= 0)
        return 0;
    if (progress >= 1)
        return 1;
    return cubicBezier(0.25, 1, 0.5, 1, progress);
}

double outCubic(double progress)
{
    const double t = std::clamp(progress, 0.0, 1.0) - 1;
    return t * t * t + 1;
}

Tween::Tween(ui::Window *window, std::chrono::milliseconds duration, Easing easing)
    : FrameClient(window)
    , m_duration(duration)
    , m_easing(easing)
{
}

void Tween::setTarget(double target, bool animate)
{
    if (!animate || m_duration.count() <= 0 || !window()) {
        snap(target);
        return;
    }
    if (target == m_to && (isTicking() || m_value == target))
        return;
    m_from = m_value;
    m_to = target;
    m_startNs = 0;
    if (m_from == m_to) {
        stopTicking();
        return;
    }
    startTicking();
}

void Tween::snap(double value)
{
    stopTicking();
    const bool changed = value != m_value;
    m_value = m_from = m_to = value;
    if (changed && onChanged)
        onChanged();
}

void Tween::tick(uint64_t frameTimeNs)
{
    // The first frame after the change shows where it starts from.
    if (!m_startNs)
        m_startNs = frameTimeNs;
    const double elapsed = double(frameTimeNs - m_startNs) / 1e6;
    const double progress = std::min(1.0, elapsed / double(m_duration.count()));
    m_value = m_from + (m_to - m_from) * m_easing(progress);
    if (progress >= 1) {
        m_value = m_to;
        stopTicking();
    }
    if (onChanged)
        onChanged();
    if (progress >= 1 && onFinished)
        onFinished();
}

} // namespace Lingmo::Shell
