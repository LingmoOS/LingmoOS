// Copyright (C) 2019 ~ 2020 UnionTech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "logtreeview.h"
#include "ut_Head.h"
#include "stub.h"

#include <DStyle>

#include <QCoreApplication>
#include <QPaintEvent>
#include <QPainter>
#include <QStyle>
#include <QWidget>

#include <gtest/gtest.h>

class UT_LogTreeView : public UT_HEAD
{
public:
    void SetUp()
    {
        m_logTreeView = new LogTreeView;
        QStandardItem *item = new QStandardItem("Disable");
        m_logTreeView->mp_Model->setItem(0, 0, item);
        m_logTreeView->setModel(m_logTreeView->mp_Model);
    }
    void TearDown()
    {
        delete m_logTreeView;
    }
    LogTreeView *m_logTreeView;
};

TEST_F(UT_LogTreeView, UT_LogTreeView_setHeaderLabels)
{
    m_logTreeView->setHeaderLabels(QStringList() << "lables");
    EXPECT_EQ(m_logTreeView->header()->count(),1);
}

int ut_row()
{
    return 1;
}

TEST_F(UT_LogTreeView, UT_LogTreeView_currentRowEnable)
{
    Stub stub;
    stub.set(ADDR(QModelIndex, row), ut_row);
    m_logTreeView->currentRowEnable();
    ASSERT_NE(m_logTreeView->currentRow(), 0);
}

TEST_F(UT_LogTreeView, UT_LogTreeView_updateCurItemEnable)
{
    Stub stub;
    m_logTreeView->mp_Model->setColumnCount(2);
    m_logTreeView->mp_Model->insertRow(0);
    QStandardItem *item1 = new QStandardItem("item1");
    QStandardItem *item2 = new QStandardItem("item2");
    m_logTreeView->mp_Model->setItem(0,0,item1);
    m_logTreeView->mp_Model->setItem(0,1,item2);
    m_logTreeView->updateCurItemEnable(0, 0);
    EXPECT_STREQ("(Disable)item1",m_logTreeView->mp_Model->item(0,0)->text().toStdString().c_str());
    m_logTreeView->updateCurItemEnable(0, 1);
    EXPECT_STREQ("item2",m_logTreeView->mp_Model->item(0,1)->text().toStdString().c_str());
    delete item1;
    delete item2;
}

TEST_F(UT_LogTreeView, UT_LogTreeView_paintEvent)
{
    QPaintEvent paint(QRect(m_logTreeView->rect()));
    m_logTreeView->paintEvent(&paint);
    EXPECT_FALSE(m_logTreeView->grab().isNull());
}

int ut_treeview_pixelMetric()
{
    return 10;
}

TEST_F(UT_LogTreeView, UT_LogTreeView_drawRow)
{
    Stub stub;
    stub.set((int (DStyle::*)(DStyle::PixelMetric, const QStyleOption *, const QWidget *widget) const)ADDR(DStyle, pixelMetric), ut_treeview_pixelMetric);
    QStyleOptionViewItem option;
    QPainter painter(m_logTreeView);
    QModelIndex index = m_logTreeView->model()->index(0, 0);
    m_logTreeView->drawRow(&painter, option, index);
    EXPECT_FALSE(m_logTreeView->grab().isNull());
}

TEST_F(UT_LogTreeView, UT_LogTreeView_setRowNum)
{
    m_logTreeView->setRowNum(0);
    m_logTreeView->clear();
    ASSERT_EQ(m_logTreeView->RowNum(), 0);
}

TEST_F(UT_LogTreeView, UT_LogTreeView_keyPressEvent)
{
    QKeyEvent keyPressEvent(QEvent::KeyPress, Qt::Key_Up, Qt::ShiftModifier);
    EXPECT_TRUE(m_logTreeView->event(&keyPressEvent));
}

TEST_F(UT_LogTreeView, UT_LogTreeView_showEvent)
{
    QShowEvent showEvent;
    EXPECT_TRUE(m_logTreeView->event(&showEvent));
}
