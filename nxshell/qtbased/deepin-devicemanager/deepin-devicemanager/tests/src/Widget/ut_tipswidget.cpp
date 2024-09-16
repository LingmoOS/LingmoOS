// Copyright (C) 2019 ~ 2020 UnionTech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "TipsWidget.h"
#include "ut_Head.h"

#include <DPlatformWindowHandle>
#include <DGuiApplicationHelper>
#include <DFontSizeManager>

#include <QCoreApplication>
#include <QPaintEvent>
#include <QPainter>

#include <gtest/gtest.h>

class UT_TipsWidget : public UT_HEAD
{
public:
    void SetUp()
    {
        tWidget = new TipsWidget;
    }
    void TearDown()
    {
        delete tWidget;
    }
    TipsWidget *tWidget;
};

TEST_F(UT_TipsWidget, UT_TipsWidget_onUpdateTheme)
{
    tWidget->onUpdateTheme();
    DPalette plt = tWidget->palette();
    EXPECT_TRUE(plt.color(Dtk::Gui::DPalette::Background) == plt.color(Dtk::Gui::DPalette::Base));
}

TEST_F(UT_TipsWidget, UT_TipsWidget_setText)
{
    tWidget->setText("text");
    EXPECT_STREQ("text",tWidget->m_text.toStdString().c_str());
}

TEST_F(UT_TipsWidget, UT_TipsWidget_setAlignment)
{
    tWidget->setAlignment(Qt::AlignTop);
    EXPECT_EQ(Qt::AlignTop,tWidget->m_alignment);
}

TEST_F(UT_TipsWidget, UT_TipsWidget_adjustContent)
{
    QFontMetricsF fontMetris(tWidget->font());
    int wordHeight = static_cast<int>(fontMetris.boundingRect(QRectF(0, 0, tWidget->width() - 2 * tWidget->m_lrMargin, 0),
                     static_cast<int>(tWidget->m_alignment | Qt::TextWrapAnywhere), "text").height() + 2 * tWidget->m_tbMargin);
    tWidget->adjustContent("text");
    EXPECT_EQ(tWidget->height(),wordHeight);
}

TEST_F(UT_TipsWidget, UT_TipsWidget_PaintEvent)
{
    QPaintEvent paint(QRect(tWidget->rect()));
    tWidget->paintEvent(&paint);
    EXPECT_FALSE(tWidget->grab().isNull());
}
