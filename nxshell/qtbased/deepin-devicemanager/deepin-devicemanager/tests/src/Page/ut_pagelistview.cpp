// Copyright (C) 2019 ~ 2020 UnionTech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "PageListView.h"
#include "DeviceListView.h"

#include "DeviceInput.h"
#include "PageInfo.h"
#include "PageMultiInfo.h"
#include "stub.h"
#include "ut_Head.h"

#include <QCoreApplication>
#include <QPaintEvent>
#include <QPainter>

#include <gtest/gtest.h>

class PageListView_UT : public UT_HEAD
{
public:
    void SetUp()
    {
        m_pageListView = new PageListView;
    }
    void TearDown()
    {
        delete m_pageListView;
    }
    PageListView *m_pageListView = nullptr;
};

TEST_F(PageListView_UT, PageListView_UT_updateListItems)
{
    QList<QPair<QString, QString>> list;
    list.append(QPair<QString, QString>("/", "/"));
    m_pageListView->updateListItems(list);
    EXPECT_EQ(0, m_pageListView->mp_ListView->mp_ItemModel->rowCount());
}

TEST_F(PageListView_UT, PageListView_UT_currentIndex)
{
    ASSERT_EQ(m_pageListView->currentIndex(), "");
    ASSERT_NE(m_pageListView->currentType(), "");
}

TEST_F(PageListView_UT, PageListView_UT_paintEvent)
{
    QPaintEvent paint(QRect(m_pageListView->rect()));
    m_pageListView->paintEvent(&paint);
}

QString ut_getConcatenateStrings()
{
    return "/";
}

TEST_F(PageListView_UT, PageListView_UT_slotListViewItemClicked)
{
    Stub stub;
    stub.set(ADDR(DeviceListView, getConcatenateStrings), ut_getConcatenateStrings);
    QModelIndex index;
    m_pageListView->slotListViewItemClicked(index);
    EXPECT_EQ("/", m_pageListView->m_CurType);
}

void ut_listview_exec()
{
    return;
}

TEST_F(PageListView_UT, PageListView_UT_slotShowMenu)
{
    Stub stub;
    stub.set((QAction * (QMenu::*)(const QPoint &, QAction *))ADDR(QMenu, exec), ut_listview_exec);

    m_pageListView->slotShowMenu(QPoint(0, 0));
    EXPECT_EQ(m_pageListView->mp_Menu->actions().size(), 0);
}
