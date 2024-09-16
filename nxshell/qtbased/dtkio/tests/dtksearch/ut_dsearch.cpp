// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dsearch.h"
#include "dtksearchtypes.h"
#include "maincontroller/maincontroller.h"

#include <gtest/gtest.h>
#include <stubext.h>

DSEARCH_USE_NAMESPACE
TEST(TestDSearch, search)
{
    stub_ext::StubExt st;
    st.set_lamda(&MainController::doSearchTask, [] { return true; });

    DSearch search;
    EXPECT_FALSE(search.search("", ""));

    search.setIndexPath("/tmp/index");
    EXPECT_TRUE(search.search("/tmp", "test"));
}

TEST(TestDSearch, stop)
{
    stub_ext::StubExt st;
    st.set_lamda(&MainController::stop, [] { return; });

    DSearch search;
    EXPECT_TRUE(search.stop());
}

TEST(TestDSearch, matchedResults)
{
    stub_ext::StubExt st;
    st.set_lamda(&MainController::getResults, [] { return QStringList("/tmp/test"); });

    DSearch search;
    const auto &result = search.matchedResults();
    EXPECT_FALSE(result.isEmpty());
}

TEST(TestDSearch, setResultFilter)
{
    ResultFilterFunc filter = [](const QString &str) {
        Q_UNUSED(str)
        return true;
    };

    DSearch search;
    EXPECT_NO_FATAL_FAILURE(search.setResultFilter(filter));
}

TEST(TestDSearch, maxResultCount)
{
    DSearch search;
    search.setMaxResultCount(100);

    EXPECT_EQ(100, search.maxResultCount());
}

TEST(TestDSearch, searchFlags)
{
    DSearch search;
    SearchFlags flags = SearchFlag::FileName | SearchFlag::FullText;
    search.setSearchFlags(flags);

    EXPECT_EQ(flags, search.searchFlags());
}

TEST(TestDSearch, indexPath)
{
    DSearch search;
    search.setIndexPath("/tmp/index");

    EXPECT_EQ("/tmp/index", search.indexPath());
}
