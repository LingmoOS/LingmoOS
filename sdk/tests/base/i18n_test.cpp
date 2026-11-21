// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>

#include <string>

#include "base/i18n.h"

using namespace Lingmo;

TEST(I18nTest, Format)
{
    EXPECT_EQ(i18n::format("Quit {name}?", {{"name", "Files"}}), "Quit Files?");
    EXPECT_EQ(i18n::format("{used} of {total}", {{"total", "8 GiB"}, {"used", "2 GiB"}}), "2 GiB of 8 GiB");
    EXPECT_EQ(i18n::format("{a}{a}", {{"a", "x"}}), "xx");
    EXPECT_EQ(i18n::format("Keep {unknown} and {", {{"name", "x"}}), "Keep {unknown} and {");
    EXPECT_EQ(i18n::format("Value: {v}", {{"v", "{n}"}}), "Value: {n}");
    EXPECT_EQ(i18n::format("", {}), "");
}

TEST(I18nTest, UntranslatedTextIsTheSource)
{
    i18n::setDomain("lingmo-toolkit-i18n-test");
    EXPECT_STREQ(i18n::tr("Open File"), "Open File");
    EXPECT_STREQ(i18n::trc("menu", "Open"), "Open");
    EXPECT_STREQ(i18n::dtr("lingmo-toolkit-i18n-test", "Cancel"), "Cancel");
    EXPECT_STREQ(i18n::trNoop("Later"), "Later");
    EXPECT_STREQ(i18n::tr(""), "");
}

TEST(I18nTest, PluralsFillInTheCount)
{
    i18n::setDomain("lingmo-toolkit-i18n-test");
    EXPECT_EQ(i18n::trn("{n} item", "{n} items", 1), "1 item");
    EXPECT_EQ(i18n::trn("{n} item", "{n} items", 0), "0 items");
    EXPECT_EQ(i18n::trn("{n} item", "{n} items", 3), "3 items");
}
