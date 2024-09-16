// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "global/grandsearch_global.h"
#include "searcher/web/statictextworker.h"
#include "global/builtinsearch.h"
#include "configuration/configer.h"
#include "global/searchconfigdefine.h"
#include "global/searchhelper.h"

#include <stubext.h>

#include <gtest/gtest.h>

#include <QTest>

GRANDSEARCH_USE_NAMESPACE

TEST(StaticTextWorker, ut_setContext)
{
    StaticTextWorker worker(GRANDSEARCH_CLASS_WEB_STATICTEXT);
    EXPECT_NO_FATAL_FAILURE(worker.setContext(""));
}

TEST(StaticTextWorker, ut_isAsync)
{
    StaticTextWorker worker(GRANDSEARCH_CLASS_WEB_STATICTEXT);
    EXPECT_FALSE(worker.isAsync());
}

TEST(StaticTextWorker, ut_working_0)
{
    StaticTextWorker worker(GRANDSEARCH_CLASS_WEB_STATICTEXT);
    EXPECT_TRUE(worker.working(nullptr));

    worker.m_status.storeRelease(ProxyWorker::Runing);
    EXPECT_FALSE(worker.working(nullptr));
}

TEST(StaticTextWorker, ut_working_1)
{
    StaticTextWorker worker(GRANDSEARCH_CLASS_WEB_STATICTEXT);
    worker.setContext("test");
    stub_ext::StubExt stu;
    stu.set_lamda(&Configer::group, [](){
        QVariantHash datas;
        datas.insert(GRANDSEARCH_WEB_SEARCHENGINE, GRANDSEARCH_WEB_SEARCHENGINE_GOOGLE);
        UserPreferencePointer u(new UserPreference(datas));
        return u;
    });

    EXPECT_TRUE(worker.working(nullptr));
}

TEST(StaticTextWorker, ut_terminate)
{
    StaticTextWorker worker(GRANDSEARCH_CLASS_WEB_STATICTEXT);
    worker.terminate();
    EXPECT_EQ(worker.status(), ProxyWorker::Terminated);
}

TEST(StaticTextWorker, ut_hasItem)
{
    StaticTextWorker worker(GRANDSEARCH_CLASS_WEB_STATICTEXT);
    EXPECT_FALSE(worker.hasItem());
}

TEST(StaticTextWorker, ut_takeAll)
{
    StaticTextWorker worker(GRANDSEARCH_CLASS_WEB_STATICTEXT);
    GrandSearch::MatchedItem item;
    item.item = "test";
    item.name = "Search for test";
    item.type = "application/x-dde-grand-search-web-statictext";
    item.icon = item.type;
    item.searcher = worker.name();

    worker.m_items << item;
    EXPECT_FALSE(worker.takeAll().isEmpty());
}

TEST(StaticTextWorker, ut_createUrl)
{
    bool language = true;
    stub_ext::StubExt stu;
    stu.set_lamda(&SearchHelper::isSimplifiedChinese, [&](){
        return language;
    });

    StaticTextWorker s(GRANDSEARCH_CLASS_WEB_STATICTEXT);
    QString result = s.createUrl(GRANDSEARCH_WEB_SEARCHENGINE_SOGOU);
    EXPECT_EQ(result, "https://www.sogou.com/web?query=%0");

    language = false;
    result = s.createUrl(GRANDSEARCH_WEB_SEARCHENGINE_YAHOO);
    EXPECT_EQ(result, "https://search.yahoo.com/search?p=%0");
}
