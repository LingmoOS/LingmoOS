// Copyright (C) 2019 ~ 2020 UnionTech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "DetailViewDelegate.h"
#include "DetailTreeView.h"
#include "ut_Head.h"
#include "stub.h"

#include <DStyle>
#include <DApplication>

#include <QCoreApplication>
#include <QPaintEvent>
#include <QPainter>
#include <QCommonStyle>
#include <QStyle>
#include <QWidget>

#include <gtest/gtest.h>
DStyle *UT_DetailViewDelegate_style;
class UT_DetailViewDelegate : public UT_HEAD
{
public:
    void SetUp()
    {
        m_treeView = new DetailTreeView;
        m_dViewDelegate = new DetailViewDelegate(m_treeView);
        m_treeView->setItemDelegate(m_dViewDelegate);
        UT_DetailViewDelegate_style = new DStyle;
    }
    void TearDown()
    {
        delete m_treeView;
        delete  UT_DetailViewDelegate_style;
        UT_DetailViewDelegate_style = nullptr;
    }
    DetailViewDelegate *m_dViewDelegate;
    DetailTreeView *m_treeView;
};

int ut_y()
{
    return 10;
}

DStyle *DetailViewDelegate_UT_style_fun()
{
    return UT_DetailViewDelegate_style;
}

TEST_F(UT_DetailViewDelegate, UT_DetailViewDelegate_paint_001)
{
    QStyleOptionViewItem option;
    QPainter painter(m_treeView);
    m_treeView->setColumnCount(1);
    m_treeView->insertRow(0);
    QTableWidgetItem *item0 = new QTableWidgetItem("xxx");
    m_treeView->setItem(0, 0, item0);
    QModelIndex index0 = m_treeView->model()->index(0, 0);


    Stub stub;
    stub.set(ADDR(DApplication, style), DetailViewDelegate_UT_style_fun);
    stub.set(ADDR(QPoint, y), ut_y);

    m_dViewDelegate->paint(&painter, option, index0);
    EXPECT_FALSE(m_treeView->grab().isNull());

    delete item0;
}

TEST_F(UT_DetailViewDelegate, UT_DetailViewDelegate_paint_002)
{
    QStyleOptionViewItem option;
    QPainter painter(m_treeView);
    m_treeView->setColumnCount(2);
    m_treeView->insertRow(0);
    QTableWidgetItem *item0 = new QTableWidgetItem("xxx");
    m_treeView->setItem(0, 0, item0);
    QTableWidgetItem *item1 = new QTableWidgetItem("xxx");
    m_treeView->setItem(0, 1, item1);
    QModelIndex index1 = m_treeView->model()->index(0, 1);

    Stub stub;
    stub.set(ADDR(DApplication, style), DetailViewDelegate_UT_style_fun);
    stub.set(ADDR(QPoint, y), ut_y);

    m_dViewDelegate->paint(&painter, option, index1);
    EXPECT_FALSE(m_treeView->grab().isNull());

    delete item0;
    delete item1;
}

TEST_F(UT_DetailViewDelegate, UT_DetailViewDelegate_paint_003)
{
    QStyleOptionViewItem option;
    QPainter painter(m_treeView);
    m_treeView->setColumnCount(1);
    m_treeView->insertRow(0);
    QTableWidgetItem *item0 = new QTableWidgetItem("xxx");
    m_treeView->setItem(0, 0, item0);
    m_treeView->insertRow(1);
    QTableWidgetItem *item2 = new QTableWidgetItem("xxx");
    m_treeView->setItem(1, 0, item2);
    QModelIndex index2 = m_treeView->model()->index(1, 0);

    Stub stub;
    stub.set(ADDR(DApplication, style), DetailViewDelegate_UT_style_fun);
    stub.set(ADDR(QPoint, y), ut_y);

    m_dViewDelegate->paint(&painter, option, index2);
    EXPECT_FALSE(m_treeView->grab().isNull());

    delete item0;
    delete item2;
}

TEST_F(UT_DetailViewDelegate, UT_DetailViewDelegate_paint_004)
{
    QStyleOptionViewItem option;
    QPainter painter(m_treeView);
    m_treeView->setColumnCount(2);
    m_treeView->insertRow(0);
    QTableWidgetItem *item0 = new QTableWidgetItem("xxx");
    m_treeView->setItem(0, 0, item0);
    QTableWidgetItem *item1 = new QTableWidgetItem("xxx");
    m_treeView->setItem(0, 1, item1);
    m_treeView->insertRow(1);
    QTableWidgetItem *item2 = new QTableWidgetItem("xxx");
    m_treeView->setItem(1, 0, item2);
    QTableWidgetItem *item3 = new QTableWidgetItem("xxx");
    m_treeView->setItem(1, 1, item3);

    QModelIndex index3 = m_treeView->model()->index(1, 1);

    Stub stub;
    stub.set(ADDR(DApplication, style), DetailViewDelegate_UT_style_fun);
    stub.set(ADDR(QPoint, y), ut_y);

    m_dViewDelegate->paint(&painter, option, index3);
    EXPECT_FALSE(m_treeView->grab().isNull());

    delete item0;
    delete item1;
    delete item2;
    delete item3;
}

TEST_F(UT_DetailViewDelegate, UT_DetailViewDelegate_createEditor)
{
    QStyleOptionViewItem m_item;

    QTableWidgetItem *item = new QTableWidgetItem;
    item->setText("/");
    m_treeView->setItem(0, 0, item);
    QModelIndex index = m_treeView->model()->index(0, 0);

    EXPECT_FALSE(m_dViewDelegate->createEditor(nullptr, m_item, index));
    delete item;
}

TEST_F(UT_DetailViewDelegate, UT_DetailViewDelegate_sizeHint)
{
    QStyleOptionViewItem m_item;

    QTableWidgetItem *item = new QTableWidgetItem;
    item->setText("/");
    m_treeView->setItem(0, 0, item);
    QModelIndex index = m_treeView->model()->index(0, 0);

    QSize size = m_dViewDelegate->sizeHint(m_item, index);
    EXPECT_EQ(150, size.width());
    EXPECT_EQ(50, size.height());
    delete item;
}
