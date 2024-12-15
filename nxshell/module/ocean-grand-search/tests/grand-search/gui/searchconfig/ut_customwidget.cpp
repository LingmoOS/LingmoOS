// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "gui/searchconfig/customwidget.h"
#include "gui/searchconfig/switchwidget/switchwidget.h"
#include "business/config/searchconfig.h"

#include "stubext.h"

#include <gtest/gtest.h>

#include <QFileInfo>
#include <QDir>

using namespace testing;
using namespace GrandSearch;

TEST(CustomWidgetTest, constructor)
{
    CustomWidget *w = new CustomWidget;

    EXPECT_TRUE(w);
    EXPECT_TRUE(w->m_mainLayout);
    EXPECT_TRUE(w->m_groupLabel);
    EXPECT_TRUE(w->m_searchPlanWidget);
    EXPECT_TRUE(w->m_tailerWidget);
    EXPECT_TRUE(w->m_bestMatchWidget);
    EXPECT_TRUE(w->m_searchEngineWidget);
    EXPECT_TRUE(w->m_innerLayout);

    delete w;
}

TEST(CustomWidgetTest, onSwitchStateChanged)
{
    CustomWidget w;

    stub_ext::StubExt stu;

    SwitchWidget *switchWidget = new SwitchWidget;
    stu.set_lamda(&QObject::sender, [&]() {
        return switchWidget;
    });

    bool ut_call_setConfig = false;
    stu.set_lamda(&SearchConfig::setConfig, [&]() {
        ut_call_setConfig = true;
    });

    w.onSwitchStateChanged(true);

    EXPECT_TRUE(ut_call_setConfig);

    delete switchWidget;
}

