// Copyright 2014 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.chromium file.
//
// Upstream: chromium/src/ui/display/manager/test/fake_display_snapshot_unittest.cc
// Revision: 7acd40ddefff49c9d36144999aafdbb674dbd07a
//
// Adapted for Lingmo: tests stable string identities and exact timing keys.

#include <gtest/gtest.h>

#include "display/display_types.h"

using namespace Lingmo::Display;
namespace base = Lingmo::base;

namespace {

DisplayMode mode(base::Size size = {1920, 1080}, int refresh = 60000)
{
    return {size, refresh, 148500, 2200, 1125, 0, "preferred", true};
}

DisplaySnapshot snapshot(std::string id, std::string port)
{
    DisplaySnapshot value;
    value.sessionId = DisplayId(std::move(id));
    value.persistentKey = "edid:vendor:model:serial";
    value.portKey = std::move(port);
    value.connectorName = "DP-1";
    value.devicePath = "/dev/dri/card0";
    value.physicalSizeMm = base::Size(600, 340);
    value.modes = {mode()};
    value.currentMode = value.modes.front();
    value.nativeMode = value.modes.front();
    return value;
}

} // namespace

TEST(DisplayTypes, ModeIdentityRetainsTiming)
{
    DisplayMode first = mode();
    DisplayMode second = first;
    second.pixelClockKHz += 1;
    EXPECT_TRUE(first.isValid());
    EXPECT_NE(first.stableKey(), second.stableKey());

    second.horizontalTotal = 100;
    EXPECT_FALSE(second.isValid());
}

TEST(DisplayTypes, SnapshotRequiresUniqueSessionAndPortIdentity)
{
    HardwareSnapshot hardware {
        "instance",
        "seat0",
        4,
        {snapshot("session-a", "card0-DP-1"),
         snapshot("session-b", "card0-DP-2")}};
    EXPECT_TRUE(hardware.isValid());

    hardware.displays[1].sessionId = hardware.displays[0].sessionId;
    EXPECT_FALSE(hardware.isValid());
    hardware.displays[1].sessionId = DisplayId("session-b");
    hardware.displays[1].portKey = hardware.displays[0].portKey;
    EXPECT_FALSE(hardware.isValid());
}

TEST(DisplayTypes, CurrentModeMustComeFromModeList)
{
    DisplaySnapshot display = snapshot("session-a", "card0-DP-1");
    display.currentMode->refreshMilliHz = 59940;
    EXPECT_FALSE(display.isValid());
}
