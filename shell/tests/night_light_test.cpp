// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// Modelled on Ash's night_light_controller_unittest.cc, for the part the
// compositor keeps: the animation and the colour scales. The schedule is
// lingmo-services' and is tested there.

#include <gtest/gtest.h>

#include <vector>

#include "display/night_light.h"

using namespace Lingmo;
using namespace std::chrono_literals;

namespace {

class NightLightTest : public ::testing::Test
{
protected:
    NightLightTest()
        : m_nightLight(
                  [this](const render::ColorTransform &transform) {
                      m_applied.push_back(transform);
                  },
                  [this] {
                      return m_now;
                  })
    {
    }

    void advance(std::chrono::milliseconds by)
    {
        m_now += by;
        m_nightLight.stepForTesting();
    }

    std::chrono::steady_clock::time_point m_now {};
    std::vector<render::ColorTransform> m_applied;
    display::NightLight m_nightLight;
};

} // namespace

// Ash's MatrixFromTemperature: red unchanged, blue to 1 - t, green to 1 - t / 2.
TEST(NightLightScales, BlueGoesFirstAndGreenHalfAsFast)
{
    EXPECT_TRUE(display::NightLight::transformFor(0).isIdentity());
    const auto half = display::NightLight::transformFor(0.5);
    EXPECT_FLOAT_EQ(half.red, 1.0f);
    EXPECT_FLOAT_EQ(half.green, 0.75f);
    EXPECT_FLOAT_EQ(half.blue, 0.5f);
    const auto full = display::NightLight::transformFor(1);
    EXPECT_FLOAT_EQ(full.green, 0.5f);
    EXPECT_FLOAT_EQ(full.blue, 0.0f);
}

TEST_F(NightLightTest, WithoutADurationTheTintAppliesAtOnce)
{
    m_nightLight.animateTo(0.5, 0ms);

    EXPECT_FALSE(m_nightLight.isAnimating());
    EXPECT_DOUBLE_EQ(m_nightLight.strength(), 0.5);
    ASSERT_EQ(m_applied.size(), 1u);
    EXPECT_EQ(m_applied.back(), display::NightLight::transformFor(0.5));
}

TEST_F(NightLightTest, TheStrengthMovesLinearlyOverTheDuration)
{
    m_nightLight.animateTo(0.8, 1000ms);
    EXPECT_TRUE(m_nightLight.isAnimating());
    EXPECT_TRUE(m_applied.empty());

    advance(250ms);
    EXPECT_NEAR(m_nightLight.strength(), 0.2, 1e-9);
    advance(500ms);
    EXPECT_NEAR(m_nightLight.strength(), 0.6, 1e-9);
    EXPECT_TRUE(m_nightLight.isAnimating());

    advance(300ms);
    EXPECT_DOUBLE_EQ(m_nightLight.strength(), 0.8);
    EXPECT_FALSE(m_nightLight.isAnimating());
    EXPECT_EQ(m_applied.back(), display::NightLight::transformFor(0.8));
}

// AnimateToNewValue: a new target starts from wherever the animation is.
TEST_F(NightLightTest, ANewTargetStartsFromTheCurrentStrength)
{
    m_nightLight.animateTo(1, 1000ms);
    advance(500ms);
    ASSERT_NEAR(m_nightLight.strength(), 0.5, 1e-9);

    m_nightLight.animateTo(0, 1000ms);
    EXPECT_DOUBLE_EQ(m_nightLight.target(), 0);
    advance(500ms);
    EXPECT_NEAR(m_nightLight.strength(), 0.25, 1e-9);
    advance(500ms);
    EXPECT_DOUBLE_EQ(m_nightLight.strength(), 0);
    EXPECT_TRUE(m_applied.back().isIdentity());
}

TEST_F(NightLightTest, TheStrengthIsClampedToTheRange)
{
    m_nightLight.animateTo(3, 0ms);
    EXPECT_DOUBLE_EQ(m_nightLight.strength(), 1);
    m_nightLight.animateTo(-1, 0ms);
    EXPECT_DOUBLE_EQ(m_nightLight.strength(), 0);
}

TEST_F(NightLightTest, AnimatingToTheCurrentStrengthDoesNotTick)
{
    m_nightLight.animateTo(0.5, 0ms);
    m_nightLight.animateTo(0.5, 1000ms);
    EXPECT_FALSE(m_nightLight.isAnimating());
}
