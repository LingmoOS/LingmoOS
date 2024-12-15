// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ut_themeitemdelegate.h"
#include "../../src/thememodule/themeitemdelegate.h"
#include <QPainter>

test_ThemeItemDelegate::test_ThemeItemDelegate()
{
}

void test_ThemeItemDelegate::SetUp()
{
    tid = new ThemeItemDelegate();
    EXPECT_NE(tid,nullptr);
}

void test_ThemeItemDelegate::TearDown()
{
    delete tid;
    tid = nullptr;
}

TEST_F(test_ThemeItemDelegate, paint)
{
    QPainter *painter = new QPainter();
    const QModelIndex index;
    const QStyleOptionViewItem option;
    //    ThemeItemDelegate tid;
    tid->paint(painter, option, index);

    EXPECT_NE(painter,nullptr);

    delete painter;
    painter=nullptr;
}

TEST_F(test_ThemeItemDelegate, sizeHint)
{
    const QModelIndex index;
    const QStyleOptionViewItem option;
    EXPECT_NE(tid->sizeHint(option, index).width(),0);
}
