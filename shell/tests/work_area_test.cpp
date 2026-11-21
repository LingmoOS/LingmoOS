// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gtest/gtest.h>

#include "base/geometry.h"
#include "wm/stacking.h"
#include "wm/work_area.h"

using namespace Lingmo;
using namespace Lingmo::Wm;

namespace {

const base::Rect kOutput(0, 0, 1920, 1080);

TEST(ExclusiveZonesTest, NoReservationsLeaveTheOutputAlone)
{
    ExclusiveZones zones;
    EXPECT_TRUE(zones.isEmpty());
    EXPECT_EQ(kOutput, zones.apply(kOutput));
}

TEST(ExclusiveZonesTest, TopAndBottomCompose)
{
    ExclusiveZones zones;
    const int statusBar = 1;
    const int dock = 2;
    EXPECT_TRUE(zones.set(&statusBar, ScreenEdge::Top, 30));
    EXPECT_TRUE(zones.set(&dock, ScreenEdge::Bottom, 70));
    EXPECT_EQ(base::Rect(0, 30, 1920, 980), zones.apply(kOutput));
}

TEST(ExclusiveZonesTest, TheLargestReservationOnAnEdgeWins)
{
    ExclusiveZones zones;
    const int a = 1;
    const int b = 2;
    zones.set(&a, ScreenEdge::Top, 30);
    zones.set(&b, ScreenEdge::Top, 50);
    EXPECT_EQ(base::Rect(0, 50, 1920, 1030), zones.apply(kOutput));
}

TEST(ExclusiveZonesTest, SettingTheSameValueTwiceIsNotAChange)
{
    ExclusiveZones zones;
    const int token = 1;
    EXPECT_TRUE(zones.set(&token, ScreenEdge::Left, 60));
    EXPECT_FALSE(zones.set(&token, ScreenEdge::Left, 60));
    EXPECT_TRUE(zones.set(&token, ScreenEdge::Left, 80));
}

TEST(ExclusiveZonesTest, ZeroThicknessReleasesTheReservation)
{
    ExclusiveZones zones;
    const int token = 1;
    zones.set(&token, ScreenEdge::Bottom, 70);
    EXPECT_TRUE(zones.set(&token, ScreenEdge::Bottom, 0));
    EXPECT_TRUE(zones.isEmpty());
    EXPECT_EQ(kOutput, zones.apply(kOutput));
}

TEST(ExclusiveZonesTest, AYieldingReservationGivesWayToMaximizedWindows)
{
    ExclusiveZones zones;
    const int statusBar = 1;
    const int dock = 2;
    zones.set(&statusBar, ScreenEdge::Top, 30);
    EXPECT_FALSE(zones.hasYieldingZone());
    EXPECT_TRUE(zones.set(&dock, ScreenEdge::Bottom, 70, {}, true));
    EXPECT_TRUE(zones.hasYieldingZone());
    EXPECT_EQ(base::Rect(0, 30, 1920, 980), zones.apply(kOutput, {}, true, false));
    EXPECT_EQ(base::Rect(0, 30, 1920, 1050), zones.apply(kOutput, {}, true, true));
    EXPECT_TRUE(zones.set(&dock, ScreenEdge::Bottom, 70));
    EXPECT_EQ(base::Rect(0, 30, 1920, 980), zones.apply(kOutput, {}, true, true));
}

TEST(ExclusiveZonesTest, AReservationLargerThanTheOutputIsIgnored)
{
    ExclusiveZones zones;
    const int token = 1;
    zones.set(&token, ScreenEdge::Top, 2000);
    EXPECT_EQ(kOutput, zones.apply(kOutput));
}

TEST(ExclusiveZonesTest, ReservationsAreScopedToTheirLogicalOutput)
{
    ExclusiveZones zones;
    const int primaryBar = 1;
    const int externalDock = 2;
    zones.set(&primaryBar, ScreenEdge::Top, 30);
    zones.set(&externalDock, ScreenEdge::Bottom, 60, "DP-2");
    EXPECT_EQ(zones.apply(kOutput, "eDP-1", true), base::Rect(0, 30, 1920, 1050));
    EXPECT_EQ(zones.apply(kOutput, "DP-2", false), base::Rect(0, 0, 1920, 1020));
}

TEST(StackingTest, RolesMapToTheDocumentedLayerOrder)
{
    EXPECT_LT(layerFor(WindowRole::Desktop, PanelBehavior::StayAbove),
              layerFor(WindowRole::Normal, PanelBehavior::StayAbove));
    EXPECT_LT(layerFor(WindowRole::Normal, PanelBehavior::StayAbove),
              layerFor(WindowRole::Launcher, PanelBehavior::StayAbove));
    EXPECT_LT(layerFor(WindowRole::Panel, PanelBehavior::StayAbove),
              layerFor(WindowRole::Launcher, PanelBehavior::StayAbove));
    EXPECT_LT(layerFor(WindowRole::Launcher, PanelBehavior::StayAbove),
              layerFor(WindowRole::Dock, PanelBehavior::StayAbove));
    EXPECT_LT(layerFor(WindowRole::Dock, PanelBehavior::StayAbove),
              layerFor(WindowRole::Notification, PanelBehavior::StayAbove));
    EXPECT_LT(layerFor(WindowRole::Notification, PanelBehavior::StayAbove),
              layerFor(WindowRole::Osd, PanelBehavior::StayAbove));
    EXPECT_LT(layerFor(WindowRole::Osd, PanelBehavior::StayAbove),
              layerFor(WindowRole::Capture, PanelBehavior::StayAbove));
}

TEST(StackingTest, WindowsCanCoverDropsAPanelIntoTheNormalLayer)
{
    EXPECT_EQ(StackingLayer::Normal, layerFor(WindowRole::Panel, PanelBehavior::WindowsCanCover));
    EXPECT_EQ(StackingLayer::Normal, layerFor(WindowRole::Dock, PanelBehavior::WindowsCanCover));
    // A launcher raised over them then covers both.
    EXPECT_LT(layerFor(WindowRole::Dock, PanelBehavior::WindowsCanCover),
              layerFor(WindowRole::Launcher, PanelBehavior::StayAbove));
}

} // namespace
