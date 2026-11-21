#include <gtest/gtest.h>

#include <chrono>
#include <limits>

#include "cadence/cadence.h"

using namespace Cadence;
using namespace std::chrono_literals;

TEST(Easing, ClampsInputAndReachesEndpoints)
{
    EXPECT_DOUBLE_EQ(0, ease(Easing::OutCubic, -1));
    EXPECT_DOUBLE_EQ(1, ease(Easing::OutCubic, 1));
    EXPECT_DOUBLE_EQ(1, ease(Easing::OutCubic, 2));
    EXPECT_GT(ease(Easing::OutBack, 0.7), 1);
    EXPECT_DOUBLE_EQ(0, ease(Easing::Linear, std::numeric_limits<double>::quiet_NaN()));
}

TEST(Timeline, AdvancesRepeatsAndAlternates)
{
    Timeline timeline;
    timeline.setDuration(100ns);
    timeline.setEasing(Easing::Linear);
    timeline.setLoops(2);
    timeline.setAlternating(true);
    timeline.start(1000ns);

    EXPECT_TRUE(timeline.advance(1050ns));
    EXPECT_DOUBLE_EQ(0.5, timeline.progress());
    EXPECT_TRUE(timeline.advance(1150ns));
    EXPECT_DOUBLE_EQ(0.5, timeline.progress());
    EXPECT_FALSE(timeline.advance(1200ns));
    EXPECT_DOUBLE_EQ(0, timeline.progress());
}

TEST(Timeline, InfiniteLoopsRemainDefinedAtLargeElapsedTimes)
{
    Timeline timeline;
    timeline.setDuration(1ns);
    timeline.setLoops(Timeline::Infinite);
    timeline.setEasing(Easing::Linear);
    timeline.start(0ns);

    EXPECT_TRUE(timeline.advance(std::chrono::nanoseconds::max()));
    EXPECT_DOUBLE_EQ(0, timeline.progress());
}

TEST(Timeline, PauseAndResumePreserveElapsedTime)
{
    Timeline timeline;
    timeline.setDuration(1000ns);
    timeline.setEasing(Easing::Linear);
    ManualClock clock(100ns);
    timeline.start(clock);
    clock.advance(300ns);
    timeline.advance(clock);
    timeline.pause(clock);
    clock.advance(300ns);
    EXPECT_FALSE(timeline.advance(clock));
    timeline.resume(clock);
    clock.advance(100ns);
    timeline.advance(clock);
    EXPECT_DOUBLE_EQ(0.4, timeline.progress());
}

TEST(Tween, RetargetsValuesAndInterpolatesColor)
{
    Tween<double> number;
    number.setDuration(100ns);
    number.setEasing(Easing::Linear);
    number.setValue(2);
    number.animateTo(10, 0ns);
    number.advance(50ns);
    EXPECT_DOUBLE_EQ(6, number.value());

    Tween<Color> color;
    color.setDuration(100ns);
    color.setEasing(Easing::Linear);
    color.animateTo({255, 100, 0, 255}, {0, 0, 0, 255}, 0ns);
    color.advance(50ns);
    EXPECT_EQ((Color{128, 50, 0, 255}), color.value());
}

TEST(Tween, InterpolatesGeometry)
{
    Tween<Rect> tween;
    tween.setDuration(100ns);
    tween.setEasing(Easing::Linear);
    tween.animateTo({20, 40, 80, 100}, {0, 10, 20, 30}, 0ns);
    tween.advance(50ns);
    EXPECT_EQ((Rect{10, 25, 50, 65}), tween.value());
}

TEST(Interpolator, ClampsIntegralAndColorOvershoot)
{
    EXPECT_EQ(std::numeric_limits<int>::max(),
              Interpolator<int>::mix(0, std::numeric_limits<int>::max(), 2));
    EXPECT_EQ((Color{255, 255, 255, 255}), Interpolator<Color>::mix({0, 0, 0, 0}, {255, 255, 255, 255}, 2));
}

TEST(Keyframes, SamplesSegmentsWithTheirOwnEasing)
{
    KeyframeTrack<double> track;
    ASSERT_TRUE(track.setKeyframes({{0, 0, Easing::OutQuad}, {0.5, 10, Easing::Linear}, {1, 20}}));

    ASSERT_TRUE(track.sample(-1));
    EXPECT_DOUBLE_EQ(0, *track.sample(-1));
    EXPECT_DOUBLE_EQ(7.5, *track.sample(0.25));
    EXPECT_DOUBLE_EQ(15, *track.sample(0.75));
    EXPECT_DOUBLE_EQ(20, *track.sample(2));
}

TEST(Keyframes, RejectsInvalidOffsetsWithoutReplacingValidFrames)
{
    KeyframeTrack<double> track;
    ASSERT_TRUE(track.setKeyframes({{0, 2}, {1, 8}}));
    EXPECT_FALSE(track.setKeyframes({{0, 1}, {0, 2}}));
    EXPECT_FALSE(track.setKeyframes({{0.8, 1}, {0.2, 2}}));
    EXPECT_FALSE(track.setKeyframes({{0, 1}, {1.1, 2}}));
    EXPECT_FALSE(track.sample(std::numeric_limits<double>::quiet_NaN()));
    EXPECT_DOUBLE_EQ(5, *track.sample(0.5));
}

TEST(Spring, SettlesTowardsItsTarget)
{
    Spring spring;
    spring.snap(0);
    spring.setTarget(1);
    for (int i = 0; i < 240; ++i)
        spring.advance(std::chrono::duration<double>(1.0 / 60.0));
    EXPECT_NEAR(1, spring.value(), 0.001);
    EXPECT_TRUE(spring.isSettled());
}

TEST(Spring, HandlesInvalidFrequencyAndInfiniteTimeStep)
{
    Spring spring(std::numeric_limits<double>::infinity());
    spring.snap(0);
    spring.setTarget(5);
    spring.advance(std::chrono::duration<double>::infinity());
    EXPECT_DOUBLE_EQ(5, spring.value());
    EXPECT_TRUE(spring.isSettled());
}

TEST(Tween, RetargetStartsFromTheCurrentValue)
{
    Tween<double> tween;
    tween.setDuration(100ns);
    tween.setEasing(Easing::Linear);
    tween.animateTo(10, 0ns);
    tween.advance(40ns);
    tween.animateTo(0, 40ns);
    tween.advance(70ns);
    EXPECT_DOUBLE_EQ(6, tween.value());
}

TEST(Clock, ManualClockAdvancesDeterministically)
{
    ManualClock clock(5ns);
    clock.advance(12ns);
    EXPECT_EQ(17ns, clock.now());
}
