// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "gui/searchconfig/bestmatchwidget.h"
#include "gui/searchconfig/switchwidget/switchwidget.h"
#include "business/config/searchconfig.h"

#include "stubext.h"

#include <gtest/gtest.h>

#include <QFileInfo>
#include <QDir>

using namespace testing;
using namespace GrandSearch;

TEST(BestMatchWidgetTest, constructor)
{
    stub_ext::StubExt stu;

    bool ut_call_updateIcons = false;
    stu.set_lamda(&BestMatchWidget::updateIcons, [&]() {
        ut_call_updateIcons = true;
    });

    BestMatchWidget *w = new BestMatchWidget;

    EXPECT_TRUE(w);
    EXPECT_TRUE(ut_call_updateIcons);
    EXPECT_TRUE(w->m_mainLayout);
    EXPECT_TRUE(w->m_groupLabel);

    delete w;
}

TEST(BestMatchWidgetTest, onSwitchStateChanged)
{
    BestMatchWidget w;

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

TEST(BestMatchWidgetTest, updateIcons)
{
    BestMatchWidget w;

    stub_ext::StubExt stu;

    bool ut_call_setIcon = false;
    stu.set_lamda(&SwitchWidget::setIcon, [&]() {
        ut_call_setIcon = true;
    });

    w.updateIcons();
    EXPECT_TRUE(ut_call_setIcon);
}
