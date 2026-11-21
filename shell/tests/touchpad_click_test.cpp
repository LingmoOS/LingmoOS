// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gtest/gtest.h>

#include "input/touchpad_click.h"

using namespace Lingmo::Input;
using namespace std::chrono_literals;

namespace {

// A 116 x 61 mm clickpad; the presses below were recorded on it.
const Lingmo::base::SizeF kPad(116.2, 60.9);
const Timestamp kPress = Timestamp(10s);

TouchpadContact finger(Lingmo::base::PointF position, std::chrono::milliseconds age)
{
    return {position, position, kPress - age};
}

} // namespace

TEST(TouchpadClick, TakesStaggeredFingersForTwo)
{
    // Index and middle finger laid diagonally, the lower one near the bottom:
    // libinput counts one finger for both.
    EXPECT_TRUE(isTwoFingerClick(finger({47.1, 41.5}, 62ms), finger({60.9, 21.2}, 46ms), kPress,
                                 kPad));
    EXPECT_TRUE(isTwoFingerClick(finger({62.4, 7.0}, 68ms), finger({34.0, 40.6}, 68ms), kPress,
                                 kPad));
    // Far apart in time, but neither finger rests at the bottom edge.
    EXPECT_TRUE(isTwoFingerClick(finger({34.9, 41.0}, 713ms), finger({62.0, 8.6}, 137ms), kPress,
                                 kPad));
}

TEST(TouchpadClick, KeepsFarApartFingersApart)
{
    EXPECT_FALSE(isTwoFingerClick(finger({10.8, 34.2}, 62ms), finger({101.8, 28.5}, 45ms), kPress,
                                  kPad));
    EXPECT_FALSE(isTwoFingerClick(finger({50.0, 5.0}, 60ms), finger({50.0, 52.0}, 60ms), kPress,
                                  kPad));
    EXPECT_FALSE(isTwoFingerClick(finger({30.0, 30.0}, 60ms), finger({77.0, 30.0}, 60ms), kPress,
                                  kPad));
}

TEST(TouchpadClick, IgnoresARestingThumb)
{
    const TouchpadContact thumb = finger({20.0, 55.0}, 800ms);
    EXPECT_FALSE(isTwoFingerClick(thumb, finger({50.0, 40.0}, 150ms), kPress, kPad));

    // A thumb that came down with the finger is part of the click.
    EXPECT_TRUE(isTwoFingerClick(finger({20.0, 55.0}, 60ms), finger({50.0, 40.0}, 50ms), kPress,
                                 kPad));

    // One that moved across the pad was pointing, not resting.
    TouchpadContact moved = thumb;
    moved.origin = Lingmo::base::PointF(60.0, 30.0);
    EXPECT_TRUE(isTwoFingerClick(moved, finger({50.0, 40.0}, 150ms), kPress, kPad));

    // Stacked under a finger that came later, it is a thumb however new.
    EXPECT_FALSE(isTwoFingerClick(finger({45.0, 55.0}, 300ms), finger({50.0, 30.0}, 150ms),
                                  kPress, kPad));
}

TEST(TouchpadClick, IgnoresAFingerFromThePalmEdge)
{
    TouchpadContact palm = finger({30.0, 30.0}, 60ms);
    palm.origin = Lingmo::base::PointF(3.0, 30.0);
    EXPECT_FALSE(isTwoFingerClick(palm, finger({45.0, 25.0}, 60ms), kPress, kPad));
}
