// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gtest/gtest.h>

#include "backend/drm/drm_selection.h"

using namespace Lingmo::Backend;

namespace {

drmModeModeInfo makeMode(int width, int height, uint32_t type = 0, int clock = 0)
{
    drmModeModeInfo mode = {};
    mode.hdisplay = width;
    mode.vdisplay = height;
    mode.htotal = width + 100;
    mode.vtotal = height + 50;
    mode.type = type;
    // Default to roughly 60Hz for the chosen totals.
    mode.clock = clock ? clock : static_cast<int>(mode.htotal * mode.vtotal * 60.0 / 1000.0);
    return mode;
}

} // namespace

TEST(DrmSelection, SelectsFirstConnectedConnectorWithModes)
{
    const std::vector<ConnectorInfo> connectors = {
        {10, 0, 1, DRM_MODE_DISCONNECTED, 3},
        {11, 0, 2, DRM_MODE_CONNECTED, 0},
        {12, 0, 3, DRM_MODE_CONNECTED, 2},
        {13, 0, 4, DRM_MODE_CONNECTED, 5},
    };

    const auto index = selectConnector(connectors);
    ASSERT_TRUE(index.has_value());
    EXPECT_EQ(connectors.at(*index).id, 12u);
}

TEST(DrmSelection, AHeadMountedDisplayIsNoDesktopOutput)
{
    ConnectorInfo headset {10, 0, 1, DRM_MODE_CONNECTED, 3};
    headset.nonDesktop = true;
    const std::vector<ConnectorInfo> connectors = {headset, {11, 0, 2, DRM_MODE_CONNECTED, 2}};
    EXPECT_FALSE(isDesktopOutput(headset));
    const auto index = selectConnector(connectors);
    ASSERT_TRUE(index.has_value());
    EXPECT_EQ(connectors.at(*index).id, 11u);
}

TEST(DrmSelection, TheBootDisplayThenOzonesDriversComeFirst)
{
    EXPECT_EQ(orderDisplayDevices({{"/dev/dri/card0", "nouveau", false},
                                   {"/dev/dri/card1", "amdgpu", false},
                                   {"/dev/dri/card2", "i915", false},
                                   {"/dev/dri/card3", "rockchip", false}}),
              std::vector<std::string>({"/dev/dri/card2", "/dev/dri/card1", "/dev/dri/card0",
                           "/dev/dri/card3"}));
    // The card the firmware booted on wins over any driver preference.
    EXPECT_EQ(orderDisplayDevices({{"/dev/dri/card0", "i915", false},
                                   {"/dev/dri/card1", "nvidia-drm", true}}),
              std::vector<std::string>({"/dev/dri/card1", "/dev/dri/card0"}));
    EXPECT_TRUE(orderDisplayDevices({}).empty());
}

TEST(DrmSelection, NoConnectorWhenNothingIsConnected)
{
    const std::vector<ConnectorInfo> connectors = {
        {10, 0, 1, DRM_MODE_DISCONNECTED, 3},
        {11, 0, 2, DRM_MODE_UNKNOWNCONNECTION, 3},
    };
    EXPECT_FALSE(selectConnector(connectors).has_value());
    EXPECT_FALSE(selectConnector({}).has_value());
}

TEST(DrmSelection, PrefersThePreferredMode)
{
    const drmModeModeInfo modes[] = {
        makeMode(1920, 1080),
        makeMode(1280, 720, DRM_MODE_TYPE_PREFERRED),
        makeMode(3840, 2160),
    };

    const auto mode = selectMode(modes, 3);
    ASSERT_TRUE(mode.has_value());
    EXPECT_EQ(mode->hdisplay, 1280);
    EXPECT_EQ(mode->vdisplay, 720);
}

TEST(DrmSelection, ModesDifferingOnlyInSyncPositionsAreOneMode)
{
    drmModeModeInfo moved = makeMode(1920, 1080, DRM_MODE_TYPE_PREFERRED);
    moved.hsync_start = 2008;
    moved.hsync_end = 2052;
    drmModeModeInfo plain = moved;
    plain.type = 0;
    plain.hsync_start = 1968;
    plain.hsync_end = 2012;
    const drmModeModeInfo modes[] = {plain, makeMode(1280, 720), moved, plain};

    const auto distinct = distinctModes(modes, 4);
    ASSERT_EQ(distinct.size(), 2u);
    EXPECT_EQ(distinct[0].hsync_start, 2008);
    EXPECT_TRUE(distinct[0].type & DRM_MODE_TYPE_PREFERRED);
    EXPECT_EQ(distinct[1].hdisplay, 1280);

    drmModeModeInfo interlaced = plain;
    interlaced.flags = DRM_MODE_FLAG_INTERLACE;
    const drmModeModeInfo different[] = {plain, interlaced};
    EXPECT_EQ(distinctModes(different, 2).size(), 2u);
}

TEST(DrmSelection, FallsBackToTheLargestMode)
{
    const drmModeModeInfo modes[] = {
        makeMode(1280, 720),
        makeMode(1920, 1080),
        makeMode(1024, 768),
    };

    const auto mode = selectMode(modes, 3);
    ASSERT_TRUE(mode.has_value());
    EXPECT_EQ(mode->hdisplay, 1920);
}

TEST(DrmSelection, AnInterlacedModeIsOnlyALastResort)
{
    drmModeModeInfo interlaced = makeMode(1920, 1080);
    interlaced.flags |= DRM_MODE_FLAG_INTERLACE;
    const drmModeModeInfo modes[] = {interlaced, makeMode(1280, 720)};
    auto mode = selectMode(modes, 2);
    ASSERT_TRUE(mode.has_value());
    EXPECT_EQ(mode->hdisplay, 1280);
    mode = selectMode(&interlaced, 1);
    ASSERT_TRUE(mode.has_value());
    EXPECT_EQ(mode->hdisplay, 1920);
}

TEST(DrmSelection, BreaksAreaTiesOnRefreshRate)
{
    drmModeModeInfo sixty = makeMode(1920, 1080);
    drmModeModeInfo onetwenty = makeMode(1920, 1080);
    onetwenty.clock = sixty.clock * 2;

    const drmModeModeInfo modes[] = {sixty, onetwenty};
    const auto mode = selectMode(modes, 2);
    ASSERT_TRUE(mode.has_value());
    EXPECT_EQ(mode->clock, onetwenty.clock);
}

TEST(DrmSelection, NoModeWhenTheConnectorReportsNone)
{
    EXPECT_FALSE(selectMode(nullptr, 0).has_value());
    const drmModeModeInfo mode = makeMode(800, 600);
    EXPECT_FALSE(selectMode(&mode, 0).has_value());
}

TEST(DrmSelection, PicksACompatibleCrtc)
{
    const std::vector<uint32_t> crtcs = {40, 41, 42};
    // Only index 1 and 2 are compatible.
    EXPECT_EQ(selectCrtc(0b110, crtcs, 0, 0), 41u);
}

TEST(DrmSelection, KeepsTheCurrentlyAttachedCrtc)
{
    const std::vector<uint32_t> crtcs = {40, 41, 42};
    EXPECT_EQ(selectCrtc(0b111, crtcs, 0, 42), 42u);
}

TEST(DrmSelection, SkipsCrtcsAlreadyInUse)
{
    const std::vector<uint32_t> crtcs = {40, 41, 42};
    EXPECT_EQ(selectCrtc(0b111, crtcs, 0b011, 0), 42u);
    // The attached CRTC does not win when it is taken.
    EXPECT_EQ(selectCrtc(0b111, crtcs, 0b100, 42), 40u);
}

TEST(DrmSelection, NoCrtcWhenNoneIsCompatibleOrFree)
{
    const std::vector<uint32_t> crtcs = {40, 41};
    EXPECT_EQ(selectCrtc(0, crtcs, 0, 0), 0u);
    EXPECT_EQ(selectCrtc(0b11, crtcs, 0b11, 0), 0u);
}

TEST(DrmSelection, ComputesRefreshRate)
{
    drmModeModeInfo mode = {};
    mode.clock = 148500;
    mode.htotal = 2200;
    mode.vtotal = 1125;
    EXPECT_NEAR(refreshRate(mode), 60.0, 0.01);

    mode.flags = DRM_MODE_FLAG_INTERLACE;
    EXPECT_NEAR(refreshRate(mode), 120.0, 0.01);
}

TEST(DrmSelection, RefreshRateOfADegenerateModeIsZero)
{
    drmModeModeInfo mode = {};
    mode.clock = 148500;
    EXPECT_EQ(refreshRate(mode), 0.0);
}

TEST(DrmSelection, NamesConnectors)
{
    EXPECT_EQ(connectorName(DRM_MODE_CONNECTOR_VIRTUAL, 1), std::string("Virtual-1"));
    EXPECT_EQ(connectorName(DRM_MODE_CONNECTOR_HDMIA, 2), std::string("HDMI-A-2"));
}
