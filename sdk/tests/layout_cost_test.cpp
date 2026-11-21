// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>

#include <algorithm>
#include <cstdio>
#include <ctime>
#include <functional>
#include <string>
#include <vector>

#include "ui/cell_layout.h"
#include "ui/label.h"
#include "ui/layout.h"
#include "ui/list_view.h"
#include "ui/painter.h"
#include "ui/scroll_view.h"
#include "ui/switch.h"
#include "ui/test_input.h"
#include "ui/theme.h"
#include "ui/window.h"

using namespace Lingmo::UI;

// This thread's CPU time, so the tests running beside this one do not count.
static double threadMilliseconds()
{
    timespec time;
    clock_gettime(CLOCK_THREAD_CPUTIME_ID, &time);
    return double(time.tv_sec) * 1000 + double(time.tv_nsec) / 1e6;
}

// The median of the frames' CPU milliseconds, each a step and a presented frame.
static double frameCost(Window& window, int frames, const std::function<void(int)>& step)
{
    TestInput::presentFrame(window);
    std::vector<double> costs;
    for (int i = 0; i < frames; ++i) {
        const double start = threadMilliseconds();
        step(i);
        TestInput::presentFrame(window);
        costs.push_back(threadMilliseconds() - start);
    }
    std::sort(costs.begin(), costs.end());
    return costs[costs.size() / 2];
}

namespace {

class Card : public ColumnLayout
{
protected:
    void paint(Painter& painter) override
    {
        const Theme& theme = Theme::current();
        painter.drawRoundedRect(localRect(), {theme.bigRadius, 0, Corners::Continuous}, theme.secondBackground,
                                1 / painter.scale(), theme.text.withAlpha(0.08));
    }
};

// A row that highlights under the pointer, as list rows do.
class HoverRow : public RowLayout
{
public:
    HoverRow() { setTracksHover(true); }

protected:
    void hoverChanged() override { requestPaint(); }
    void paint(Painter& painter) override
    {
        if (isHovered())
            painter.fillRect(localRect(), Theme::current().text.withAlpha(0.06));
    }
};

// A settings window: a sidebar list beside a scrolled page of sections, each
// a title over a card of rows with a label, a value and a switch.
struct SettingsWindow
{
    Window window;
    ScrollView* page = nullptr;
    ListView* sidebar = nullptr;
    std::vector<Label*> values;

    SettingsWindow()
    {
        window.setScale(1.5);
        auto root = std::make_unique<RowLayout>();
        root->setSpacing(0);

        sidebar = root->addChild<ListView>();
        sidebar->setLayoutHints({.fillHeight = true, .preferredWidth = 220});
        sidebar->setRowHeight(36);
        sidebar->createItem = [] {
            auto row = std::make_unique<HoverRow>();
            row->setPadding({12, 0, 12, 0});
            row->addChild<Label>()->setLayoutHints({.fillWidth = true});
            return row;
        };
        sidebar->bindItem = [](View* item, size_t row) {
            static_cast<Label*>(item->children()[0].get())->setText("Page " + std::to_string(row));
        };
        sidebar->setRowCount(30);

        page = root->addChild<ScrollView>();
        page->setLayoutHints({.fillWidth = true, .fillHeight = true});
        auto* sections = page->setContent<ColumnLayout>();
        sections->setLayoutHints({.fillWidth = true});
        sections->setPadding({24, 24, 24, 24});
        sections->setSpacing(18);
        for (int s = 0; s < 8; ++s) {
            auto* section = sections->addChild<ColumnLayout>();
            section->setLayoutHints({.fillWidth = true});
            section->setSpacing(6);
            section->addChild<Label>("Section " + std::to_string(s));
            auto* card = section->addChild<Card>();
            card->setLayoutHints({.fillWidth = true});
            card->setSpacing(0);
            for (int r = 0; r < 5; ++r) {
                auto* row = card->addChild<HoverRow>();
                row->setLayoutHints({.fillWidth = true, .minimumHeight = 44});
                row->setPadding({16, 0, 16, 0});
                row->setSpacing(10);
                auto* text = row->addChild<ColumnLayout>();
                text->setLayoutHints({.fillWidth = true});
                text->setSpacing(2);
                text->addChild<Label>("Setting " + std::to_string(s * 5 + r));
                if (r % 2 == 0) {
                    auto* detail = text->addChild<Label>("A longer description of what the setting does, "
                                                         "wrapping when the window is narrow");
                    detail->setWrap(true);
                    detail->setLayoutHints({.fillWidth = true});
                }
                values.push_back(row->addChild<Label>("Value"));
                row->addChild<Switch>();
            }
        }
        window.setContentView(std::move(root));
        window.resize({960, 700});
        window.show();
        TestInput::configure(window, 960, 700, {});
    }
};

} // namespace

// Interactive resize: every frame lays the whole window out at a new size.
TEST(LayoutCostTest, SettingsResize)
{
    SettingsWindow settings;
    Window& window = settings.window;
    const double resize = frameCost(window, 90, [&window](int i) {
        TestInput::resizeInteractively(window, {double(960 - (i % 30) * 4), double(700 - (i % 30) * 2)});
    });
    TestInput::resizeInteractively(window, {960, 700});
    const double still = frameCost(window, 90, [&window](int) { window.contentView()->requestPaint(); });
    printf("settings at 1.5: resize %.3f ms, full repaint %.3f ms\n", resize, still);
}

// One label's text changes, as a value updating: one row repaints.
TEST(LayoutCostTest, SettingsValueChange)
{
    SettingsWindow settings;
    const double value = frameCost(settings.window, 200, [&settings](int i) {
        settings.values[size_t(i) % settings.values.size()]->setText("Value " + std::to_string(i));
    });
    printf("settings at 1.5: value change %.3f ms\n", value);
}

TEST(LayoutCostTest, SettingsScrollAndHover)
{
    SettingsWindow settings;
    Window& window = settings.window;
    const double scroll = frameCost(window, 90, [&settings](int i) {
        settings.page->setScrollPosition({0, double((i % 30) * 10)});
    });
    TestInput::movePointer(window, {600, 300});
    const double hover = frameCost(window, 90, [&window](int i) {
        TestInput::movePointer(window, {600 + double(i % 10), 100 + double((i % 30) * 15)});
    });
    printf("settings at 1.5: scroll %.3f ms, hover move %.3f ms\n", scroll, hover);
}

// A list of thousands of rows, as a folder in list view: only the rows in
// sight have views.
TEST(LayoutCostTest, LongList)
{
    Window window;
    window.setScale(1.5);
    auto list = std::make_unique<ListView>();
    ListView* view = list.get();
    view->setRowHeight(28);
    view->createItem = [] {
        auto row = std::make_unique<HoverRow>();
        row->setPadding({12, 0, 12, 0});
        row->setSpacing(8);
        row->addChild<Label>()->setLayoutHints({.fillWidth = true});
        row->addChild<Label>()->setLayoutHints({.preferredWidth = 140});
        row->addChild<Label>()->setLayoutHints({.preferredWidth = 80});
        row->addChild<Label>()->setLayoutHints({.preferredWidth = 120});
        return row;
    };
    view->bindItem = [](View* item, size_t row) {
        const auto& children = item->children();
        static_cast<Label*>(children[0].get())->setText("File " + std::to_string(row) + ".txt");
        static_cast<Label*>(children[1].get())->setText("17 Oct 2026 08:" + std::to_string(row % 60));
        static_cast<Label*>(children[2].get())->setText(std::to_string(row * 7 % 1000) + " KB");
        static_cast<Label*>(children[3].get())->setText("Plain text");
    };
    view->setRowCount(10000);
    window.setContentView(std::move(list));
    window.resize({900, 700});
    window.show();
    TestInput::configure(window, 900, 700, {});

    const int frames = 120;
    const double scroll = frameCost(window, frames, [view](int i) {
        view->setScrollPosition({0, double(i) * 9.5});
    });
    TestInput::movePointer(window, {300, 300});
    const double scrollHovered = frameCost(window, frames, [view](int i) {
        view->setScrollPosition({0, 2000 + double(i) * 9.5});
    });
    const double resize = frameCost(window, frames / 2, [&window](int i) {
        TestInput::resizeInteractively(window, {double(900 - (i % 30) * 4), double(700 - (i % 30) * 2)});
    });
    printf("list of 10000 at 1.5: scroll %.3f ms, scroll under the pointer %.3f ms, resize %.3f ms\n",
           scroll, scrollHovered, resize);
}

// The layout engine alone: a page of nested cell layouts over fixed-size
// leaves, measured again at a new width, as a resize does every frame.
TEST(LayoutCostTest, CellLayoutSolve)
{
    auto page = std::make_unique<ColumnLayout>();
    page->setPadding({24, 24, 24, 24});
    page->setSpacing(18);
    std::vector<View*> leaves;
    for (int s = 0; s < 8; ++s) {
        auto* section = page->addChild<ColumnLayout>();
        section->setLayoutHints({.fillWidth = true});
        leaves.push_back(section->addChild<View>());
        leaves.back()->setLayoutHints({.preferredWidth = 80, .preferredHeight = 16});
        auto* card = section->addChild<ColumnLayout>();
        card->setLayoutHints({.fillWidth = true});
        for (int r = 0; r < 5; ++r) {
            auto* row = card->addChild<RowLayout>();
            row->setLayoutHints({.fillWidth = true, .minimumHeight = 44});
            row->setPadding({16, 0, 16, 0});
            auto* text = row->addChild<ColumnLayout>();
            text->setLayoutHints({.fillWidth = true});
            for (int l = 0; l < 2; ++l) {
                leaves.push_back(text->addChild<View>());
                leaves.back()->setLayoutHints({.fillWidth = true, .preferredWidth = 120, .preferredHeight = 16});
            }
            leaves.push_back(row->addChild<View>());
            leaves.back()->setLayoutHints({.preferredWidth = 40, .preferredHeight = 16});
            leaves.push_back(row->addChild<View>());
            leaves.back()->setLayoutHints({.preferredWidth = 38, .preferredHeight = 22});
        }
    }

    const int iterations = 4000;
    std::vector<double> costs;
    double height = 0;
    for (int i = 0; i < iterations; ++i) {
        leaves[size_t(i) % leaves.size()]->requestLayout();
        const double start = threadMilliseconds();
        height += page->heightForWidth(600 + i % 7);
        costs.push_back(threadMilliseconds() - start);
    }
    std::sort(costs.begin(), costs.end());
    printf("cell layout solve of %zu leaves: %.1f us\n", leaves.size(), costs[costs.size() / 2] * 1000);
    EXPECT_GT(height, 0);
}

// The same for rows and columns: cards of rows, each with a column of text.
TEST(LayoutCostTest, BoxLayoutMeasure)
{
    auto page = std::make_unique<Column>();
    page->setMargins({24, 24, 24, 24});
    page->setSpacing(12);
    std::vector<View*> leaves;
    for (int c = 0; c < 6; ++c) {
        auto* card = page->addChild<Column>();
        card->setLayoutHints({.fillWidth = true});
        card->setMargins({16, 8, 16, 8});
        for (int r = 0; r < 4; ++r) {
            auto* row = card->addChild<Row>();
            row->setLayoutHints({.fillWidth = true});
            row->setSpacing(8);
            auto* text = row->addChild<Column>();
            text->setLayoutHints({.fillWidth = true});
            for (int l = 0; l < 2; ++l) {
                leaves.push_back(text->addChild<View>());
                leaves.back()->setLayoutHints({.preferredWidth = 120, .preferredHeight = 16});
            }
            leaves.push_back(row->addChild<View>());
            leaves.back()->setLayoutHints({.preferredWidth = 60, .preferredHeight = 16});
        }
    }

    const int iterations = 4000;
    std::vector<double> costs;
    double height = 0;
    for (int i = 0; i < iterations; ++i) {
        leaves[size_t(i) % leaves.size()]->requestLayout();
        const double start = threadMilliseconds();
        height += page->heightForWidth(600 + i % 7);
        costs.push_back(threadMilliseconds() - start);
    }
    std::sort(costs.begin(), costs.end());
    printf("box layout measure of %zu leaves: %.1f us\n", leaves.size(), costs[costs.size() / 2] * 1000);
    EXPECT_GT(height, 0);
}
