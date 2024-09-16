// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifdef ENABLE_DEEPINANYTHING

#include "global/grandsearch_global.h"
#include "searcher/file/filenamesearcher.h"
#include "global/builtinsearch.h"
#include "configuration/configer.h"
#include "searcher/file/filenameworker.h"

#include <stubext.h>

#include <gtest/gtest.h>

#include <QTest>
#include <QDBusInterface>
#include <QDBusReply>

GRANDSEARCH_USE_NAMESPACE

TEST(FileNameSearcherTest, ut_name)
{
    FileNameSearcher searcher;
    EXPECT_EQ(searcher.name(), GRANDSEARCH_CLASS_FILE_DEEPIN);
}

TEST(FileNameSearcherTest, ut_isActive_0)
{
    FileNameSearcher searcher;
    stub_ext::StubExt st;
    st.set_lamda(&QDBusInterface::isValid, [](){ return false; });

    EXPECT_FALSE(searcher.isActive());
}

TEST(FileNameSearcherTest, ut_isActive_1)
{
    FileNameSearcher searcher;
    stub_ext::StubExt st;
    st.set_lamda(&QDBusInterface::isValid, [](){ return true; });
    st.set_lamda(&QDBusReply<QStringList>::isValid, [](){ return false; });

    EXPECT_TRUE(searcher.isActive());
}

TEST(FileNameSearcherTest, ut_isActive_2)
{
    FileNameSearcher searcher;
    stub_ext::StubExt st;
    st.set_lamda(&QDBusInterface::isValid, [](){ return true; });
    st.set_lamda(&QDBusReply<QStringList>::isValid, [](){ return true; });
    st.set_lamda(&QStringList::isEmpty, [](){ return true; });

    EXPECT_TRUE(searcher.isActive());
}

TEST(FileNameSearcherTest, ut_activate)
{
    FileNameSearcher searcher;
    EXPECT_FALSE(searcher.activate());
}

TEST(FileNameSearcherTest, ut_createWorker)
{
    stub_ext::StubExt st;
    //空配置，测试默认值
    UserPreferencePointer upp(new UserPreference(QVariantHash()));
    st.set_lamda(&Configer::group, [&upp](Configer *, const QString &name) {
        if (name == GRANDSEARCH_CLASS_FILE_DEEPIN)
            return upp;

        return UserPreferencePointer();
    });

    FileNameSearcher searcher;
    auto worker = searcher.createWorker();
    EXPECT_TRUE(worker != nullptr);
    EXPECT_NO_FATAL_FAILURE(delete worker);
}

TEST(FileNameSearcherTest, ut_action)
{
    FileNameSearcher searcher;
    EXPECT_FALSE(searcher.action("", ""));
}

#endif
