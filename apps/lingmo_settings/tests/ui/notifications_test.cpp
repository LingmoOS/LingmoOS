// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>

#include "pagetest.h"

TEST(NotificationsPage, ShowsTheEmptyCardWithoutApplications)
{
    PageFixture page("notifications");
    EXPECT_EQ(page.count("settings::SwitchRow"), 1);
    EXPECT_EQ(page.count("settings::RowComboBox"), 0);
    EXPECT_EQ(page.count("settings::Footnote"), 0);
}
