// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "global/grandsearch_global.h"
#include "searcher/extend/extendworker.h"
#include "searchplugin/pluginliaison.h"

#include <stubext.h>

#include <gtest/gtest.h>

#include <QTest>

GRANDSEARCH_USE_NAMESPACE

TEST(ExtendWorkerTest, ut_setService_0)
{
    stub_ext::StubExt st;
    st.set_lamda(&PluginLiaison::init, [](){ return false; });

    ExtendWorker worker("test");
    EXPECT_FALSE(worker.setService("", "", "", ""));

    worker.m_liaison = new PluginLiaison(&worker);
    EXPECT_FALSE(worker.setService("test", "test", "test", "test"));

    delete worker.m_liaison;
    worker.m_liaison = nullptr;
    EXPECT_FALSE(worker.setService("test", "test", "test", "test"));
}

TEST(ExtendWorkerTest, ut_setService_1)
{
    stub_ext::StubExt st;
    st.set_lamda(&PluginLiaison::init, [](){ return true; });

    ExtendWorker worker("test");
    EXPECT_TRUE(worker.setService("test", "test", "test", "test"));
}

TEST(ExtendWorkerTest, ut_setContext)
{
    ExtendWorker worker("test");
    worker.setContext("test");
    EXPECT_TRUE(worker.m_context == "test");
}

TEST(ExtendWorkerTest, ut_isAsync)
{
    ExtendWorker worker("test");
    EXPECT_TRUE(worker.isAsync());
}

TEST(ExtendWorkerTest, ut_working_0)
{
    ExtendWorker worker("test");
    QString context("test");
    EXPECT_FALSE(worker.working(&context));

    worker.m_status.storeRelease(ProxyWorker::Runing);
    EXPECT_FALSE(worker.working(&context));
}

TEST(ExtendWorkerTest, ut_working_1)
{
    stub_ext::StubExt st;
    st.set_lamda(&PluginLiaison::isVaild, [](){ return true; });
    st.set_lamda(&PluginLiaison::search, [](){ return true; });

    ExtendWorker worker("test");
    worker.m_liaison = new PluginLiaison(&worker);
    worker.setContext("test");
    QString context("test");
    EXPECT_TRUE(worker.working(&context));

    st.reset(&PluginLiaison::search);
    st.set_lamda(&PluginLiaison::search, [](){ return false; });
    worker.m_status.storeRelease(ProxyWorker::Ready);
    EXPECT_FALSE(worker.working(&context));

    st.reset(&PluginLiaison::isVaild);
    st.set_lamda(&PluginLiaison::isVaild, [](){ return false; });
    worker.m_status.storeRelease(ProxyWorker::Ready);
    EXPECT_TRUE(worker.working(&context));
}

TEST(ExtendWorkerTest, ut_terminate)
{
    stub_ext::StubExt st;
    st.set_lamda(&PluginLiaison::stop, [](){ return true; });

    ExtendWorker worker("test");
    worker.m_liaison = new PluginLiaison(&worker);
    worker.terminate();
    EXPECT_TRUE(worker.status() == ProxyWorker::Terminated);
}

TEST(ExtendWorkerTest, ut_hasItem)
{
    ExtendWorker worker("test");
    EXPECT_FALSE(worker.hasItem());
}

TEST(ExtendWorkerTest, ut_takeAll)
{
    GrandSearch::MatchedItem item;
    item.name = "test";
    item.item = "test";
    item.icon = "test";
    item.type = "test";
    item.searcher = "test";

    ExtendWorker worker("test");
    worker.m_items.insert("test", {item});
    EXPECT_TRUE(!worker.takeAll().isEmpty());
}

TEST(ExtendWorkerTest, ut_tryWorking)
{
    stub_ext::StubExt st;
    st.set_lamda(&PluginLiaison::search, [](){ return false; });

    ExtendWorker worker("test");
    worker.m_liaison = new PluginLiaison(&worker);
    EXPECT_NO_FATAL_FAILURE(worker.tryWorking());

    worker.m_callSerach = ExtendWorker::WaitCall;
    EXPECT_NO_FATAL_FAILURE(worker.tryWorking());
}

TEST(ExtendWorkerTest, ut_onWorkFinished)
{
    GrandSearch::MatchedItem item;
    GrandSearch::MatchedItemMap itemMap;
    itemMap.insert("test", {item});

    ExtendWorker worker("test");
    EXPECT_NO_FATAL_FAILURE(worker.onWorkFinished(itemMap));
}
