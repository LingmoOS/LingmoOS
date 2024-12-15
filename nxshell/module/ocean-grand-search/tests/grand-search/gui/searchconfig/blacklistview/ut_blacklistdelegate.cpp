// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "global/grandsearch_global.h"
#include "gui/searchconfig/blacklistview/blacklistdelegate.h"
#include "gui/searchconfig/blacklistview/blacklistmodel.h"
#include "gui/searchconfig/blacklistview/blacklistview.h"

#include "stubext.h"

#include <gtest/gtest.h>

#include <QPainter>
#include <DGuiApplicationHelper>

using namespace testing;
GRANDSEARCH_USE_NAMESPACE

TEST(BlackListDelegateTest, constructor)
{
    BlackListView *w = new BlackListView;
    BlackListDelegate *delegate = w->m_delegate;

    ASSERT_TRUE(w);
    ASSERT_TRUE(delegate);

    delete w;
}

TEST(BlackListDelegateTest, sizeHint)
{
    BlackListView w;
    BlackListDelegate *delegate = w.m_delegate;
    QSize size(456, 36);
    EXPECT_EQ(delegate->sizeHint(QStyleOptionViewItem(), QModelIndex()), size);
}

TEST(BlackListDelegateTest, paint)
{
    BlackListView w;
    BlackListDelegate *d = w.m_delegate;

    stub_ext::StubExt stu;

    bool ut_call_paint = false;
    stu.set_lamda(VADDR(QStyledItemDelegate, paint), [&](){
        ut_call_paint = true;
    });

    bool ut_call_drawPathsText = false;
    stu.set_lamda(&BlackListDelegate::drawPathsText, [&](){
        ut_call_drawPathsText = true;
    });

    bool ut_call_drawItemBackground = false;
    stu.set_lamda(&BlackListDelegate::drawItemBackground, [&](){
        ut_call_drawItemBackground = true;
    });

    QPainter painter;
    QStyleOptionViewItem option;
    QModelIndex index;
    d->paint(&painter, option, index);
    EXPECT_TRUE(ut_call_paint);
    EXPECT_TRUE(ut_call_drawItemBackground);
    EXPECT_TRUE(ut_call_drawPathsText);
}

TEST(BlackListDelegateTest, drawPathsText)
{
    BlackListView w;
    BlackListDelegate *d = w.m_delegate;

    QModelIndex index = w.m_model->index(0, 0, QModelIndex());
    w.m_model->setData(index, "test", DATAROLE);

    stub_ext::StubExt stu;
    bool ut_call_isSelected = false;
    stu.set_lamda(&QItemSelectionModel::isSelected, [&](){
        return ut_call_isSelected;
    });

    DGuiApplicationHelper::ColorType ut_call_themeType = DGuiApplicationHelper::LightType;
    stu.set_lamda(&DGuiApplicationHelper::themeType, [&](){
        return ut_call_themeType;
    });

    bool ut_call_restore = false;
    stu.set_lamda(&QPainter::restore, [&](){
        ut_call_restore = true;
    });

    QPainter painter;
    QStyleOptionViewItem option;
    option.widget = &w;
    d->drawPathsText(&painter, option, index);
    EXPECT_TRUE(ut_call_restore);

    ut_call_themeType = DGuiApplicationHelper::DarkType;
    ut_call_restore = false;

    d->drawPathsText(&painter, option, index);
    EXPECT_TRUE(ut_call_restore);
}

TEST(BlackListWrapperTest, drawItemBackground)
{
    BlackListView w;
    BlackListDelegate *d = w.m_delegate;

    QPainter painter;
    QStyleOptionViewItem option;
    option.widget = &w;

    stub_ext::StubExt stu;

    int ut_call_row = 1;
    stu.set_lamda(&QModelIndex::row, [&](){
       return ut_call_row;
    });

    bool ut_call_restore = false;
    stu.set_lamda(&QPainter::restore, [&](){
        ut_call_restore = true;
    });

    d->drawItemBackground(&painter, option, QModelIndex());
    EXPECT_FALSE(ut_call_restore);

    ut_call_row = 2;
    d->drawItemBackground(&painter, option, QModelIndex());
    EXPECT_TRUE(ut_call_restore);
}
