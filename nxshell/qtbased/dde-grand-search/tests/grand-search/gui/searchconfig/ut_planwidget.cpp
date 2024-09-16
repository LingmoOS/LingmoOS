// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "gui/searchconfig/planwidget.h"
#include "gui/searchconfig/switchwidget/switchwidget.h"
#include "business/config/searchconfig.h"

#include "stubext.h"

#include <gtest/gtest.h>

using namespace testing;
using namespace GrandSearch;

TEST(PlanWidgetTest, constructor)
{
    stub_ext::StubExt stu;

    bool ut_call_updateIcons = false;
    stu.set_lamda(&PlanWidget::updateIcons, [&]() {
        ut_call_updateIcons = true;
    });

    PlanWidget *w = new PlanWidget;

    EXPECT_TRUE(w);
    EXPECT_TRUE(ut_call_updateIcons);

    delete w;
}

TEST(PlanWidgetTest, onSwitchStateChanged)
{
    PlanWidget w;

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

TEST(PlanWidgetTest, updateIcons)
{
    PlanWidget w;

    stub_ext::StubExt stu;

    bool ut_call_setIcon = false;
    stu.set_lamda(&SwitchWidget::setIcon, [&]() {
        ut_call_setIcon = true;
    });

    w.updateIcons();
    EXPECT_TRUE(ut_call_setIcon);
}
