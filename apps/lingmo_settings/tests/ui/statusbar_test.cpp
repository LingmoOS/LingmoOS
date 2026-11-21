// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>

#include <chrono>
#include <string>

#include "input_test_support.h"
#include "pagetest.h"
#include "ui/slider.h"
#include "ui/test_input.h"
#include "widgets/sliderrow.h"
#include "window/page.h"

using namespace Lingmo::ui;
using settings::SliderRow;
using settings::testing::findView;
using settings::testing::waitFor;

namespace {

PointF trackPoint(const Slider& slider, double fraction)
{
    const RectF geometry = slider.geometry();
    const double handle = 22;
    return slider.mapToWindow({handle / 2 + (geometry.width - handle) * fraction, geometry.height / 2});
}

// Drags the row's handle and waits out the release and the settle after the
// last move, when the row puts the handle on the setting's value.
void expectDragHolds(PageFixture& page, const std::string& name, double fraction)
{
    Window& window = page.window->window();
    auto* row = findView<SliderRow>(window.contentView(), name);
    ASSERT_NE(row, nullptr);
    auto* pageView=page.window->currentPage();
    const auto position=pageView->contentView()->mapFromWindow(row->mapToWindow({0,0}));
    pageView->ensureVisible({position.x,position.y,row->geometry().width,row->geometry().height});
    window.grab();
    Slider& slider = *row->slider();
    const double before = row->value();

    TestInput::press(window, trackPoint(slider, fraction - 0.1));
    TestInput::movePointer(window, trackPoint(slider, fraction));
    TestInput::release(window, trackPoint(slider, fraction));
    const double dragged = row->value();
    ASSERT_NE(dragged, before);

    waitFor([] { return false; }, std::chrono::milliseconds(600));
    EXPECT_NEAR(row->value(), dragged, 1e-9);
}

} // namespace

TEST(StatusBarPage, HasBothSliders)
{
    PageFixture page("statusbar");
    EXPECT_EQ(page.count("settings::SliderRow"), 2);
}

TEST(StatusBarPage, BlurStaysWhereItIsDragged)
{
    PageFixture page("statusbar");
    expectDragHolds(page, "statusbar:blur", 0.7);
}

TEST(StatusBarPage, TransparencyStaysWhereItIsDragged)
{
    PageFixture page("statusbar");
    expectDragHolds(page, "statusbar:transparency", 0.3);
}
