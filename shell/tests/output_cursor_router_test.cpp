// Copyright 2015 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.chromium file.
//
// Upstream: chromium/src/ash/display/extended_mouse_warp_controller_unittest.cc
// Revision: 7acd40ddefff49c9d36144999aafdbb674dbd07a
//
// Adapted for Lingmo: tests cover output-local cursor sinks and mirrored
// physical destinations.

#include <gtest/gtest.h>

#include "cursor/output_cursor_router.h"

using namespace Lingmo;

namespace {
class Sink final : public render::CursorSink
{
public:
    bool setCursorImage(const render::CursorImage&) override
    {
        ++images;
        return true;
    }

    void hideCursor() override { ++hides; }

    void moveCursor(const base::PointF& point) override { position = point; }

    int images = 0;
    int hides = 0;
    base::PointF position;
};
} // namespace

TEST(OutputCursorRouter, CrossingOutputsHidesOldPlaneAndUsesLocalCoordinates)
{
    Sink left;
    Sink right;
    cursor::OutputCursorRouter router;
    router.setTargets({{{0, 0, 100, 100}, &left}, {{100, 20, 200, 100}, &right}});
    uint32_t pixel = 0xffffffff;
    ASSERT_TRUE(
        router.setCursorImage({reinterpret_cast<const uint8_t*>(&pixel), base::Size(1, 1), 4, {}}));
    router.moveCursor({40, 30});
    EXPECT_EQ(left.position, base::PointF(40, 30));
    router.moveCursor({150, 50});
    EXPECT_EQ(left.hides, 1);
    EXPECT_EQ(right.images, 1);
    EXPECT_EQ(right.position, base::PointF(50, 30));
    EXPECT_EQ(router.activeTarget(), 1);
}

TEST(OutputCursorRouter, ShowsTheCursorOnEveryMirroredDestination)
{
    Sink first;
    Sink second;
    cursor::OutputCursorRouter router;
    router.setTargets({{{0, 0, 100, 100}, &first}, {{0, 0, 200, 200}, &second}});
    router.moveCursor({40, 30});
    uint32_t pixel = 0xffffffff;
    ASSERT_TRUE(
        router.setCursorImage({reinterpret_cast<const uint8_t*>(&pixel), base::Size(1, 1), 4, {}}));
    router.moveCursor({50, 40});
    EXPECT_EQ(first.position, base::PointF(50, 40));
    EXPECT_EQ(second.position, base::PointF(50, 40));
    EXPECT_EQ(first.images, 1);
    EXPECT_EQ(second.images, 1);
}

TEST(OutputCursorRouter, MapsCursorThroughEachMirrorDestination)
{
    Sink wide;
    Sink rotated;
    cursor::OutputCursorRouter router;
    router.setTargets({{{-100, 20, 100, 100},
                        &wide,
                        display::MirrorTransform({-100, 20, 100, 100}, {0, 50, 200, 200},
                                                 display::DisplayRotation::Normal)},
                       {{-100, 20, 100, 100},
                        &rotated,
                        display::MirrorTransform({-100, 20, 100, 100}, {10, 0, 100, 200},
                                                 display::DisplayRotation::Rotate90)}});
    router.moveCursor({-75, 70});
    uint32_t pixel = 0xffffffff;
    ASSERT_TRUE(
        router.setCursorImage({reinterpret_cast<const uint8_t*>(&pixel), base::Size(1, 1), 4, {}}));
    router.moveCursor({-50, 45});
    EXPECT_EQ(wide.position, base::PointF(100, 100));
    EXPECT_EQ(rotated.position, base::PointF(35, 100));
}

TEST(OutputCursorRouter, ReconfigurationKeepsAStationaryCursorVisible)
{
    Sink sink;
    cursor::OutputCursorRouter router;
    router.setTargets({{{0, 0, 100, 100}, &sink}});
    uint32_t pixel = 0xffffffff;
    ASSERT_TRUE(router.setCursorImage({reinterpret_cast<const uint8_t*>(&pixel), {1, 1}, 4, {}}));
    router.moveCursor({40, 30});
    router.setTargets({{{0, 0, 100, 100}, &sink,
        display::MirrorTransform({0, 0, 100, 100}, {0, 0, 200, 200}, display::DisplayRotation::Normal)}});
    EXPECT_EQ(sink.images, 2);
    EXPECT_EQ(sink.position, base::PointF(80, 60));
    EXPECT_EQ(router.activeTarget(), 0);
}

// An animated cursor sets a new image every frame without moving; the next move
// must neither blank the plane nor upload the image again.
TEST(OutputCursorRouter, AnimationFramesKeepOnePlaneActive)
{
    Sink sink;
    cursor::OutputCursorRouter router;
    router.setTargets({{{0, 0, 100, 100}, &sink}});
    router.moveCursor({40, 30});
    uint32_t pixel = 0xffffffff;
    for (int frame = 0; frame < 3; ++frame)
        ASSERT_TRUE(
            router.setCursorImage({reinterpret_cast<const uint8_t*>(&pixel), base::Size(1, 1), 4, {}}));
    router.moveCursor({40, 30});
    EXPECT_EQ(sink.images, 3);
    EXPECT_EQ(sink.hides, 0);
    EXPECT_EQ(sink.position, base::PointF(40, 30));
    EXPECT_EQ(router.activeTarget(), 0);
}

TEST(OutputCursorRouter, ImageBetweenOutputsStaysOnTheActiveOutput)
{
    Sink left;
    Sink right;
    cursor::OutputCursorRouter router;
    router.setTargets({{{0, 0, 100, 100}, &left}, {{200, 0, 100, 100}, &right}});
    router.moveCursor({250, 50});
    uint32_t pixel = 0xffffffff;
    ASSERT_TRUE(
        router.setCursorImage({reinterpret_cast<const uint8_t*>(&pixel), base::Size(1, 1), 4, {}}));
    router.moveCursor({150, 50});
    ASSERT_TRUE(
        router.setCursorImage({reinterpret_cast<const uint8_t*>(&pixel), base::Size(1, 1), 4, {}}));
    EXPECT_EQ(right.images, 2);
    EXPECT_EQ(right.hides, 0);
    EXPECT_EQ(left.images, 0);
    EXPECT_EQ(router.activeTarget(), 1);
}
