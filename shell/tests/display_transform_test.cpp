// Copyright 2013 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.chromium file.
//
// Upstream: chromium/src/ash/display/root_window_transformers_unittest.cc
// Revision: 7acd40ddefff49c9d36144999aafdbb674dbd07a
//
// Adapted for Lingmo: tests the direct global-logical/native transform contract.

#include <gtest/gtest.h>

#include "display/display_transform.h"

using namespace Lingmo;

TEST(DisplayTransform, FractionalScaleUsesRotatedLogicalExtent)
{
    const auto scale = *compositor::OutputScale::fromNumerator(150);
    display::DisplayTransform normal({-300, 40}, {1920, 1080}, scale,
                                     display::DisplayRotation::Normal);
    display::DisplayTransform rotated({0, 0}, {1920, 1080}, scale,
                                      display::DisplayRotation::Rotate90);
    EXPECT_EQ(normal.logicalBounds(), base::Rect(-300, 40, 1536, 864));
    EXPECT_EQ(rotated.logicalSize(), base::Size(864, 1536));
    EXPECT_TRUE(normal.containsGlobal(base::PointF(-300, 40)));
    EXPECT_FALSE(normal.containsGlobal(base::PointF(1236, 40)));
}

TEST(DisplayTransform, EveryRotationRoundTripsPointsAndRectangles)
{
    const auto scale = *compositor::OutputScale::fromNumerator(180);
    for (const auto rotation :
         {display::DisplayRotation::Normal, display::DisplayRotation::Rotate90,
          display::DisplayRotation::Rotate180, display::DisplayRotation::Rotate270}) {
        display::DisplayTransform transform({-640, 120}, {1513, 819}, scale, rotation);
        const base::PointF global(-500.25, 300.75);
        const base::PointF roundTrip = transform.nativeToGlobal(transform.globalToNative(global));
        EXPECT_NEAR(roundTrip.x(), global.x(), 0.0001);
        EXPECT_NEAR(roundTrip.y(), global.y(), 0.0001);

        const base::RectF rectangle(-510.5, 150.25, 80.75, 100.5);
        const base::RectF mapped = transform.nativeToGlobal(transform.globalToNative(rectangle));
        EXPECT_NEAR(mapped.x(), rectangle.x(), 0.0001);
        EXPECT_NEAR(mapped.y(), rectangle.y(), 0.0001);
        EXPECT_NEAR(mapped.width(), rectangle.width(), 0.0001);
        EXPECT_NEAR(mapped.height(), rectangle.height(), 0.0001);
    }
}

TEST(DisplayTransform, AdjacentHalfOpenBoundsHaveOneOwner)
{
    display::DisplayTransform left({-100, 0}, {100, 100}, {}, display::DisplayRotation::Normal);
    display::DisplayTransform right({0, 0}, {100, 100}, {}, display::DisplayRotation::Normal);
    EXPECT_TRUE(left.containsGlobal(base::PointF(-0.001, 50)));
    EXPECT_FALSE(left.containsGlobal(base::PointF(0, 50)));
    EXPECT_TRUE(right.containsGlobal(base::PointF(0, 50)));
}
