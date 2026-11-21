// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <cmath>

#include <gtest/gtest.h>

#include "input/pointer_acceleration.h"

using namespace Lingmo::Input;

namespace {

// libinput's custom profile interpolates linearly between the points.
double pointerSpeed(const AccelerationPoints &curve, double countsPerMs)
{
    const double position = countsPerMs / curve.step;
    const size_t index = size_t(position);
    if (index + 1 >= curve.points.size())
        return curve.points.back();
    const double t = position - index;
    return curve.points[index] * (1 - t) + curve.points[index + 1] * t;
}

} // namespace

TEST(PointerAcceleration, FollowsTheParametricCurve)
{
    // The default curve worked by hand from its parameters, for a hand speed
    // on each segment: the polynomial (1 and 4 in/s, standardized 1.34 and
    // 5.36), the tangent line (8 in/s) and the square root (16 in/s).
    EXPECT_NEAR(pointerGain(1, 0), 389, 4);
    EXPECT_NEAR(pointerGain(4, 0), 886, 9);
    EXPECT_NEAR(pointerGain(8, 0), 1504, 15);
    EXPECT_NEAR(pointerGain(16, 0), 1860, 19);
}

TEST(PointerAcceleration, SlowMotionIsPreciseAndAFlickCrossesTheScreen)
{
    // A crawl moves a couple of hundred pixels per inch of hand travel, a
    // flick well over a thousand, and the gain climbs between them without a
    // step, across the joins between segments too.
    EXPECT_LT(pointerGain(0.25, 0), 300);
    EXPECT_GT(pointerGain(12, 0), 1500);
    // Past the square root join the gain eases off again while the pointer
    // still speeds up: a flick goes far without running away.
    double previousGain = pointerGain(0.01, 0);
    double previousSpeed = 0;
    for (double v = 0.05; v <= 20; v += 0.05) {
        const double gain = pointerGain(v, 0);
        EXPECT_GT(gain * v, previousSpeed) << v;
        EXPECT_LT(std::abs(gain / previousGain - 1), 0.05) << v;
        previousGain = gain;
        previousSpeed = gain * v;
    }
    EXPECT_LT(pointerGain(20, 0), pointerGain(16, 0));
    // The tangent speeds of the default curve, in hand speed.
    for (double join : {8.4 / 1.34, 17 / 1.34})
        EXPECT_NEAR(pointerGain(join - 1e-4, 0), pointerGain(join + 1e-4, 0), 0.5) << join;
}

TEST(PointerAcceleration, TheSliderPicksAGentlerOrSteeperCurve)
{
    for (double v : {0.5, 2.0, 8.0}) {
        double previous = 0;
        for (double speed : {-1.0, -0.5, 0.0, 0.5, 1.0}) {
            const double gain = pointerGain(v, speed);
            EXPECT_GT(gain, previous) << v << " at " << speed;
            previous = gain;
        }
        EXPECT_DOUBLE_EQ(pointerGain(v, 5), pointerGain(v, 1));
        EXPECT_DOUBLE_EQ(pointerGain(v, -5), pointerGain(v, -1));
    }
}

TEST(PointerAcceleration, EachSliderStepIsATableEntry)
{
    // The first step is the linear entry: the same gain at every speed.
    EXPECT_NEAR(pointerGain(0.5, -1), 240, 1);
    EXPECT_NEAR(pointerGain(4, -1), 240, 1);
    // The fifth, one sixth above the default, is the 0.875 entry, worked by
    // hand at 1 in/s: 179.1 * (0.95 * 1.34 + (0.88 * 1.34)^2 + (0.15 * 1.34)^3).
    EXPECT_NEAR(pointerGain(1, 1.0 / 6), 478, 5);
    // Halfway between two steps is halfway between their entries.
    EXPECT_GT(pointerGain(1, 1.0 / 12), pointerGain(1, 0));
    EXPECT_LT(pointerGain(1, 1.0 / 12), pointerGain(1, 1.0 / 6));
}

TEST(PointerAcceleration, TheSameHandSpeedMovesThePointerAlikeAtEveryResolution)
{
    for (int dpi : {400, 800, 1000, 1600, 3200}) {
        const auto curve = accelerationPoints(dpi, 0);
        ASSERT_EQ(curve.points.size(), 64u);
        EXPECT_EQ(curve.points.front(), 0);
        for (size_t i = 1; i < curve.points.size(); ++i)
            EXPECT_GT(curve.points[i], curve.points[i - 1]) << dpi;
        for (double inchesPerSecond : {0.5, 3.0, 10.0}) {
            // Counts per millisecond this mouse reports at that hand speed.
            const double counts = inchesPerSecond * dpi / 1000.0;
            const double expected = inchesPerSecond * pointerGain(inchesPerSecond, 0) / 1000.0;
            EXPECT_NEAR(pointerSpeed(curve, counts), expected, expected * 0.01)
                    << dpi << " dpi at " << inchesPerSecond;
        }
    }
    // Unknown resolution: libinput's default.
    EXPECT_EQ(accelerationPoints(0, 0).step, accelerationPoints(1000, 0).step);
}

TEST(PointerAcceleration, MouseDpiReadsTheDefaultResolution)
{
    EXPECT_EQ(mouseDpi(nullptr), 1000);
    EXPECT_EQ(mouseDpi(""), 1000);
    EXPECT_EQ(mouseDpi("1000@125"), 1000);
    EXPECT_EQ(mouseDpi("800"), 800);
    EXPECT_EQ(mouseDpi("400 *800 1600@125"), 800);
    EXPECT_EQ(mouseDpi("400 800 *1600@1000"), 1600);
    EXPECT_EQ(mouseDpi("400 1600"), 400);
    EXPECT_EQ(mouseDpi("garbage"), 1000);
}
