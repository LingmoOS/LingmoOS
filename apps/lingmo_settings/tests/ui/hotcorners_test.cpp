// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>

#include "pagetest.h"

// The tests run on a private bus, where no compositor answers.
TEST(HotCornersPage, ExplainsWhenTheCompositorIsMissing)
{
    PageFixture page("hotcorners");
    EXPECT_EQ(page.count("settings::RowComboBox"), 0);
    EXPECT_EQ(page.count("settings::Footnote"), 0);
}
