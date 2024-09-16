// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "global/grandsearch_global.h"
#include "gui/searchconfig/indexwidget.h"

#include "stubext.h"

#include <gtest/gtest.h>

using namespace testing;
GRANDSEARCH_USE_NAMESPACE

TEST(IndexWidgetText, constructor)
{
    IndexWidget *indexWidget = new IndexWidget;

    EXPECT_TRUE(indexWidget->m_mainLayout);
    EXPECT_TRUE(indexWidget->m_groupLabel);
    EXPECT_TRUE(indexWidget->m_blackListWidget);

    delete indexWidget;
}
