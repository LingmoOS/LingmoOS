// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gtest/gtest.h>

#include <algorithm>
#include <cmath>
#include <numbers>

#include "scene/motion.h"

using namespace Lingmo::Scene::Motion;

namespace {

// Central difference of the position, in units per second.
double numericVelocity(const Spring &spring, double time)
{
    constexpr double h = 0.01;
    return (spring.value(time + h) - spring.value(time - h)) / (2 * h / 1000);
}

} // namespace

TEST(MotionTest, CriticallyDampedSpringArrivesWithoutOvershoot)
{
    Spring spring({0.4, 1});
    spring.reset(1000, 0, 100);
    double previous = spring.value(1000);
    EXPECT_DOUBLE_EQ(previous, 0);
    for (double t = 1000; t <= 2000; t += 1) {
        const double value = spring.value(t);
        EXPECT_GE(value, previous - 1e-9);
        EXPECT_LE(value, 100 + 1e-9);
        previous = value;
    }
    // The response is about the time to cover 99% of the way from rest.
    EXPECT_GT(spring.value(1000 + 400), 98.5);
    EXPECT_LT(spring.value(1000 + 400), 99.5);
    EXPECT_NEAR(spring.value(10000), 100, 1e-9);
}

TEST(MotionTest, StartsFromRestAndStaysStillBeforeItsStart)
{
    Spring spring({0.3, 1});
    spring.reset(500, 10, 50);
    EXPECT_DOUBLE_EQ(spring.value(400), 10);
    EXPECT_DOUBLE_EQ(spring.velocity(500), 0);
    EXPECT_GT(spring.value(516), 10);
}

TEST(MotionTest, ClosedFormVelocityMatchesThePath)
{
    for (const double zeta : {0.3, 0.7, 1.0, 1.4, 3.0}) {
        Spring spring({0.35, zeta});
        spring.reset(0, -40, 60, 900);
        for (double t = 5; t < 800; t += 37)
            EXPECT_NEAR(spring.velocity(t), numericVelocity(spring, t), 1e-3) << zeta << " " << t;
    }
}

TEST(MotionTest, SamplingDoesNotChangeTheTrajectory)
{
    Spring spring({0.35, 0.8});
    spring.reset(0, 0, 1);
    const double once = spring.value(333.33);
    for (double t = 0; t < 333.33; t += 13.333)
        spring.value(t);
    EXPECT_DOUBLE_EQ(spring.value(333.33), once);
}

TEST(MotionTest, RetargetKeepsPositionAndVelocity)
{
    Spring spring({0.4, 1});
    spring.reset(0, 0, 800);
    const double value = spring.value(120);
    const double velocity = spring.velocity(120);
    ASSERT_GT(velocity, 0);
    spring.retarget(120, -100);
    EXPECT_NEAR(spring.value(120), value, 1e-9);
    EXPECT_NEAR(spring.velocity(120), velocity, 1e-9);
    // It keeps going the old way for a moment before turning around.
    EXPECT_GT(spring.value(130), value);
    EXPECT_NEAR(spring.value(5000), -100, 1e-6);
}

TEST(MotionTest, UnderdampedSpringOvershootsByTheDampingRatio)
{
    const double zeta = 0.5;
    Spring spring({0.4, zeta});
    spring.reset(0, 0, 1);
    double peak = 0;
    for (double t = 0; t < 2000; t += 0.25)
        peak = std::max(peak, spring.value(t));
    const double expected = std::exp(-std::numbers::pi * zeta / std::sqrt(1 - zeta * zeta));
    EXPECT_NEAR(peak - 1, expected, 1e-4);
}

TEST(MotionTest, ZeroResponseJumpsToTheTarget)
{
    Spring spring({0, 1});
    spring.reset(0, 3, 9, 100);
    EXPECT_DOUBLE_EQ(spring.value(0), 9);
    EXPECT_DOUBLE_EQ(spring.velocity(0), 0);
}

TEST(MotionTest, TweenLandsOnTimeWithoutOvershoot)
{
    const Tween tween(100, 2, 6, 300);
    EXPECT_DOUBLE_EQ(2, tween.value(100));
    EXPECT_NEAR(0, tween.velocity(100), 1e-6);
    double last = 2;
    for (double time = 100; time < 400; time += 5) {
        const double value = tween.value(time);
        EXPECT_GE(value, last);
        EXPECT_LE(value, 6);
        last = value;
    }
    EXPECT_FALSE(tween.isFinished(399));
    EXPECT_TRUE(tween.isFinished(400));
    EXPECT_DOUBLE_EQ(6, tween.value(400));
}

TEST(MotionTest, TweenLeavesAtItsStartVelocity)
{
    // 10 a second over 4 in 0.3 s: three quarters of the average speed.
    const Tween tween(0, 2, 6, 300, 10);
    EXPECT_NEAR(10, tween.velocity(0), 1e-6);
    for (const double time : {20.0, 150.0, 280.0}) {
        const double numeric = (tween.value(time + 0.01) - tween.value(time - 0.01)) / 0.02 * 1000;
        EXPECT_NEAR(numeric, tween.velocity(time), 1e-3 * std::max(1.0, std::abs(numeric)));
    }
    EXPECT_TRUE(tween.isFinished(300));
}

TEST(MotionTest, ATweenOnQuickStartIsUnderWayAtOnceWithoutLunging)
{
    // 2.5 a second on average over 400 ms.
    const Tween tween(0, 0, 1, 400, 0, kQuickStart);
    EXPECT_NEAR(0, tween.velocity(0), 1e-6);
    EXPECT_GT(tween.value(60), 0.1);
    EXPECT_GT(tween.value(200), 0.7);
    double fastest = 0;
    for (double time = 0; time < 400; time += 1)
        fastest = std::max(fastest, tween.velocity(time));
    EXPECT_LT(fastest, 1.9 * 2.5);
    EXPECT_LT(tween.velocity(380), tween.velocity(200));
    EXPECT_TRUE(tween.isFinished(400));

    // Bent to leave at a speed, it still slows into its landing.
    const Tween moving(0, 0, 1, 400, 4, kQuickStart);
    EXPECT_NEAR(4, moving.velocity(0), 1e-6);
    for (const double time : {20.0, 200.0, 380.0}) {
        const double numeric =
                (moving.value(time + 0.01) - moving.value(time - 0.01)) / 0.02 * 1000;
        EXPECT_NEAR(numeric, moving.velocity(time), 1e-3 * std::max(1.0, std::abs(numeric)));
    }
    EXPECT_LT(moving.velocity(380), moving.velocity(200));
    EXPECT_LT(moving.velocity(380), 0.5);

    // Bent to a slow start, it speeds up to under twice the average.
    const Tween slow(0, 0, 1, 400, 1, kQuickStart);
    fastest = 0;
    for (double time = 0; time < 400; time += 1)
        fastest = std::max(fastest, slow.velocity(time));
    EXPECT_LT(fastest, 2 * 2.5);
}

TEST(MotionTest, AFastStartShortensTheTweenAndABackwardOneIsDropped)
{
    // 200 a second would need a start five times the average over 300 ms.
    const Tween fast(0, 0, 4, 300, 200);
    EXPECT_NEAR(200, fast.velocity(0), 1e-6);
    EXPECT_TRUE(fast.isFinished(100));
    EXPECT_LE(fast.value(50), 4);

    const Tween backward(0, 0, 4, 300, -20);
    EXPECT_NEAR(0, backward.velocity(0), 1e-6);
    EXPECT_FALSE(backward.isFinished(299));
}
