// Copyright (C) 2019 ~ 2020 UnionTech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "logviewheaderview.h"
#include "ut_Head.h"
#include "stub.h"

#include <DApplication>

#include <QCoreApplication>
#include <QPaintEvent>
#include <QPainter>
#include <QStandardItemModel>

#include <gtest/gtest.h>

DStyle *style = nullptr;
class UT_LogviewHeaderView : public UT_HEAD
{
public:
    void SetUp()
    {
        m_Model = new QStandardItemModel;
        m_logViewHeaderView = new LogViewHeaderView(Qt::Horizontal);
        m_logViewHeaderView->setModel(m_Model);
        style = new DStyle;
    }
    void TearDown()
    {
        delete m_logViewHeaderView;
        delete m_Model;
        delete style;
    }
    LogViewHeaderView *m_logViewHeaderView;
    QStandardItemModel *m_Model;
};

int ut_headerview_pixelMetric()
{
    return 10;
}

DStyle *ut_header_style()
{
    return style;
}

TEST_F(UT_LogviewHeaderView, UT_LogviewHeaderView_paintSection)
{
    QPainter painter(m_logViewHeaderView);
    Stub stub;
    stub.set(ADDR(DApplication, style), ut_header_style); // DApplication::style()
    stub.set((int (DStyle::*)(DStyle::PixelMetric, const QStyleOption *, const QWidget * widget) const)ADDR(DStyle, pixelMetric), ut_headerview_pixelMetric);
    m_logViewHeaderView->paintSection(&painter, QRect(10, 10, 10, 10), 0);
    EXPECT_FALSE(m_logViewHeaderView->grab().isNull());
}

TEST_F(UT_LogviewHeaderView, UT_LogviewHeaderView_paintEvent)
{
    QPaintEvent paint(QRect(m_logViewHeaderView->rect()));
    Stub stub;
    stub.set(ADDR(DApplication, style), ut_header_style);
    stub.set((int (DStyle::*)(DStyle::PixelMetric, const QStyleOption *, const QWidget * widget) const)ADDR(DStyle, pixelMetric), ut_headerview_pixelMetric);
    m_logViewHeaderView->paintEvent(&paint);
    EXPECT_FALSE(m_logViewHeaderView->grab().isNull());
}

TEST_F(UT_LogviewHeaderView, UT_LogviewHeaderView_sizeHint)
{
    QSize size = m_logViewHeaderView->sizeHint();
    EXPECT_EQ(640, size.width());
    EXPECT_EQ(37, size.height());
}

TEST_F(UT_LogviewHeaderView, UT_LogviewHeaderView_sectionSizeHint)
{
    Stub stub;
    stub.set(ADDR(DApplication, style), ut_header_style);
    int size = m_logViewHeaderView->sectionSizeHint(0);

    EXPECT_EQ(38, size);
}

