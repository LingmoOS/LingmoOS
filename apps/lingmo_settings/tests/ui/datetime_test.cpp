// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>

#include <algorithm>
#include <chrono>
#include <cmath>

#include "lingmo/timedate/time_date.h"
#include "pages/datetime/timezonedialog.h"
#include "pages/datetime/timezonemap.h"
#include "ui/keys.h"
#include "ui/test_input.h"
#include "ui/text_field.h"
#include "ui/window.h"
#include "window/settingswindow.h"

using namespace Lingmo;
using namespace settings;

namespace {

// The map without timedated, so nothing here reaches the system.
struct TimeZoneMapFixture : ::testing::Test
{
    timedate::TimeDate timeDate{nullptr};
    TimeZoneMap map{timeDate};
};

} // namespace

TEST_F(TimeZoneMapFixture, FindsAZoneAtItsOwnSpot)
{
    const std::string zone = "Asia/Singapore";
    const ui::PointF point = map.position(zone, 3600, 1800);
    ASSERT_GE(point.x, 0);
    ASSERT_GE(point.y, 0);
    EXPECT_EQ(map.nearest(int(std::lround(point.x)), int(std::lround(point.y)), 3600, 1800), zone);
    EXPECT_EQ(map.position("Nowhere/Land", 3600, 1800).x, -1);
}

TEST_F(TimeZoneMapFixture, AClickPicksTheZonesAroundIt)
{
    // Berlin, Copenhagen, Prague and their neighbours lie within a few pixels
    // of each other on a small map: all of them are offered, sorted.
    const ui::PointF berlin = map.position("Europe/Berlin", 400, 200);
    map.clicked(int(berlin.x), int(berlin.y), 400, 200);
    const auto& zones = map.availableList();
    ASSERT_GT(zones.size(), 1u);
    EXPECT_TRUE(std::is_sorted(zones.begin(), zones.end()));
    EXPECT_NE(std::find(zones.begin(), zones.end(), "Europe/Berlin"), zones.end());

    // Far from every zone, the nearest one alone.
    map.clicked(0, 199, 400, 200);
    EXPECT_EQ(map.availableList().size(), 1u);

    map.select("America/New_York");
    EXPECT_EQ(map.availableList(), std::vector<std::string>{"America/New_York"});
}

TEST_F(TimeZoneMapFixture, NamesAndSearch)
{
    EXPECT_EQ(TimeZoneMap::localeTimeZoneName("America/Los_Angeles"), "Los Angeles");
    EXPECT_EQ(TimeZoneMap::localeTimeZoneName("UTC"), "UTC");
    EXPECT_EQ(TimeZoneMap::territoryName("US"), "United States");
    EXPECT_EQ(TimeZoneMap::territoryName("SG"), "Singapore");

    // City first, then the id, then a match inside either, then the country.
    const auto newYork = map.search("new york");
    ASSERT_FALSE(newYork.empty());
    EXPECT_EQ(newYork.front().id, "America/New_York");
    EXPECT_EQ(newYork.front().region, "United States");
    const auto byCode = map.search("  sg ");
    ASSERT_FALSE(byCode.empty());
    EXPECT_EQ(byCode.front().id, "Asia/Singapore");
    EXPECT_EQ(map.search("america").size(), 12u);
    EXPECT_TRUE(map.search("   ").empty());
    EXPECT_TRUE(map.search("no such place").empty());
}

TEST_F(TimeZoneMapFixture, InfoSaysTheDifferenceInWords)
{
    const auto info = map.info("Asia/Kolkata");
    ASSERT_EQ(info.id, "Asia/Kolkata");
    EXPECT_EQ(info.name, "Kolkata");
    EXPECT_EQ(info.region, "India");
    EXPECT_EQ(info.offsetSeconds, 5 * 3600 + 1800);
    EXPECT_FALSE(info.time.empty());
    if (map.currentTimeZoneId() == "Asia/Singapore")
        EXPECT_EQ(info.difference, "2 h 30 min behind");
    EXPECT_EQ(map.info(map.currentTimeZoneId()).difference, "Same time as now");
    EXPECT_TRUE(map.info("Nowhere/Land").id.empty());
}

TEST_F(TimeZoneMapFixture, EveryZoneIsAPoint)
{
    const auto points = map.points();
    ASSERT_GT(points.size(), 300u);
    for (const auto& point : points) {
        EXPECT_GE(point.x, -0.1);
        EXPECT_LE(point.x, 1.1);
    }
}

namespace {

// The view of the type with the window coordinates of its rect.
template<typename T>
T* find(ui::View* view)
{
    if (auto* found = dynamic_cast<T*>(view))
        return found;
    for (const auto& child : view->children()) {
        if (T* found = find<T>(child.get()))
            return found;
    }
    return nullptr;
}

ui::RectF windowRect(ui::View* view)
{
    const ui::PointF origin = view->mapToWindow({0, 0});
    return {origin.x, origin.y, view->geometry().width, view->geometry().height};
}

} // namespace

TEST_F(TimeZoneMapFixture, DialogLayout)
{
    SettingsWindow window;
    TimeZoneDialog dialog(window, map);
    dialog.window()->setScale(1.5);
    dialog.show();
    ui::Window& dialogWindow = *dialog.window();
    dialogWindow.grab();
    EXPECT_EQ(dialogWindow.size().width, 900);
    EXPECT_EQ(dialogWindow.size().height, 640);

    // The search box in the middle of the window, on the title bar's band.
    ui::TextField* field = find<ui::TextField>(dialogWindow.contentView());
    ASSERT_TRUE(field);
    const ui::RectF search = windowRect(field->parent());
    EXPECT_EQ(search.x, 310);
    EXPECT_EQ(search.y, 9);
    EXPECT_EQ(search.width, 280);
    EXPECT_EQ(search.height, 30);

    // The zone in use is selected and its card shown.
    EXPECT_EQ(dialog.selectedZone(), map.currentTimeZoneId());
    EXPECT_FALSE(dialog.isChanged());

    // Typing lists matches; Down moves through them, Return picks.
    ui::TestInput::click(dialogWindow, {search.x + 140, search.y + 15});
    ui::TestInput::typeText(dialogWindow, "berl");
    ui::TestInput::typeKey(dialogWindow, ui::key::Return);
    dialogWindow.grab();
    EXPECT_EQ(dialog.selectedZone(), "Europe/Berlin");
    EXPECT_TRUE(dialog.isChanged());
    EXPECT_EQ(field->text(), "");
}
