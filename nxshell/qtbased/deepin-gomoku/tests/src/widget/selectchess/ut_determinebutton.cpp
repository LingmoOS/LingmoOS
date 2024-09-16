// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ut_determinebutton.h"

#include <QEvent>
#include <QMouseEvent>
#include <QPaintEvent>

TEST_F(UT_DetermineButton, UT_DetermineButton_mousePressEvent)
{
    QMouseEvent event(QMouseEvent::Type::MouseButtonPress, QPointF(1, 1), Qt::LeftButton, Qt::LeftButton, nullptr);
    m_DetermineButton->mousePressEvent(&event);
    EXPECT_EQ(m_DetermineButton->buttonPressed, true)
            << "check determineButton mousePressEvent";
}

TEST_F(UT_DetermineButton, UT_DetermineButton_mouseReleaseEvent)
{
    QMouseEvent event(QMouseEvent::Type::MouseButtonPress, QPointF(1, 1), Qt::LeftButton, Qt::LeftButton, nullptr);
    m_DetermineButton->mouseReleaseEvent(&event);
    EXPECT_EQ(m_DetermineButton->buttonPressed, false)
            << "check determineButton mouseReleaseEvent";
}

TEST_F(UT_DetermineButton, UT_DetermineButton_enterEvent)
{
    QEvent event(QEvent::Enter);
    m_DetermineButton->enterEvent(&event);
    EXPECT_EQ(m_DetermineButton->currentPixmap, m_DetermineButton->buttonHover)
            << "check determineButton enterEvent";
}

TEST_F(UT_DetermineButton, UT_DetermineButton_leaveEvent)
{
    QEvent event(QEvent::Enter);
    m_DetermineButton->leaveEvent(&event);
    EXPECT_EQ(m_DetermineButton->buttonPressed, false)
            << "check determineButton leaveEvent";
}

TEST_F(UT_DetermineButton, UT_DetermineButton_paintEvent)
{
    QRect rect;
    QPaintEvent event(rect);
    m_DetermineButton->buttonPressed = true;
    m_DetermineButton->paintEvent(&event);
}
