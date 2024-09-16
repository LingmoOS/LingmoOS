// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "global/grandsearch_global.h"
#include "searcher/extend/extendsearcher.h"
#include "searcher/extend/extendsearcher_p.h"
#include "searcher/extend/extendworker.h"
#include "searchplugin/pluginliaison.h"

#include <stubext.h>

#include <gtest/gtest.h>

#include <QTest>
#include <QDBusConnectionInterface>

GRANDSEARCH_USE_NAMESPACE

TEST(ExtendSearcherTest, ut_setService)
{
    ExtendSearcher searcher("test");
    EXPECT_NO_FATAL_FAILURE(searcher.setService("", "", "", ""));
    EXPECT_NO_FATAL_FAILURE(searcher.setService("service", "address", "interface", "ver"));
}

TEST(ExtendSearcherTest, ut_setActivatable)
{
    ExtendSearcher searcher("test");
    searcher.setActivatable(ExtendSearcher::Inactivatable);
    EXPECT_EQ(searcher.d->m_activatable, ExtendSearcher::Inactivatable);
}

TEST(ExtendSearcherTest, ut_name)
{
    ExtendSearcher searcher("test");
    EXPECT_TRUE(searcher.name() == "test");
}

TEST(ExtendSearcherTest, ut_isActive)
{
    ExtendSearcher searcher("test");
    EXPECT_FALSE(searcher.isActive());

    stub_ext::StubExt st;
    st.set_lamda(&QDBusConnectionInterface::isServiceRegistered, [](){
        return QDBusReply<bool>();
    });
    searcher.d->m_service = "test_searvice";
    EXPECT_FALSE(searcher.isActive());
}

TEST(ExtendSearcherTest, ut_activate)
{
    stub_ext::StubExt st;
    st.set_lamda(QDBusMessage::createMethodCall, [](){
        return QDBusMessage::createError("test", "test");
    });

    ExtendSearcher searcher("test");
    searcher.d->m_activatable = ExtendSearcher::Inactivatable;
    EXPECT_FALSE(searcher.activate());

    searcher.d->m_activatable = ExtendSearcher::InnerActivation;
    EXPECT_FALSE(searcher.activate());

    searcher.d->m_activatable = ExtendSearcher::Trigger;
    EXPECT_TRUE(searcher.activate());
}

TEST(ExtendSearcherTest, ut_createWorker_0)
{
    stub_ext::StubExt st;
    st.set_lamda(&ExtendWorker::setService, [](){ return true; });

    ExtendSearcher searcher("test");
    auto worker = searcher.createWorker();
    EXPECT_TRUE(worker != nullptr);
    EXPECT_NO_FATAL_FAILURE(delete worker);
}

TEST(ExtendSearcherTest, ut_createWorker_1)
{
    stub_ext::StubExt st;
    st.set_lamda(&ExtendWorker::setService, [](){ return false; });

    ExtendSearcher searcher("test");
    auto worker = searcher.createWorker();
    EXPECT_TRUE(worker == nullptr);
}

TEST(ExtendSearcherTest, ut_action_0)
{
    stub_ext::StubExt st;
    st.set_lamda(&PluginLiaison::init, [](){ return true; });

    ExtendSearcher searcher("test");
    EXPECT_FALSE(searcher.action("", ""));

    st.set_lamda(&PluginLiaison::action, [](){ return true; });
    EXPECT_TRUE(searcher.action("", ""));
}

TEST(ExtendSearcherTest, ut_action_1)
{
    stub_ext::StubExt st;
    st.set_lamda(&PluginLiaison::init, [](){ return false; });

    ExtendSearcher searcher("test");
    EXPECT_FALSE(searcher.action("", ""));
}
