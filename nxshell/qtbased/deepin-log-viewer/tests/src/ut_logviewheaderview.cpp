// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "logviewheaderview.h"

#include <gtest/gtest.h>
#include <stub.h>

#include <QDebug>
#include <QPainter>
#include <QEvent>
#include <QFocusEvent>
#include <DStyle>

int stub_pixelMetric(DStyle::PixelMetric m, const QStyleOption *opt = nullptr, const QWidget *widget = nullptr)
{
    return 1;
}

void stub_drawPrimitive(DStyle::PrimitiveElement pe, const QStyleOption *opt, QPainter *p, const QWidget *w = nullptr)
{
}

class LogViewHeaderView_UT : public testing::Test
{
public:
    //添加日志
    void SetUp() //TEST跑之前会执行SetUp
    {
        m_LogViewHeaderView = new LogViewHeaderView(Qt::Vertical);
        qDebug() << "SetUp" << endl;
    }
    void TearDown() //TEST跑完之后会执行TearDown
    {
        delete m_LogViewHeaderView;
    }
    LogViewHeaderView *m_LogViewHeaderView;
};

TEST_F(LogViewHeaderView_UT, LogViewHeaderView_Constructor_UT_001)
{
    EXPECT_NE(m_LogViewHeaderView, nullptr);
}

TEST(LogViewHeaderView_paintSection_UT, LogViewHeaderView_paintSection_UT_001)
{
    Stub stub;
    stub.set((int (DStyle::*)(DStyle::PixelMetric, const QStyleOption *, const QWidget *) const)ADDR(DStyle, pixelMetric), stub_pixelMetric);
    stub.set((void (DStyle::*)(DStyle::PrimitiveElement, const QStyleOption *, QPainter *, const QWidget *) const)ADDR(DStyle, drawPrimitive), stub_drawPrimitive);
    LogViewHeaderView *p = new LogViewHeaderView(Qt::Horizontal, nullptr);
    EXPECT_NE(p, nullptr);
    QPainter painter;
    p->paintSection(&painter, p->rect(), 0);
    p->deleteLater();
}

TEST_F(LogViewHeaderView_UT, LogViewHeaderView_focusInEvent_UT)
{
    EXPECT_NE(m_LogViewHeaderView, nullptr);
    QFocusEvent *e = new QFocusEvent(QEvent::FocusOut, Qt::ShortcutFocusReason);
    m_LogViewHeaderView->focusInEvent(e);
    EXPECT_EQ(m_LogViewHeaderView->m_reson, Qt::ShortcutFocusReason);
    delete e;
}

TEST_F(LogViewHeaderView_UT, LogViewHeaderView_paintEvent_UT_001)
{
    Stub stub;
    stub.set((int (DStyle::*)(DStyle::PixelMetric, const QStyleOption *, const QWidget *) const)ADDR(DStyle, pixelMetric), stub_pixelMetric);
    EXPECT_NE(m_LogViewHeaderView, nullptr);
    QPaintEvent repaint(m_LogViewHeaderView->rect());
    m_LogViewHeaderView->paintEvent(&repaint);
}

TEST_F(LogViewHeaderView_UT, LogViewHeaderView_sizeHint_UT_001)
{
    QSize size= m_LogViewHeaderView->sizeHint();
    EXPECT_NE(m_LogViewHeaderView, nullptr);
    EXPECT_EQ(size, QSize(640,37))<<"check the status after sizeHint()";
}

TEST(LogViewHeaderView_sectionSizeHint_UT, LogViewHeaderView_sectionSizeHint_UT_001)
{
    Stub stub;
    stub.set((int (DStyle::*)(DStyle::PixelMetric, const QStyleOption *, const QWidget *) const)ADDR(DStyle, pixelMetric), stub_pixelMetric);
    LogViewHeaderView *p = new LogViewHeaderView(Qt::Horizontal, nullptr);
    EXPECT_NE(p, nullptr);
    p->sectionSizeHint(0);
    p->deleteLater();
}
