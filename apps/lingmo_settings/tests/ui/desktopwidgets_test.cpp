// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "pagetest.h"

#include <gtest/gtest.h>

TEST(DesktopWidgetsPage, OffersOpeningTheGallery)
{
    PageFixture page("desktopwidgets");
    EXPECT_NE(page.dump().find("\"desktopwidgets:edit\""), std::string::npos);
    EXPECT_NE(page.dump().find("\"desktopwidgets:weather-city\""), std::string::npos);
}
