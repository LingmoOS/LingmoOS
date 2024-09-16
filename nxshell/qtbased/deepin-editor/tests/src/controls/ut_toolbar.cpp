// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ut_toolbar.h"
#include "../src/controls/toolbar.h"
test_toolbar::test_toolbar()
{

}

TEST_F(test_toolbar, ToolBar)
{
    ToolBar* tool = new ToolBar();

    EXPECT_NE(tool,nullptr);
    delete tool;
    tool = nullptr;

}

TEST_F(test_toolbar, setTabbar)
{
    ToolBar* tool = new ToolBar();
    tool->setTabbar(nullptr);

    EXPECT_NE(tool->m_layout->count(),0);

    EXPECT_NE(tool,nullptr);
    tool->deleteLater();

}
