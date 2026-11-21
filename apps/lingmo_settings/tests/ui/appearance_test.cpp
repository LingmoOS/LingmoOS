// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>

#include "lingmo/appearance/appearance.h"

// The tests run on a private bus, where no appearance service answers: a
// change goes out, and the value stays until the service reports one, as
// the Qt version's did.
TEST(AppearanceClient, WaitsForTheServiceToReportAChange)
{
    Lingmo::appearance::Appearance appearance;
    int changes = 0;
    Lingmo::base::Subscription subscription = appearance.changed.connect([&changes] { ++changes; });
    const int accent = appearance.accentColor();
    appearance.setAccentColor(accent + 1);
    appearance.setDarkModeSchedule(Lingmo::appearance::Appearance::SunsetToSunrise);
    EXPECT_EQ(appearance.accentColor(), accent);
    EXPECT_EQ(appearance.darkModeSchedule(), int(Lingmo::appearance::Appearance::Never));
    EXPECT_EQ(changes, 0);
}
