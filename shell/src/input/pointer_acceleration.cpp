// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "input/pointer_acceleration.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdlib>
#include <string>

namespace Lingmo::Input {

namespace {

struct Curve {
    double index;
    double gainLinear;
    double gainParabolic;
    double gainCubic;
    double tangentLinear;
    double tangentParabolicRoot;
};

// HIDAccelCurves of the generic HID pointer, converted from 16.16 fixed point.
// No entry has a quartic gain.
constexpr std::array<Curve, 10> kCurves = {{
        {0.0, 1.0, 0.0, 0.0, 8.0, 0.0},
        {0.125, 0.92, 0.4, 0.08, 8.2, 19.0},
        {0.5, 0.93, 0.55, 0.1, 8.3, 18.0},
        {0.6875, 0.94, 0.71, 0.12, 8.4, 17.0},
        {0.875, 0.95, 0.88, 0.15, 8.5, 16.0},
        {1.0, 0.96, 1.06, 0.18, 8.6, 15.0},
        {1.5, 0.97, 1.25, 0.22, 8.7, 14.0},
        {2.0, 0.98, 1.45, 0.27, 8.8, 13.0},
        {2.5, 0.99, 1.66, 0.33, 8.9, 12.0},
        {3.0, 1.0, 1.88, 0.4, 9.0, 12.0},
}};
// The slider has a step per table entry, the default the fourth.
constexpr double kDefaultStep = 3;

// The table's units: counts per 1/67 s frame, scaled by a 400 dpi default
// resolution, in and 96 pixels per inch out. Those units read the counts of a
// single report, so the curve moves with the report rate; here it is pinned
// to hand speed as a 1000 dpi mouse reporting at 125 Hz meets it.
constexpr double kFrameRate = 67;
constexpr double kScreenResolution = 96;
constexpr double kTableResolution = 400;
constexpr double kReferenceResolution = 1000;
constexpr double kReferenceRate = 125;
constexpr double kStandardPerInchPerSecond =
        kReferenceResolution / kReferenceRate * kFrameRate / kTableResolution;
constexpr double kPixelsPerSecondPerOutput = kReferenceRate * kScreenResolution / kFrameRate;

// libinput extends the last segment beyond the curve.
constexpr double kMaxSpeed = 16; // inches per second
// libinput's LIBINPUT_ACCEL_NPOINTS_MAX.
constexpr int kPoints = 64;
constexpr int kDefaultDpi = 1000;

double tableIndex(double speed)
{
    // 0 stays the default: the steps below it are thirds of the range, the
    // six above it sixths.
    speed = std::clamp(speed, -1.0, 1.0);
    const double last = double(kCurves.size() - 1);
    const double step = speed < 0 ? kDefaultStep * (1 + speed)
                                  : kDefaultStep + speed * (last - kDefaultStep);
    const size_t low = std::min(size_t(step), kCurves.size() - 2);
    const double t = step - double(low);
    return kCurves[low].index + (kCurves[low + 1].index - kCurves[low].index) * t;
}

// Parameters for an index between two table entries are interpolated.
Curve curveAt(double index)
{
    size_t current = 0;
    for (size_t i = 0; i < kCurves.size(); ++i) {
        if (index >= kCurves[i].index)
            current = i;
    }
    if (current + 1 >= kCurves.size() || kCurves[current].index >= index)
        return kCurves[current];
    const Curve &low = kCurves[current];
    const Curve &high = kCurves[current + 1];
    const double t = (index - low.index) / (high.index - low.index);
    const auto mix = [t](double a, double b) {
        return a + (b - a) * t;
    };
    return {index,
            mix(low.gainLinear, high.gainLinear),
            mix(low.gainParabolic, high.gainParabolic),
            mix(low.gainCubic, high.gainCubic),
            mix(low.tangentLinear, high.tangentLinear),
            mix(low.tangentParabolicRoot, high.tangentParabolicRoot)};
}

// The accelerated speed for a standardized one: the polynomial up to the
// first tangent speed, its tangent line up to the second, then the square
// root that meets that line with the same slope. Each gain multiplies the
// speed before the power is taken.
double accelerated(const Curve &c, double x)
{
    const auto polynomial = [&c](double v) {
        return c.gainLinear * v + std::pow(c.gainParabolic * v, 2) + std::pow(c.gainCubic * v, 3);
    };
    if (x <= c.tangentLinear)
        return polynomial(x);
    const double slope = c.gainLinear + 2 * c.tangentLinear * std::pow(c.gainParabolic, 2)
            + 3 * std::pow(c.tangentLinear, 2) * std::pow(c.gainCubic, 3);
    const double intercept = polynomial(c.tangentLinear) - slope * c.tangentLinear;
    if (c.tangentParabolicRoot <= 0 || x <= c.tangentParabolicRoot)
        return slope * x + intercept;
    const double meeting = slope * c.tangentParabolicRoot + intercept;
    const double rootSlope = 2 * meeting * slope;
    const double rootIntercept = meeting * meeting - rootSlope * c.tangentParabolicRoot;
    return std::sqrt(rootSlope * x + rootIntercept);
}

// Logical pixels per second at a hand speed in inches per second.
double pointerSpeed(const Curve &curve, double inchesPerSecond)
{
    return kPixelsPerSecondPerOutput
            * accelerated(curve, inchesPerSecond * kStandardPerInchPerSecond);
}

} // namespace

double pointerGain(double inchesPerSecond, double speed)
{
    const Curve curve = curveAt(tableIndex(speed));
    // The gain at rest is the slope there.
    const double v = std::max(inchesPerSecond, 1e-6);
    return pointerSpeed(curve, v) / v;
}

AccelerationPoints accelerationPoints(int dotsPerInch, double speed)
{
    const int dpi = dotsPerInch > 0 ? dotsPerInch : kDefaultDpi;
    const Curve curve = curveAt(tableIndex(speed));
    AccelerationPoints result;
    // Counts per millisecond at kMaxSpeed, spread over the points.
    result.step = kMaxSpeed * dpi / 1000.0 / (kPoints - 1);
    result.points.reserve(kPoints);
    for (int i = 0; i < kPoints; ++i) {
        const double inchesPerSecond = kMaxSpeed * i / (kPoints - 1);
        result.points.push_back(pointerSpeed(curve, inchesPerSecond) / 1000.0);
    }
    return result;
}

int mouseDpi(const char *property)
{
    if (!property || !*property)
        return kDefaultDpi;
    const std::string entries(property);
    // With several resolutions the default is starred; with one it is that one.
    std::string chosen;
    size_t start = 0;
    while (start < entries.size()) {
        size_t end = entries.find(' ', start);
        if (end == std::string::npos)
            end = entries.size();
        const std::string entry = entries.substr(start, end - start);
        if (!entry.empty() && entry.front() == '*') {
            chosen = entry.substr(1);
            break;
        }
        if (!entry.empty() && chosen.empty())
            chosen = entry;
        start = end + 1;
    }
    const int dpi = std::atoi(chosen.c_str());
    return dpi > 0 ? dpi : kDefaultDpi;
}

} // namespace Lingmo::Input
