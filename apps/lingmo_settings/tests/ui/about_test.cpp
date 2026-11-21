// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>

#include "pagetest.h"
#include "pages/about/aboutinfo.h"

TEST(AboutPage, ShowsTheSystemRows)
{
    PageFixture page("about");
    EXPECT_NE(page.dump().find("\"page:about\""), std::string::npos);
    // PC name and OS version; the model moved into the system card's resources.
    EXPECT_EQ(page.count("settings::InfoRow"), 2);
    EXPECT_NE(page.dump().find("InfoCell"), std::string::npos);
    EXPECT_NE(page.dump().find("\"about:copy-info\""), std::string::npos);
    EXPECT_NE(page.dump().find("\"about:more-info\""), std::string::npos);
    EXPECT_NE(page.dump().find("\"about:edit-pc-name\""), std::string::npos);
    EXPECT_NE(page.dump().find("\"about:aurora-version\""), std::string::npos);
    EXPECT_NE(page.dump().find("\"about:aurora-version-label\""), std::string::npos);
}

TEST(AboutPage, FormatsSizesAsTheQtVersionDid)
{
    using settings::about::formatByteSize;
    EXPECT_EQ(formatByteSize(512), "512 B");
    EXPECT_EQ(formatByteSize(1536), "2 KB");
    EXPECT_EQ(formatByteSize(16.0 * 1024 * 1024 * 1024), "16 GB");
}

TEST(AboutPage, NamesTheComputerModel)
{
    using settings::about::computerModel;
    EXPECT_EQ(computerModel("VENDOR", "Book 13", "AB-1234", ""), "VENDOR Book 13");
    EXPECT_EQ(computerModel("Vendor Inc.", "Vendor Book", "", ""), "Vendor Book");
    EXPECT_EQ(computerModel("Vendor", "To be filled by O.E.M.", "Book 14", ""), "Vendor Book 14");
    EXPECT_EQ(computerModel("System manufacturer", "", "System Product Name", ""), "");
    EXPECT_EQ(computerModel("", "", "", "Board Model B"), "Board Model B");
    EXPECT_EQ(computerModel("Default string", "Default string", "Default string", ""), "");
}
