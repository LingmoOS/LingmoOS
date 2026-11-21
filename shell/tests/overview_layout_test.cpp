// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// The overview's spatial spread: what holds for any windows, that windows
// keep roughly where they were, and that small ones grow into the room left.

#include <algorithm>
#include <array>
#include <cmath>
#include <random>

#include <gtest/gtest.h>

#include "base/geometry.h"
#include "wm/overview/overview_layout.h"

using namespace Lingmo;
using namespace Lingmo::WM::Overview;

namespace {

const base::Rect kArea(40, 120, 1840, 900);
constexpr int kGap = 32;

// The space between two rectangles along whichever axis parts them; negative
// when they overlap.
double gapBetween(const base::RectF &a, const base::RectF &b)
{
    const double horizontal = std::max(b.left() - a.right(), a.left() - b.right());
    const double vertical = std::max(b.top() - a.bottom(), a.top() - b.bottom());
    return std::max(horizontal, vertical);
}

void expectSpread(const std::vector<base::Rect> &windows, const std::vector<base::RectF> &result,
                  const base::Rect &area, double deviceScale)
{
    ASSERT_EQ(windows.size(), result.size());
    const double pixel = 1 / deviceScale;
    for (size_t i = 0; i < result.size(); ++i) {
        const base::RectF &rect = result[i];
        SCOPED_TRACE(i);
        // Inside the area.
        EXPECT_GE(rect.left(), area.left() - pixel);
        EXPECT_GE(rect.top(), area.top() - pixel);
        EXPECT_LE(rect.right(), area.x() + area.width() + pixel);
        EXPECT_LE(rect.bottom(), area.y() + area.height() + pixel);
        // Never larger than the window, and of its shape but for the rounding
        // of its edges.
        EXPECT_LE(rect.width(), windows[i].width() + pixel);
        EXPECT_LE(rect.height(), windows[i].height() + pixel);
        const double aspect = double(windows[i].height()) / windows[i].width();
        EXPECT_NEAR(rect.width() * aspect, rect.height(), pixel * (1 + aspect) + 1e-6);
        // On the device pixel grid.
        for (const double edge : {rect.left(), rect.top(), rect.right(), rect.bottom()})
            EXPECT_NEAR(edge * deviceScale, std::round(edge * deviceScale), 1e-6);
        for (size_t j = i + 1; j < result.size(); ++j)
            EXPECT_GE(gapBetween(rect, result[j]), kGap - 1e-6) << "with " << j;
    }
}

} // namespace

TEST(OverviewLayout, NoWindowsNoLayout)
{
    EXPECT_TRUE(spreadLayout({}, kArea, kGap, 1).empty());
}

TEST(OverviewLayout, WindowsThatFitKeepTheirSizeAndOrder)
{
    const std::vector<base::Rect> windows = {base::Rect(700, 250, 500, 400), base::Rect(100, 200, 400, 300),
                                        base::Rect(300, 700, 300, 200)};
    const auto result = spreadLayout(windows, kArea, kGap, 1);
    expectSpread(windows, result, kArea, 1);
    for (size_t i = 0; i < windows.size(); ++i)
        EXPECT_EQ(base::SizeF(windows[i].size()), result[i].size());
    // Left of each other as they were.
    EXPECT_LT(result[1].right(), result[0].left());
}

TEST(OverviewLayout, WindowsOnOneVerticalLineStillUseTheWidth)
{
    // Centred windows of every size, as a desk of new windows has them: laid
    // out side by side, not in a column down the middle.
    const std::vector<base::Rect> windows = {base::Rect(560, 180, 800, 500), base::Rect(610, 240, 700, 560),
                                        base::Rect(0, 40, 1920, 1000), base::Rect(460, 190, 1000, 700)};
    const auto result = spreadLayout(windows, kArea, kGap, 1);
    expectSpread(windows, result, kArea, 1);
    base::RectF extent;
    for (const base::RectF &rect : result)
        extent |= rect;
    EXPECT_GT(extent.width(), kArea.width() * 0.8);
    int beside = 0;
    for (size_t i = 0; i < result.size(); ++i) {
        for (size_t j = i + 1; j < result.size(); ++j) {
            if (result[i].top() < result[j].bottom() && result[j].top() < result[i].bottom())
                ++beside;
        }
    }
    EXPECT_GE(beside, 1);
}

TEST(OverviewLayout, HigherWindowsTakeHigherRows)
{
    const std::vector<base::Rect> windows = {base::Rect(0, 600, 1600, 900), base::Rect(200, 0, 1600, 900),
                                        base::Rect(100, 300, 1600, 900)};
    const auto result = spreadLayout(windows, kArea, kGap, 1);
    expectSpread(windows, result, kArea, 1);
    // The one highest up is never below the lowest.
    EXPECT_LE(result[1].top(), result[0].top());
}

TEST(OverviewLayout, MaximizedWindowsOnOneCentreFanOut)
{
    const std::vector<base::Rect> windows(4, base::Rect(0, 40, 1920, 1000));
    const auto result = spreadLayout(windows, kArea, kGap, 1);
    expectSpread(windows, result, kArea, 1);
    for (const base::RectF &rect : result)
        EXPECT_GT(rect.width(), 200);
}

TEST(OverviewLayout, ASmallWindowGrowsIntoTheRoomLargeOnesLeave)
{
    // Four maximized windows need a small scale to fit; the small one, off to
    // a side, does not.
    std::vector<base::Rect> windows(4, base::Rect(0, 40, 1920, 1000));
    windows.emplace_back(1400, 700, 480, 320);
    const auto result = spreadLayout(windows, kArea, kGap, 1);
    expectSpread(windows, result, kArea, 1);
    double large = 0;
    for (size_t i = 0; i < 4; ++i)
        large = std::max(large, result[i].width() / windows[i].width());
    EXPECT_GT(result[4].width() / windows[4].width(), large + 0.1);
    // And the whole is centred in the area.
    base::RectF extent;
    for (const base::RectF &rect : result)
        extent |= rect;
    EXPECT_NEAR(base::RectF(kArea).center().x(), extent.center().x(), 1);
    EXPECT_NEAR(base::RectF(kArea).center().y(), extent.center().y(), 1);
}

TEST(OverviewLayout, TheSameWindowsGiveTheSameLayout)
{
    const std::vector<base::Rect> windows = {base::Rect(0, 0, 900, 700), base::Rect(100, 60, 900, 700),
                                        base::Rect(200, 120, 900, 700)};
    EXPECT_EQ(spreadLayout(windows, kArea, kGap, 1.5), spreadLayout(windows, kArea, kGap, 1.5));
}

TEST(OverviewLayout, AnyWindowsAtAnyScaleSpreadWithinTheArea)
{
    std::mt19937 random(7);
    for (int round = 0; round < 300; ++round) {
        SCOPED_TRACE(round);
        std::uniform_int_distribution<int> count(1, 14);
        std::uniform_int_distribution<int> x(-200, 1800);
        std::uniform_int_distribution<int> y(-100, 1000);
        std::uniform_int_distribution<int> width(120, 1920);
        std::uniform_int_distribution<int> height(90, 1080);
        std::vector<base::Rect> windows;
        for (int i = count(random); i > 0; --i)
            windows.emplace_back(x(random), y(random), width(random), height(random));
        const double scale = std::array {1.0, 1.25, 1.5, 2.0}[round % 4];
        expectSpread(windows, spreadLayout(windows, kArea, kGap, scale), kArea, scale);
    }
}
