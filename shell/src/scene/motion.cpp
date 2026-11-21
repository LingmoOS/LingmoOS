// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "scene/motion.h"

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <numbers>

namespace Lingmo::Scene::Motion {

namespace {

// Below this the closed forms for the under- and overdamped cases lose
// precision; the critically damped one is exact there.
constexpr double kCriticalTolerance = 1e-4;

double bezierAt(double a, double b, double s)
{
    return 3 * a * (1 - s) * (1 - s) * s + 3 * b * (1 - s) * s * s + s * s * s;
}

double bezierSlope(double a, double b, double s)
{
    return 3 * a * (1 - s) * (1 - s) + 6 * (b - a) * (1 - s) * s + 3 * (1 - b) * s * s;
}

// The curve parameter where its x is `x`.
double bezierParameter(double x1, double x2, double x)
{
    x = std::clamp(x, 0.0, 1.0);
    double low = 0;
    double high = 1;
    for (int i = 0; i < 32; ++i) {
        const double middle = (low + high) / 2;
        (bezierAt(x1, x2, middle) < x ? low : high) = middle;
    }
    return (low + high) / 2;
}

} // namespace

namespace {

double s_durationScale = 1;

} // namespace

double durationScale()
{
    return s_durationScale;
}

void setDurationScale(double scale)
{
    s_durationScale = std::isfinite(scale) && scale > 0 ? scale : 1;
}

double cubicBezier(double x1, double y1, double x2, double y2, double x)
{
    return bezierAt(y1, y2, bezierParameter(x1, x2, x));
}

Tween::Tween(double time, double from, double target, double durationMs, double velocity,
             const TimingCurve &curve)
    : m_start(time)
    , m_from(from)
    , m_target(target)
    , m_duration(std::max(0.0, durationMs * durationScale()))
    , m_x1(curve.x1)
    , m_y1(curve.y1)
    , m_x2(curve.x2)
    , m_y2(curve.y2)
{
    const double distance = target - from;
    if (distance == 0 || m_duration <= 0 || velocity * distance <= 0)
        return;
    // The curve leaves at y1 / x1 times its average speed; y1 above 1 would
    // overshoot, so a faster start takes less time.
    const double maxSlope = 1 / m_x1;
    double slope = velocity * m_duration / 1000 / distance;
    if (slope > maxSlope) {
        m_duration *= maxSlope / slope;
        slope = maxSlope;
    }
    m_y1 = m_x1 * slope;
}

double Tween::value(double time) const
{
    if (isFinished(time))
        return m_target;
    if (time <= m_start)
        return m_from;
    const double y = cubicBezier(m_x1, m_y1, m_x2, m_y2, (time - m_start) / m_duration);
    return m_from + (m_target - m_from) * y;
}

double Tween::velocity(double time) const
{
    if (isFinished(time) || time < m_start)
        return 0;
    const double s = bezierParameter(m_x1, m_x2, (time - m_start) / m_duration);
    const double dx = bezierSlope(m_x1, m_x2, s);
    if (dx <= 0)
        return 0;
    return (m_target - m_from) * bezierSlope(m_y1, m_y2, s) / dx / (m_duration / 1000);
}

Spring::Spring()
{
    updateCoefficients();
}

Spring::Spring(const SpringParameters &parameters)
    : m_parameters(parameters)
{
    updateCoefficients();
}

void Spring::updateCoefficients()
{
    const double response = m_parameters.response * durationScale();
    m_omega = response > 0 ? 2 * std::numbers::pi / response : 0;
    m_zeta = std::max(0.0, m_parameters.dampingRatio);
}

void Spring::reset(double time, double value, double target, double velocity)
{
    m_start = time;
    m_target = target;
    m_displacement = value - target;
    m_velocity = velocity;
}

void Spring::retarget(double time, double target)
{
    reset(time, value(time), target, velocity(time));
}

double Spring::elapsed(double time) const
{
    return std::max(0.0, time - m_start) / 1000;
}

double Spring::displacement(double t) const
{
    const double x0 = m_displacement;
    const double v0 = m_velocity;
    if (m_omega <= 0)
        return 0;
    const double w = m_omega;
    if (std::abs(m_zeta - 1) < kCriticalTolerance)
        return std::exp(-w * t) * (x0 + (v0 + w * x0) * t);
    if (m_zeta < 1) {
        const double decay = m_zeta * w;
        const double wd = w * std::sqrt(1 - m_zeta * m_zeta);
        return std::exp(-decay * t)
                * (x0 * std::cos(wd * t) + (v0 + decay * x0) / wd * std::sin(wd * t));
    }
    const double root = std::sqrt(m_zeta * m_zeta - 1);
    const double r1 = -w * (m_zeta - root);
    const double r2 = -w * (m_zeta + root);
    const double c2 = (v0 - r1 * x0) / (r2 - r1);
    const double c1 = x0 - c2;
    return c1 * std::exp(r1 * t) + c2 * std::exp(r2 * t);
}

double Spring::displacementVelocity(double t) const
{
    const double x0 = m_displacement;
    const double v0 = m_velocity;
    if (m_omega <= 0)
        return 0;
    const double w = m_omega;
    if (std::abs(m_zeta - 1) < kCriticalTolerance)
        return std::exp(-w * t) * (v0 - w * (v0 + w * x0) * t);
    if (m_zeta < 1) {
        const double decay = m_zeta * w;
        const double wd = w * std::sqrt(1 - m_zeta * m_zeta);
        return std::exp(-decay * t)
                * (v0 * std::cos(wd * t) - (w * w * x0 + decay * v0) / wd * std::sin(wd * t));
    }
    const double root = std::sqrt(m_zeta * m_zeta - 1);
    const double r1 = -w * (m_zeta - root);
    const double r2 = -w * (m_zeta + root);
    const double c2 = (v0 - r1 * x0) / (r2 - r1);
    const double c1 = x0 - c2;
    return c1 * r1 * std::exp(r1 * t) + c2 * r2 * std::exp(r2 * t);
}

double Spring::value(double time) const
{
    return m_target + displacement(elapsed(time));
}

double Spring::velocity(double time) const
{
    return displacementVelocity(elapsed(time));
}

Spring settlingSpring(double time, double from, double target, double velocity, double response)
{
    const double omega = 2 * std::numbers::pi / (response * durationScale());
    const double towards = target - from;
    if (velocity * towards > 0 && std::abs(velocity) > omega * std::abs(towards))
        velocity = omega * towards;
    Spring spring({response, 1});
    spring.reset(time, from, target, velocity);
    return spring;
}

} // namespace Lingmo::Scene::Motion
