// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "global/searchhelper.h"

#include <gtest/gtest.h>

TEST(SearchHelperTest, ut_parseKeyword)
{
    QStringList groupList, suffixList, keywordList;
    EXPECT_FALSE(searchHelper->parseKeyword("test", groupList, suffixList, keywordList));
    EXPECT_TRUE(searchHelper->parseKeyword("text::png:b", groupList, suffixList, keywordList));
}

TEST(SearchHelperTest, ut_isSuffix)
{
    EXPECT_FALSE(searchHelper->isSuffix(""));
    EXPECT_TRUE(searchHelper->isSuffix("zip"));
}

TEST(SearchHelperTest, ut_isGroupName)
{
    EXPECT_FALSE(searchHelper->isGroupName(""));
    EXPECT_TRUE(searchHelper->isGroupName("file"));
}

TEST(SearchHelperTest, ut_getSearcherByGroupName)
{
    EXPECT_TRUE(searchHelper->getSearcherByGroupName("test").isEmpty());
    EXPECT_FALSE(searchHelper->getSearcherByGroupName("file").isEmpty());
}

TEST(SearchHelperTest, ut_getSuffixByGroupName)
{
    EXPECT_TRUE(searchHelper->getSuffixByGroupName("test").isEmpty());
    EXPECT_FALSE(searchHelper->getSuffixByGroupName("file").isEmpty());
}

TEST(SearchHelperTest, ut_tropeInputSymbol)
{
    auto str = searchHelper->tropeInputSymbol("test*");
    EXPECT_EQ(str, "test\\*");
}
