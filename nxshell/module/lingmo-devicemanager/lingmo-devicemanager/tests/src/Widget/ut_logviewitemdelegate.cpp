// Copyright (C) 2019 ~ 2020 UnionTech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "logviewitemdelegate.h"
#include "logtreeview.h"
#include "ut_Head.h"
#include "stub.h"

#include <DApplication>

#include <QCoreApplication>
#include <QPaintEvent>
#include <QPainter>

#include <gtest/gtest.h>

DStyle *logViewItemDelegateSytle = nullptr;
class UT_LogViewItemDelegate : public UT_HEAD
{
public:
    void SetUp()
    {
        m_Model = new QStandardItemModel;
        m_view = new LogTreeView;
        m_view->setModel(m_Model);
        m_Model->appendRow(new QStandardItem());
        m_logDelegate = new LogViewItemDelegate(m_view);
        logViewItemDelegateSytle = new DStyle;
    }
    void TearDown()
    {
        delete m_logDelegate;
        delete m_view;
        delete m_Model;
        delete logViewItemDelegateSytle;
    }
    LogViewItemDelegate *m_logDelegate;
    LogTreeView *m_view;
    QStandardItemModel *m_Model;
};

int ut_itemdelegate_pixelMetric()
{
    return 10;
}

DStyle *ut_style()
{
    return logViewItemDelegateSytle;
}

TEST_F(UT_LogViewItemDelegate, UT_LogViewItemDelegate_paint)
{
    QPainter painter(m_view);
    QStyleOptionViewItem option;
    QModelIndex index = m_view->model()->index(0, 0);

    Stub stub;
    stub.set((int (DStyle::*)(DStyle::PixelMetric, const QStyleOption *, const QWidget * widget) const)ADDR(DStyle, pixelMetric), ut_itemdelegate_pixelMetric);
    stub.set(ADDR(DApplication, style), ut_style); // DApplication::style()
    m_logDelegate->paint(&painter, option, index);
    EXPECT_FALSE(m_view->grab().isNull());
}

TEST_F(UT_LogViewItemDelegate, UT_LogViewItemDelegate_createEditor)
{
    QPainter painter(m_view);
    QStyleOptionViewItem option;
    QModelIndex index = m_view->model()->index(0, 0);
    EXPECT_FALSE(m_logDelegate->createEditor(nullptr, option, index));
}

TEST_F(UT_LogViewItemDelegate, UT_LogViewItemDelegate_sizeHint)
{
    QPainter painter(m_view);
    QStyleOptionViewItem option;
    QModelIndex index = m_view->model()->index(0, 0);
    QSize size = m_logDelegate->sizeHint(option, index);
    EXPECT_EQ(6, size.width());
    EXPECT_EQ(36, size.height());
}
