// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>

#include <chrono>
#include <string>

#include "input_test_support.h"
#include "pagetest.h"
#include "ui/slider.h"
#include "ui/switch.h"
#include "ui/test_input.h"
#include "widgets/sliderrow.h"
#include "widgets/rowcombobox.h"
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
    EXPECT_EQ(row->value(), dragged);
}

} // namespace

TEST(DockPage, HasEveryControl)
{
    PageFixture page("dock");
    EXPECT_EQ(page.count("settings::ImageSelectionItem"), 3);
    EXPECT_EQ(page.count("settings::SliderRow"), 3);
    EXPECT_NE(findView<settings::RowComboBox>(page.window->window().contentView(), "dock:visibility"), nullptr);
    EXPECT_EQ(page.count("settings::NavigationItem"), 3);
    EXPECT_EQ(page.count("settings::SwitchRow"), 4);
}

TEST(DockPage, SizeStaysWhereItIsDragged)
{
    PageFixture page("dock");
    expectDragHolds(page, "dock:size", 0.8);
}

TEST(DockPage, GlassEffectStaysWhereItIsDragged)
{
    PageFixture page("dock");
    expectDragHolds(page, "dock:glass", 0.3);
}

TEST(DockPage, MagnificationFactorStaysWhereItIsDragged)
{
    PageFixture page("dock");
    // The slider is only usable with the magnification on; a test host may
    // have it off in its own shell.conf.
    Window& window = page.window->window();
    auto* toggle = findView<settings::SwitchRow>(window.contentView(), "dock:magnification");
    ASSERT_NE(toggle, nullptr);
    if (!toggle->isChecked()) {
        const RectF control = toggle->control()->geometry();
        TestInput::click(window, toggle->control()->mapToWindow({control.width / 2, control.height / 2}));
    }
    ASSERT_TRUE(toggle->isChecked());
    expectDragHolds(page, "dock:magnification-factor", 0.7);
}

TEST(DockPage, TheMagnificationSwitchGatesItsSize)
{
    PageFixture page("dock");
    Window& window = page.window->window();
    auto* toggle = findView<settings::SwitchRow>(window.contentView(), "dock:magnification");
    auto* size = findView<SliderRow>(window.contentView(), "dock:magnification-factor");
    ASSERT_NE(toggle, nullptr);
    ASSERT_NE(size, nullptr);
    const bool on = toggle->isChecked();
    EXPECT_EQ(on, size->slider()->isEnabled());
    const double chosen = size->value();

    // The switch turns the slider with it, keeping the size that was chosen.
    const RectF control = toggle->control()->geometry();
    TestInput::click(window, toggle->control()->mapToWindow({control.width / 2, control.height / 2}));
    EXPECT_EQ(!on, toggle->isChecked());
    EXPECT_EQ(toggle->isChecked(), size->slider()->isEnabled());
    EXPECT_EQ(chosen, size->value());

    TestInput::click(window, toggle->control()->mapToWindow({control.width / 2, control.height / 2}));
    EXPECT_EQ(on, toggle->isChecked());
    EXPECT_EQ(toggle->isChecked(), size->slider()->isEnabled());
}

TEST(DockPage, TheBounceSwitchTurnsOff)
{
    PageFixture page("dock");
    Window& window = page.window->window();
    auto* bounce = findView<settings::SwitchRow>(window.contentView(), "dock:bounce");
    ASSERT_NE(bounce, nullptr);
    const bool on = bounce->isChecked();
    const RectF control = bounce->control()->geometry();
    TestInput::click(window, bounce->control()->mapToWindow({control.width / 2, control.height / 2}));
    EXPECT_EQ(!on, bounce->isChecked());
}
