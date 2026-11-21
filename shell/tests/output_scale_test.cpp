// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gtest/gtest.h>

#include "compositor/compositor.h"
#include "compositor/output.h"
#include "input/input_manager.h"
#include "input/seat.h"

using namespace Lingmo;

TEST(OutputScale, ParsingCanonicalValuesAndLimits)
{
    for (const auto& [text, numerator] : std::vector<std::pair<std::string, uint32_t>> {
             {"1.0", 120}, {"1.25", 150}, {"1.5", 180}, {"1.75", 210}, {"2.0", 240}, {"4", 480}}) {
        const auto scale = compositor::OutputScale::parse(text);
        ASSERT_TRUE(scale);
        EXPECT_EQ(scale->numerator(), numerator);
        EXPECT_DOUBLE_EQ(scale->value(), numerator / 120.0);
        EXPECT_EQ(scale->integerScale(), int((numerator + 119) / 120));
    }
    for (const auto& text : {"", "nan", "inf", "-1", "0", "0.99", "4.01", "1e0", "1.5x", " 1", "1.",
                             "+1", "99999999999999999999999999"})
        EXPECT_FALSE(compositor::OutputScale::parse(text)) << text;
    EXPECT_EQ(compositor::OutputScale::parse("1.5001"), compositor::OutputScale::parse("1.5"));
    EXPECT_EQ(compositor::OutputScale::parse("1.5\n"), compositor::OutputScale::parse("1.5"));
    EXPECT_FALSE(compositor::OutputScale::parse("1.5\n\n"));
    EXPECT_FALSE(compositor::OutputScale::fromNumerator(0));
    EXPECT_FALSE(compositor::OutputScale::fromNumerator(UINT32_MAX));
}

TEST(OutputScale, OutputRoundingAndConservativeDeviceCoverage)
{
    const auto scale = *compositor::OutputScale::parse("1.5");
    EXPECT_EQ(scale.logicalSize({1920, 1080}), base::Size(1280, 720));
    EXPECT_EQ(scale.logicalSize({1512, 817}), base::Size(1008, 545));
    EXPECT_EQ(scale.logicalSize({1513, 819}), base::Size(1009, 546));
    EXPECT_EQ(scale.deviceCoverage({100, 50, 800, 600}), base::Rect(150, 75, 1200, 900));
    EXPECT_EQ(scale.deviceCoverage({0.25, 0.25, 1, 1}), base::Rect(0, 0, 2, 2));
    for (uint32_t numerator : {120u, 150u, 180u, 210u, 240u}) {
        const auto value = *compositor::OutputScale::fromNumerator(numerator);
        for (base::Size native : {base::Size(1920, 1080), base::Size(1512, 817),
                                  base::Size(1513, 819)}) {
            const auto logical = value.logicalSize(native);
            const auto coverage = value.deviceCoverage(base::RectF(base::SizeF(logical)));
            EXPECT_TRUE(coverage.contains(base::Rect(native)));
            EXPECT_LT(coverage.width() - native.width(), std::ceil(value.value()) + 1);
        }
    }
}

TEST(OutputScale, DeviceInputAndSceneUseSameMapping)
{
    compositor::Compositor compositor(nullptr);
    input::Seat seat(&compositor);
    input::InputManager input(&seat);
    const auto scale = *compositor::OutputScale::parse("1.5");
    input.setDeviceBounds({1920, 1080});
    input.setDeviceScale(scale.value());
    input.setBounds(scale.logicalSize({1920, 1080}));
    input.onPointerMotionAbsolute({}, {450, 300});
    EXPECT_EQ(input.pointerPosition(), base::PointF(300, 200));
    EXPECT_EQ(input.pointerPosition() - base::PointF(100, 50), base::PointF(200, 150));
    input.onPointerMotion({}, {150, 75});
    EXPECT_EQ(input.pointerPosition(), base::PointF(400, 250));
    input.onPointerMotionAbsolute({}, {99999, 99999});
    EXPECT_EQ(input.pointerPosition(), base::PointF(1919 / 1.5, 1079 / 1.5));
    EXPECT_EQ(input.deviceBounds(), base::Size(1920, 1080));
    input.setDeviceScale(2);
    input.setBounds({960, 540});
    EXPECT_EQ(input.pointerPosition(), base::PointF(959.5, 539.5));
    EXPECT_EQ(seat.cursorPosition(), input.pointerPosition());
}
