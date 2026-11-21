// Copyright 2016 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.chromium file.
//
// Upstream: chromium/src/ui/display/display_layout_unittest.cc
// Revision: 7acd40ddefff49c9d36144999aafdbb674dbd07a
//
// Adapted for Lingmo: uses opaque IDs, base geometry, and strict graph errors.

#include <gtest/gtest.h>

#include "display/display_layout.h"

using namespace Lingmo::Display;
namespace base = Lingmo::base;

namespace {

DisplayId id(const char* value)
{
    return DisplayId(std::string(value));
}

} // namespace

TEST(DisplayLayout, ResolvesNegativeCoordinatesAndNormalizesSharedEdge)
{
    const std::map<DisplayId, base::Size> sizes {
        {id("internal"), {100, 80}}, {id("left"), {60, 120}}, {id("top"), {50, 50}}};
    const std::vector<DisplayPlacement> placements {
        {id("left"), id("internal"), LayoutEdge::Left, -1000},
        {id("top"), id("internal"), LayoutEdge::Top, 80},
    };
    const auto result = DisplayLayout::resolve(id("internal"), sizes, placements);
    ASSERT_TRUE(result.succeeded) << result.error.message;
    EXPECT_EQ(result.bounds.at(id("internal")), base::Rect(0, 0, 100, 80));
    EXPECT_EQ(result.bounds.at(id("left")), base::Rect(-60, -88, 60, 120));
    EXPECT_EQ(result.bounds.at(id("top")), base::Rect(68, -50, 50, 50));
    EXPECT_EQ(result.placements.at(0).offset, -88);
    EXPECT_EQ(result.placements.at(1).offset, 68);
}

TEST(DisplayLayout, RerootsTheSameAdjacencyGraphAtNewPrimary)
{
    const std::map<DisplayId, base::Size> sizes {
        {id("a"), {100, 100}}, {id("b"), {80, 60}}, {id("c"), {40, 40}}};
    const std::vector<DisplayPlacement> placements {
        {id("b"), id("a"), LayoutEdge::Right, 20},
        {id("c"), id("b"), LayoutEdge::Bottom, 10},
    };
    const auto fromA = DisplayLayout::resolve(id("a"), sizes, placements);
    const auto fromB = DisplayLayout::resolve(id("b"), sizes, placements);
    ASSERT_TRUE(fromA.succeeded);
    ASSERT_TRUE(fromB.succeeded);
    EXPECT_EQ(fromB.bounds.at(id("b")).topLeft(), base::Point(0, 0));
    const base::Point translation = fromA.bounds.at(id("b")).topLeft();
    for (const auto& [display, size] : sizes)
        EXPECT_EQ(fromB.bounds.at(display).topLeft(),
                  fromA.bounds.at(display).topLeft() - translation);
}

TEST(DisplayLayout, RejectsDisconnectedCyclesMissingIdsAndOverlap)
{
    const std::map<DisplayId, base::Size> sizes {
        {id("a"), {100, 100}}, {id("b"), {100, 100}}, {id("c"), {100, 100}}};
    EXPECT_FALSE(DisplayLayout::resolve(id("a"), sizes,
                                        {{id("b"), id("a"), LayoutEdge::Right, 0},
                                         {id("a"), id("b"), LayoutEdge::Left, 0}})
                     .succeeded);
    EXPECT_FALSE(DisplayLayout::resolve(id("a"), sizes,
                                        {{id("b"), id("missing"), LayoutEdge::Right, 0},
                                         {id("c"), id("a"), LayoutEdge::Left, 0}})
                     .succeeded);
    EXPECT_FALSE(DisplayLayout::resolve(id("a"), sizes,
                                        {{id("b"), id("a"), LayoutEdge::Right, 0},
                                         {id("c"), id("a"), LayoutEdge::Right, 0}})
                     .succeeded);
}

TEST(DisplayLayout, SupportsSingleDisplay)
{
    const auto result = DisplayLayout::resolve(id("only"), {{id("only"), {1920, 1080}}}, {});
    ASSERT_TRUE(result.succeeded);
    EXPECT_EQ(result.bounds.at(id("only")), base::Rect(0, 0, 1920, 1080));
}
