// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>

#include <string>

#include "ui/window.h"
#include "widgets/listrow.h"
#include "window/options.h"
#include "window/page.h"
#include "window/settingswindow.h"

using namespace Lingmo::ui;
using namespace settings;

namespace {

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

struct PowerPageTest : testing::Test
{
    void open(bool battery)
    {
        setOptions({.mockBattery = battery});
        settingsWindow = std::make_unique<SettingsWindow>();
        settingsWindow->window().setScale(1.5);
        settingsWindow->window().resize({900, 610});
        settingsWindow->reveal();
        settingsWindow->switchToPage("power");
        settingsWindow->window().grab();
    }

    void TearDown() override { setOptions({}); }

    ListRow* row(const std::string& name)
    {
        return dynamic_cast<ListRow*>(findView(settingsWindow->currentPage(), name));
    }

    std::unique_ptr<SettingsWindow> settingsWindow;
};

} // namespace

TEST_F(PowerPageTest, OnBatteryEachSourceHasItsTimeout)
{
    open(true);
    ASSERT_TRUE(row("power:batteryscreenoff"));
    EXPECT_TRUE(row("power:batteryscreenoff")->isVisible());
    EXPECT_EQ(row("power:batteryscreenoff")->text(), "Turn off display when inactive on battery");
    EXPECT_EQ(row("power:acscreenoff")->text(), "Turn off display when inactive on power adapter");
    EXPECT_EQ(row("power:preventsleep")->text(),
              "Prevent automatic sleeping on power adapter when the display is off");
}

TEST_F(PowerPageTest, WithoutABatteryThereIsOneTimeout)
{
    // The battery service has not answered here, so there is none.
    open(false);
    EXPECT_FALSE(row("power:batteryscreenoff")->isVisible());
    EXPECT_EQ(row("power:acscreenoff")->text(), "Turn off display when inactive");
    EXPECT_EQ(row("power:preventsleep")->text(), "Prevent automatic sleeping when the display is off");
}

TEST_F(PowerPageTest, ModesWaitForTheDaemon)
{
    open(true);
    // No profiles before power-profiles-daemon answers.
    EXPECT_FALSE(findView(settingsWindow->currentPage(), "power:mode")->isVisible());
}
