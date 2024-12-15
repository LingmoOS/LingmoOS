// Copyright (C) 2019 ~ 2020 UnionTech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "TipsWidget.h"
#include "stub.h"

#include <DPlatformWindowHandle>
#include <DGuiApplicationHelper>
#include <DFontSizeManager>

#include <QCoreApplication>
#include <QPaintEvent>
#include <QPainter>

#include <gtest/gtest.h>

class UT_TipsWidget : public ::testing::Test
{
public:
    void SetUp()
    {
        m_tester = new TipsWidget;
    }
    void TearDown()
    {
        delete m_tester;
    }

protected:
    TipsWidget *m_tester;
};

TEST_F(UT_TipsWidget, inittest)
{

}

TEST_F(UT_TipsWidget, UT_TipsWidget_onUpdateTheme)
{
    m_tester->onUpdateTheme();
    DPalette plt = m_tester->palette();
    EXPECT_TRUE(plt.color(Dtk::Gui::DPalette::Background) == plt.color(Dtk::Gui::DPalette::Base));
}

TEST_F(UT_TipsWidget, UT_TipsWidget_setText)
{
    m_tester->setText("text");
    EXPECT_STREQ("text", m_tester->m_text.toStdString().c_str());
}

TEST_F(UT_TipsWidget, UT_TipsWidget_setAlignment)
{
    m_tester->setAlignment(Qt::AlignTop);
    EXPECT_EQ(Qt::AlignTop, m_tester->m_alignment);
}

TEST_F(UT_TipsWidget, UT_TipsWidget_setLeftRightMargin)
{
    m_tester->setLeftRightMargin(0);
    EXPECT_TRUE(m_tester->m_lrMargin == 0);
}

TEST_F(UT_TipsWidget, UT_TipsWidget_setTopBottomMargin)
{
    m_tester->setTopBottomMargin(0);
    EXPECT_TRUE(m_tester->m_tbMargin == 0);
}

TEST_F(UT_TipsWidget, UT_TipsWidget_setMaxLineCount)
{
    m_tester->setMaxLineCount(0);
    EXPECT_TRUE(m_tester->m_maxLineCount == 0);
}

TEST_F(UT_TipsWidget, UT_TipsWidget_adjustContent)
{
    QFontMetricsF fontMetris(m_tester->font());
    int wordHeight = static_cast<int>(fontMetris.boundingRect(QRectF(0, 0, m_tester->width() - 2 * m_tester->m_lrMargin, 0),
                                                              static_cast<int>(m_tester->m_alignment | Qt::TextWrapAnywhere), "text").height() + 2 * m_tester->m_tbMargin);
    m_tester->adjustContent("text");
    EXPECT_EQ(m_tester->height(), wordHeight);
}

TEST_F(UT_TipsWidget, UT_TipsWidget_PaintEvent)
{
    QPaintEvent paint(QRect(m_tester->rect()));
    m_tester->paintEvent(&paint);
    EXPECT_FALSE(m_tester->grab().isNull());
}

static QString g_funcname;
void qTimer_start_stub2()
{
    g_funcname = __FUNCTION__;
    return;
}

TEST_F(UT_TipsWidget, UT_TipsWidget_showEvent)
{
    Stub stub;
    stub.set((void(QTimer::*)())ADDR(QTimer, start), qTimer_start_stub2);
    m_tester->m_autoChecked = true;
    QShowEvent *event = new QShowEvent();
    m_tester->showEvent(event);
    delete event;
    EXPECT_TRUE(g_funcname == "qTimer_start_stub2");
}

TEST_F(UT_TipsWidget, UT_TipsWidget_hideEvent)
{
    QHideEvent *event = new QHideEvent();
    m_tester->hideEvent(event);
    delete event;
    EXPECT_TRUE(m_tester->m_timer.isActive() == false);
}

TEST_F(UT_TipsWidget, UT_TipsWidget_setAutoChecked)
{
    m_tester->setAutoChecked(true);
    EXPECT_TRUE(m_tester->m_autoChecked == true);
}

TEST_F(UT_TipsWidget, UT_TipsWidget_onTimeOut)
{
    m_tester->onTimeOut();
    EXPECT_TRUE(m_tester->isVisible() == false);
}
