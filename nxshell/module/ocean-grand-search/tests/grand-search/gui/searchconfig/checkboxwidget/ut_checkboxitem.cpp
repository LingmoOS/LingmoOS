// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "global/grandsearch_global.h"
#include "gui/searchconfig/checkboxwidget/checkboxitem.h"
#include "stubext.h"

#include <gtest/gtest.h>

#include <QPaintEvent>

using namespace testing;
GRANDSEARCH_USE_NAMESPACE

TEST(CheckBoxItemTest, setChecked)
{
    CheckBoxItem item("test");
    item.setChecked(true);
    EXPECT_TRUE(item.m_checkBox->isChecked());
}

TEST(CheckBoxItemTest, setTopRound)
{
    CheckBoxItem item("test");
    item.setTopRound(true);
    EXPECT_TRUE(item.m_topRound);
}

TEST(CheckBoxItemTest, setBottomRound)
{
    CheckBoxItem item("test");
    item.setBottomRound(true);
    EXPECT_TRUE(item.m_bottomRound);
}

TEST(CheckBoxItemTest, text)
{
    CheckBoxItem item("test");
    EXPECT_EQ(item.text(), "test");
}

TEST(CheckBoxItemTest, paintEvent1)
{
    QPaintEvent event(QRect(QPoint(10, 10), QSize(20, 20)));
    CheckBoxItem item("test");
    EXPECT_NO_FATAL_FAILURE(item.paintEvent(&event));
}

TEST(CheckBoxItemTest, paintEvent2)
{
    QPaintEvent event(QRect(QPoint(10, 10), QSize(20, 20)));
    CheckBoxItem item("test");
    item.m_topRound = true;
    item.m_bottomRound = true;

    EXPECT_NO_FATAL_FAILURE(item.paintEvent(&event));
}
