// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>

#include <algorithm>
#include <cmath>
#include <optional>
#include <string>
#include <vector>

#include "pages/battery/batteryhistory.h"
#include "pages/battery/timeformat.h"
#include "ui/label.h"
#include "ui/test_input.h"
#include "ui/window.h"
#include "window/options.h"
#include "window/page.h"
#include "window/settingswindow.h"

using namespace Lingmo::ui;
using Lingmo::upower::HistorySample;
using namespace settings;

namespace {

std::string dumpLine(const Window& window, const std::string& name)
{
    const std::string dump = window.layoutDump();
    const size_t at = dump.find("\"" + name + "\"");
    if (at == std::string::npos)
        return {};
    const size_t start = dump.rfind('\n', at) + 1;
    return dump.substr(start, dump.find('\n', at) - start);
}

// The view's logical rect in window coordinates, from its dump line.
RectF windowRect(const Window& window, const std::string& name)
{
    const std::string line = dumpLine(window, name);
    const size_t device = line.find(" device ");
    if (device == std::string::npos)
        return {-1, -1, -1, -1};
    long x, y, width, height;
    sscanf(line.c_str() + device, " device %ld %ld %ld %ld", &x, &y, &width, &height);
    const double scale = window.scale();
    return {x / scale, y / scale, width / scale, height / scale};
}

bool isHidden(const Window& window, const std::string& name)
{
    return dumpLine(window, name).ends_with(" hidden");
}

View* findView(View* view, const std::string& name)
{
    if (view->name() == name)
        return view;
    for (const auto& child : view->children()) {
        if (View* found = findView(child.get(), name))
            return found;
    }
    return nullptr;
}

struct BatteryPageTest : testing::Test
{
    void SetUp() override
    {
        setOptions({.mockBattery = true});
        settingsWindow = std::make_unique<SettingsWindow>();
        window().setScale(1.5);
        window().resize({900, 610});
        settingsWindow->reveal();
        settingsWindow->switchToPage("battery");
        // Through the curve's reveal.
        for (uint64_t ms = 0; ms <= 1000; ms += 100)
            TestInput::presentFrame(window(), ms * 1000000);
        window().grab();
    }

    void TearDown() override { setOptions({}); }

    Window& window() { return settingsWindow->window(); }

    std::string labelText(const std::string& name)
    {
        auto* label = dynamic_cast<Label*>(findView(settingsWindow->currentPage(), name));
        return label ? label->text() : std::string();
    }

    std::unique_ptr<SettingsWindow> settingsWindow;
};

} // namespace

TEST_F(BatteryPageTest, ShowsTheSampleBattery)
{
    EXPECT_FALSE(isHidden(window(), "battery:history"));
    EXPECT_FALSE(isHidden(window(), "battery:health"));
    EXPECT_FALSE(isHidden(window(), "battery:lastcharged"));
    EXPECT_FALSE(isHidden(window(), "battery:charging"));
}

TEST_F(BatteryPageTest, TheRangePickerEndsWithTheCard)
{
    const RectF range = windowRect(window(), "battery:range");
    const RectF card = windowRect(window(), "battery:graphcard");
    // Compact, to sit in the title row.
    EXPECT_EQ(range.height, 32);
    EXPECT_EQ(range.x + range.width, card.x + card.width);
    // Below the title row with the section's label spacing.
    EXPECT_EQ(card.y, range.y + range.height + 8);
    // 240 for the graph and the card's padding.
    EXPECT_EQ(card.height, 240 + 2 * 12);
    const RectF graph = windowRect(window(), "battery:graph");
    EXPECT_EQ(graph.x, card.x + 16);
    EXPECT_EQ(graph.width, card.width - 32);
    EXPECT_EQ(graph.height, 240);
}

TEST_F(BatteryPageTest, TheTitleCentresOnThePicker)
{
    View* page = settingsWindow->currentPage();
    const RectF range = findView(page, "battery:range")->geometry();
    const RectF title = findView(page, "battery:historytitle")->geometry();
    // Where Qt's RowLayout puts it, measured with the QML: y 8 next to the 32
    // high picker.
    EXPECT_EQ(title.y, 8);
    EXPECT_EQ(title.height, 17);
    EXPECT_EQ(range.y, 0);
}

TEST_F(BatteryPageTest, ThePlotLeavesRoomForTheAxes)
{
    const RectF graph = windowRect(window(), "battery:graph");
    const RectF plot = windowRect(window(), "graph:plot");
    const RectF legend = windowRect(window(), "graph:legend");
    EXPECT_EQ(plot.x, graph.x);
    EXPECT_GT(plot.y, legend.y + legend.height);
    // The value labels on the right, the time labels below.
    EXPECT_LT(plot.x + plot.width, graph.x + graph.width - 30);
    EXPECT_LT(plot.y + plot.height, graph.y + graph.height - 15);
}

TEST_F(BatteryPageTest, TheHoveringPointerReadsTheCurveOff)
{
    EXPECT_TRUE(isHidden(window(), "graph:readout"));
    const RectF plot = windowRect(window(), "graph:plot");
    TestInput::movePointer(window(), {plot.x + plot.width / 2, plot.y + plot.height / 2});
    window().grab();
    EXPECT_FALSE(isHidden(window(), "graph:readout"));
    // A whole percentage of the sample day, then the state and the time.
    const std::string value = labelText("graph:readout:value");
    ASSERT_FALSE(value.empty());
    EXPECT_EQ(value.back(), '%');
    const int percent = std::stoi(value);
    EXPECT_GE(percent, 26);
    EXPECT_LE(percent, 90);
    EXPECT_NE(labelText("graph:readout:time").find(" · "), std::string::npos);
    const RectF readout = windowRect(window(), "graph:readout");
    const RectF legend = windowRect(window(), "graph:legend");
    EXPECT_EQ(readout.y, legend.y);
    EXPECT_EQ(readout.height, legend.height);

    // Off the plot, the legend comes back.
    TestInput::movePointer(window(), {plot.x + plot.width / 2, plot.y + plot.height + 40});
    window().grab();
    EXPECT_TRUE(isHidden(window(), "graph:readout"));
    TestInput::movePointer(window(), {plot.x + plot.width / 2, plot.y + plot.height / 2});
    TestInput::leavePointer(window());
    window().grab();
    EXPECT_TRUE(isHidden(window(), "graph:readout"));
}

TEST_F(BatteryPageTest, SevenDaysPlotsAWeek)
{
    const RectF range = windowRect(window(), "battery:range");
    TestInput::click(window(), {range.x + range.width * 3 / 4, range.y + range.height / 2});
    for (uint64_t ms = 1000; ms <= 2000; ms += 100)
        TestInput::presentFrame(window(), ms * 1000000);
    window().grab();
    const RectF plot = windowRect(window(), "graph:plot");
    TestInput::movePointer(window(), {plot.x + 1, plot.y + plot.height / 2});
    window().grab();
    // A week's reading names the day.
    const std::string time = labelText("graph:readout:time");
    bool named = false;
    for (int day = 0; day < 7; ++day)
        named = named || time.find(" · " + timeformat::shortDayName(1792000000 + day * 86400) + " ") != std::string::npos;
    EXPECT_TRUE(named) << time;
}

TEST(BatteryRevealTest, TheCurveDrawsInFromTheLeft)
{
    setOptions({.mockBattery = true});
    auto settingsWindow = std::make_unique<SettingsWindow>();
    Window& window = settingsWindow->window();
    window.setScale(1.5);
    window.resize({900, 610});
    settingsWindow->reveal();
    settingsWindow->switchToPage("battery");
    // Lays the page out and starts the reveal, then its first tick.
    TestInput::presentFrame(window, 1000000);
    TestInput::presentFrame(window, 2000000);
    const RectF plot = windowRect(window, "graph:plot");
    // The curve's pixels in a column: the line in either colour, against the
    // grey and white of the card and the grid.
    const auto curveAt = [&](const Image& image, double x) {
        int count = 0;
        for (double y = plot.y; y < plot.y + plot.height; y += 1) {
            const Color pixel = image.pixel(int(x * 1.5), int(y * 1.5));
            if (std::max({pixel.r, pixel.g, pixel.b}) - std::min({pixel.r, pixel.g, pixel.b}) > 60)
                ++count;
        }
        return count;
    };
    // Half way through 700 ms of an out-cubic: seven eighths drawn.
    const Image early = TestInput::presentFrame(window, 352000000);
    EXPECT_GT(curveAt(early, plot.x + plot.width * 0.5), 0);
    EXPECT_EQ(curveAt(early, plot.x + plot.width * 0.97), 0);
    const Image done = TestInput::presentFrame(window, 800000000);
    EXPECT_GT(curveAt(done, plot.x + plot.width * 0.97), 0);
    settingsWindow.reset();
    setOptions({});
}

TEST(BatteryHistoryTest, StartsWithTheValueInForce)
{
    const uint32_t now = 100000;
    // Newest first, as UPower returns them.
    const std::vector<HistorySample> window{{99000, 50, 2}, {95000, 60, 2}};
    const std::vector<HistorySample> all{{99000, 50, 2}, {95000, 60, 2}, {90000, 70, 1}, {80000, 75, 1}};
    const auto values = BatteryHistory::plotted(window, all, now, 10000);
    ASSERT_EQ(values.size(), 4u);
    // The sample before the window, moved to its start.
    EXPECT_EQ(values[0].time, 90000u);
    EXPECT_EQ(values[0].value, 70);
    EXPECT_EQ(values[1].time, 95000u);
    EXPECT_EQ(values[2].time, 99000u);
    // The last value runs on to the present.
    EXPECT_EQ(values[3].time, now);
    EXPECT_EQ(values[3].value, 50);
}

TEST(BatteryHistoryTest, SkipsRestartsWrongClocksAndDuplicates)
{
    const uint32_t now = 100000;
    // Newest first, as UPower returns them.
    const std::vector<HistorySample> window{
        {99500, 40, 2},
        // From the future.
        {200000, 45, 2},
        // Stamped before the clock was set: older than the one recorded before it.
        {96500, 58, 2},
        // Two in one second: the later one wins.
        {97000, 56, 2},
        {97000, 55, 2},
        // A restart marker.
        {96000, 0, 0},
        {95000, 60, 2},
    };
    const auto values = BatteryHistory::plotted(window, std::nullopt, now, 10000);
    ASSERT_EQ(values.size(), 4u);
    EXPECT_EQ(values[0].time, 95000u);
    EXPECT_EQ(values[0].value, 60);
    EXPECT_EQ(values[1].time, 97000u);
    EXPECT_EQ(values[1].value, 56);
    EXPECT_EQ(values[2].time, 99500u);
    EXPECT_EQ(values[3].time, now);
    EXPECT_EQ(values[3].value, 40);
}

TEST(BatteryHistoryTest, AnEmptyWindowPlotsNothing)
{
    EXPECT_TRUE(BatteryHistory::plotted({}, std::nullopt, 100000, 86400).empty());
}

TEST(TimeFormatTest, CountsLocalDays)
{
    const int64_t noon = 1792000000;
    EXPECT_EQ(timeformat::daysBetween(noon, noon), 0);
    EXPECT_EQ(timeformat::daysBetween(noon - 86400, noon), 1);
    EXPECT_EQ(timeformat::daysBetween(noon - 3 * 86400, noon), 3);
}
