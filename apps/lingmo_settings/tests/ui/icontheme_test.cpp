// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>

#include <algorithm>

#include "pages/icontheme/iconthemes.h"
#include "pagetest.h"

TEST(IconThemePage, ListsTheInstalledThemes)
{
    const auto themes = settings::IconTheme::installed();
    // hicolor is a fallback, not a theme to choose.
    EXPECT_TRUE(std::none_of(themes.begin(), themes.end(), [](const auto& theme) { return theme.id == "hicolor"; }));
    PageFixture page("appearance", "icontheme");
    EXPECT_EQ(page.count("settings::ImageSelectionItem"), int(themes.size()));
}
