// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gtest/gtest.h>

#include "display/recommended_scale.h"

using namespace Lingmo::Display;
namespace base = Lingmo::base;

namespace {

DisplayMode mode(base::Size size, bool preferred = false)
{
    DisplayMode result;
    result.pixelSize = size;
    result.refreshMilliHz = 60000;
    result.preferred = preferred;
    return result;
}

DisplaySnapshot display(DisplayConnectionType type, base::Size pixels, base::Size mm)
{
    DisplaySnapshot result;
    result.connectionType = type;
    result.physicalSizeMm = mm;
    result.modes = {mode(pixels, true)};
    result.nativeMode = result.modes.front();
    result.currentMode = result.nativeMode;
    return result;
}

double scale(DisplayConnectionType type, base::Size pixels, base::Size mm)
{
    return recommendedScale(display(type, pixels, mm)).value();
}

} // namespace

TEST(RecommendedScale, InternalPanelsFollowChromiumDensities)
{
    constexpr auto internal = DisplayConnectionType::Internal;
    // 15.6" 1080p, 141 dpi.
    EXPECT_EQ(scale(internal, {1920, 1080}, {344, 194}), 1.0);
    // 13.3" 1080p, 166 dpi.
    EXPECT_EQ(scale(internal, {1920, 1080}, {294, 165}), 1.25);
    // 13.5" 2256x1504, 201 dpi: Chromium's 1.6.
    EXPECT_EQ(scale(internal, {2256, 1504}, {285, 190}), 1.5);
    // 13.3" 2560x1600, 227 dpi: Chromium's 1.777.
    EXPECT_EQ(scale(internal, {2560, 1600}, {286, 179}), 1.75);
    // 14" 2880x1800, 242 dpi.
    EXPECT_EQ(scale(internal, {2880, 1800}, {302, 189}), 2.0);
    // 13.4" 3840x2400, 336 dpi: Chromium's 2.666.
    EXPECT_EQ(scale(internal, {3840, 2400}, {290, 181}), 2.5);
}

TEST(RecommendedScale, ExternalMonitorsAllowForViewingDistance)
{
    constexpr auto external = DisplayConnectionType::External;
    // 27" 1440p, 109 dpi.
    EXPECT_EQ(scale(external, {2560, 1440}, {597, 336}), 1.0);
    // 32" 4K, 137 dpi.
    EXPECT_EQ(scale(external, {3840, 2160}, {708, 398}), 1.25);
    // 27" 4K, 163 dpi.
    EXPECT_EQ(scale(external, {3840, 2160}, {597, 336}), 1.5);
    // 24" 4K, 184 dpi.
    EXPECT_EQ(scale(external, {3840, 2160}, {527, 296}), 1.75);
    // 27" 5K, 218 dpi.
    EXPECT_EQ(scale(external, {5120, 2880}, {597, 336}), 2.0);
}

TEST(RecommendedScale, ReadsTheNativeModeNotTheCurrentOne)
{
    auto monitor = display(DisplayConnectionType::External, {3840, 2160}, {597, 336});
    monitor.modes.push_back(mode({1920, 1080}));
    monitor.currentMode = monitor.modes.back();
    EXPECT_EQ(recommendedScale(monitor).value(), 1.5);
}

TEST(RecommendedScale, UnknownSizesAndVirtualOutputsStayAtOne)
{
    // Aspect ratios reported in place of a size, as projectors do.
    EXPECT_EQ(scale(DisplayConnectionType::External, {3840, 2160}, {160, 90}), 1.0);
    EXPECT_EQ(scale(DisplayConnectionType::External, {3840, 2160}, {0, 0}), 1.0);
    EXPECT_EQ(scale(DisplayConnectionType::Virtual, {3840, 2160}, {300, 170}), 1.0);
}
