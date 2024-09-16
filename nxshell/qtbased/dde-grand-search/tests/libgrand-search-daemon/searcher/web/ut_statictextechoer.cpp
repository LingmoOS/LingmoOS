// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "global/grandsearch_global.h"
#include "searcher/web/statictextechoer.h"
#include "global/builtinsearch.h"
#include "searcher/web/statictextworker.h"

#include <stubext.h>

#include <gtest/gtest.h>

#include <QTest>

GRANDSEARCH_USE_NAMESPACE

TEST(StaticTextEchoerTest, ut_name)
{
    StaticTextEchoer ste;
    EXPECT_TRUE(ste.name() == GRANDSEARCH_CLASS_WEB_STATICTEXT);
}

TEST(StaticTextEchoerTest, ut_isActive)
{
    StaticTextEchoer ste;
    EXPECT_TRUE(ste.isActive());
}

TEST(StaticTextEchoerTest, ut_activate)
{
    StaticTextEchoer ste;
    EXPECT_FALSE(ste.activate());
}

TEST(StaticTextEchoerTest, ut_createWorker)
{
    StaticTextEchoer ste;
    auto worker = ste.createWorker();
    EXPECT_TRUE(worker != nullptr);
    EXPECT_NO_FATAL_FAILURE(delete worker);
}

TEST(StaticTextEchoerTest, ut_action)
{
    StaticTextEchoer ste;
    EXPECT_FALSE(ste.action("", ""));
}
