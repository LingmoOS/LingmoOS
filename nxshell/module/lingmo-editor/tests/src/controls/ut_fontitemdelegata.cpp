// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ut_fontitemdelegata.h"
#include "../src/controls/fontitemdelegate.h"
#include <QPainter>
test_fontitemdelegata::test_fontitemdelegata()
{

}

TEST_F(test_fontitemdelegata, FontItemDelegate)
{
    FontItemDelegate* delegate = new FontItemDelegate();

    EXPECT_NE(delegate,nullptr);

    delete delegate;delegate=nullptr;
}


TEST_F(test_fontitemdelegata, paint)
{
    FontItemDelegate* delegate = new FontItemDelegate();
    QPainter* painter = new QPainter;
    QStyleOptionViewItem option;
    QModelIndex index;
    option.state |= QStyle::State_Selected;
    delegate->paint(painter,option,index);

    EXPECT_NE(delegate,nullptr);
    EXPECT_NE(painter->pen().color(),Qt::white);

    delete painter;painter=nullptr;
    delete delegate;delegate= nullptr;

}

TEST_F(test_fontitemdelegata, sizeHint)
{
    FontItemDelegate* delegate = new FontItemDelegate();
    QPainter* painter = new QPainter;
    QStyleOptionViewItem option;
    QModelIndex index;
    option.state |= QStyle::State_Selected;

    EXPECT_EQ(delegate->sizeHint(option,index),QSize(-1, 30));
    EXPECT_NE(delegate,nullptr);

    delete painter;painter=nullptr;
    delete delegate;delegate=nullptr;

}
