// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>

#include <cstdio>
#include <string>
#include <unistd.h>
#include <vector>

#include "base/file.h"
#include "input_test_support.h"
#include "pages/keyboard/keyboard.h"
#include "ui/test_input.h"
#include "ui/window.h"
#include "window/settingswindow.h"

using namespace Lingmo::ui;
using namespace settings;

static constexpr const char* kRules = R"(<?xml version="1.0" encoding="UTF-8"?>
<xkbConfigRegistry version="1.1">
  <modelList>
    <model><configItem><name>pc105</name><description>Generic 105-key PC</description></configItem></model>
  </modelList>
  <layoutList>
    <layout>
      <configItem><name>us</name><shortDescription>en</shortDescription><description>English (US)</description></configItem>
      <variantList>
        <variant><configItem><name>intl</name><description>English (US, intl., with dead keys)</description></configItem></variant>
      </variantList>
    </layout>
    <layout>
      <configItem><name>custom</name><description>A user-defined custom Layout</description></configItem>
    </layout>
    <layout>
      <configItem><name>de</name><description>German</description></configItem>
      <variantList>
        <variant><configItem><name>nodeadkeys</name><description>German (no dead keys)</description></configItem></variant>
      </variantList>
    </layout>
    <layout>
      <configItem><name>ad</name><description>catalan &amp; andorra</description></configItem>
    </layout>
  </layoutList>
  <optionList>
    <group><configItem><name>grp</name><description>Switching to another layout</description></configItem></group>
  </optionList>
</xkbConfigRegistry>
)";

TEST(KeyboardLayouts, ReadsLayoutsAndVariantsSortedByName)
{
    char path[] = "/tmp/settings-rules-XXXXXX";
    const int fd = mkstemp(path);
    ASSERT_GE(fd, 0);
    close(fd);
    ASSERT_TRUE(Lingmo::base::file::writeAtomically(path, kRules));
    const std::vector<KeyboardLayout> layouts = readKeyboardLayouts(path);
    unlink(path);

    // The custom placeholder is left out; case does not order the names, and
    // a comma sorts before a closing parenthesis, as with QCollator.
    ASSERT_EQ(layouts.size(), 5u);
    EXPECT_EQ(layouts[0].id, "ad");
    EXPECT_EQ(layouts[0].name, "catalan & andorra");
    EXPECT_EQ(layouts[1].id, "us-intl");
    EXPECT_EQ(layouts[1].name, "English (US, intl., with dead keys)");
    EXPECT_EQ(layouts[2].id, "us");
    EXPECT_EQ(layouts[3].id, "de");
    EXPECT_EQ(layouts[4].id, "de-nodeadkeys");
}

TEST(KeyboardLayoutPage, FiltersByNameOrIdAndSaysWhenNothingMatches)
{
    if (keyboardLayouts().empty())
        GTEST_SKIP() << "no xkb rules file";
    SettingsWindow settingsWindow;
    Window& window = settingsWindow.window();
    window.setScale(1.5);
    window.resize({900, 610});
    settingsWindow.reveal();
    settingsWindow.switchToPage("keyboard");
    settingsWindow.pushPage("keyboardlayout");
    window.grab();

    // The list fills the page below the search field, with the page margins.
    const RectF search = settings::testing::windowRect(window, "keyboardlayout:search");
    const RectF list = settings::testing::windowRect(window, "keyboardlayout:list");
    EXPECT_EQ(search.x, 254);
    EXPECT_EQ(search.y, 56);
    EXPECT_EQ(list.y, search.y + search.height + 16);
    EXPECT_EQ(list.y + list.height, 610 - 24);
    EXPECT_TRUE(settings::testing::isHidden(window, "keyboardlayout:empty"));

    TestInput::setKeyboardFocus(window, true);
    TestInput::click(window, {search.x + 40, search.y + search.height / 2});
    TestInput::typeText(window, "zzzz");
    window.grab();
    EXPECT_FALSE(settings::testing::isHidden(window, "keyboardlayout:empty"));
}
