// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ut_exitbutton.h"

#include <QMouseEvent>

TEST_F(UT_ExitButton, UT_ExitButton_mousePressEvent)
{
    QMouseEvent event(QMouseEvent::Type::MouseButtonPress, QPointF(1, 1), Qt::LeftButton, Qt::LeftButton, nullptr);
    m_ExitButton->mousePressEvent(&event);
    EXPECT_EQ(m_ExitButton->buttonPressed,true)
            << "check exitButton mousePressEvent";
}

TEST_F(UT_ExitButton, UT_ExitButton_mouseReleaseEvent)
{
    QMouseEvent event(QMouseEvent::Type::MouseButtonPress, QPointF(1, 1), Qt::LeftButton, Qt::LeftButton, nullptr);
    m_ExitButton->mouseReleaseEvent(&event);
    EXPECT_EQ(m_ExitButton->buttonPressed,false)
            << "check exitButton mouseReleaseEvent";
}

TEST_F(UT_ExitButton, UT_ExitButton_enterEvent)
{
    QEvent event(QEvent::Enter);
    m_ExitButton->enterEvent(&event);
    EXPECT_EQ(m_ExitButton->currentPixmap, m_ExitButton->buttonHover)
            << "check exitButton enterEvent";
}

TEST_F(UT_ExitButton, UT_ExitButton_leaveEvent)
{
    QEvent event(QEvent::Enter);
    m_ExitButton->leaveEvent(&event);
    EXPECT_EQ(m_ExitButton->currentPixmap, m_ExitButton->buttonNormal)
            << "check exitButton enterLeave";
}

TEST_F(UT_ExitButton, UT_ExitButton_paintEvent)
{
    QRect rect;
    QPaintEvent event(rect);
    m_ExitButton->buttonPressed = true;
    m_ExitButton->paintEvent(&event);
}
