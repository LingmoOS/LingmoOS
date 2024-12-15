// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "global/grandsearch_global.h"
#include "gui/searchconfig/configwidget.h"

#include "stubext.h"

#include <gtest/gtest.h>

using namespace testing;
GRANDSEARCH_USE_NAMESPACE

TEST(ConfigWidgetTest, constructor)
{
    stub_ext::StubExt stu;

    bool ut_call_init = false;

    stu.set_lamda(ADDR(ConfigWidget, initUI), [&](){
        ut_call_init = true;
    });

    ConfigWidget *w = new ConfigWidget;

    EXPECT_TRUE(w);
    EXPECT_TRUE(ut_call_init);

    delete w;
}

TEST(ConfigWidgetTest, initUI)
{
    ConfigWidget w;

    EXPECT_TRUE(w.m_searchGroupWidget);
    EXPECT_TRUE(w.m_searchCustomWidget);
}
