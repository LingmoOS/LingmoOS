// Copyright 2015 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.chromium file.
//
// Upstream: chromium/src/ash/display/root_window_transformers_unittest.cc
// Revision: 7acd40ddefff49c9d36144999aafdbb674dbd07a
//
// Adapted for Lingmo: tests reversible mirror input mapping across native
// aspect ratios, letterboxing, negative origins, and output rotations.

#include <gtest/gtest.h>

#include "display/mirror_transform.h"

using namespace Lingmo;

TEST(MirrorTransform, MapsLogicalSourceIntoLetterboxedDestination)
{
    display::MirrorTransform transform({-1280, 40, 1280, 720}, {0, 240, 3840, 2160},
                                       display::DisplayRotation::Normal);
    EXPECT_EQ(transform.logicalToNative({-640, 400}), base::PointF(1920, 1320));
    const auto logical = transform.nativeToLogical({1920, 1320});
    ASSERT_TRUE(logical);
    EXPECT_EQ(*logical, base::PointF(-640, 400));
    EXPECT_FALSE(transform.nativeToLogical({20, 100}));
}

TEST(MirrorTransform, RoundTripsEveryRotation)
{
    for (const auto rotation :
         {display::DisplayRotation::Normal, display::DisplayRotation::Rotate90,
          display::DisplayRotation::Rotate180, display::DisplayRotation::Rotate270}) {
        display::MirrorTransform transform({0, 0, 1600, 900}, {100, 50, 1800, 1000}, rotation);
        const base::PointF source(240.25, 600.75);
        const auto roundTrip = transform.nativeToLogical(transform.logicalToNative(source));
        ASSERT_TRUE(roundTrip);
        EXPECT_NEAR(roundTrip->x(), source.x(), 0.0001);
        EXPECT_NEAR(roundTrip->y(), source.y(), 0.0001);
    }
}

TEST(MirrorTransform, APixelAtAFractionalScaleComesBackToTheSamePixel)
{
    // The cursor takeover puts the pointer on a device pixel through the
    // inverse, and the plane is placed through the forward mapping, rounded:
    // at 150% the round trip has to land on that same pixel, or the cursor
    // moves by one at every hand-over.
    display::MirrorTransform transform({0, 0, 1440, 960}, {0, 0, 2160, 1440},
                                       display::DisplayRotation::Normal);
    for (int x = 0; x < 2160; x += 7) {
        for (int y = 0; y < 1440; y += 11) {
            const auto logical = transform.nativeToLogical(base::PointF(x, y));
            ASSERT_TRUE(logical);
            EXPECT_EQ(transform.logicalToNative(*logical).toPoint(), base::Point(x, y));
        }
    }
}
