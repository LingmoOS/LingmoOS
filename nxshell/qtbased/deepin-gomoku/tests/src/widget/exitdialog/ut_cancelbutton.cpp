// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ut_cancelbutton.h"

#include <QMouseEvent>

TEST_F(UT_CancelButton, UT_CancelButton_mousePressEvent)
{
    QMouseEvent event(QMouseEvent::Type::MouseButtonPress, QPointF(1, 1), Qt::LeftButton, Qt::LeftButton, nullptr);
    m_CalcelButton->mousePressEvent(&event);
    EXPECT_EQ(m_CalcelButton->buttonPressed, true)
            << "check calcelButton mousePressEvent";
}

TEST_F(UT_CancelButton, UT_CancelButton_mouseReleaseEvent)
{
    QMouseEvent event(QMouseEvent::Type::MouseButtonPress, QPointF(1, 1), Qt::LeftButton, Qt::LeftButton, nullptr);
    m_CalcelButton->mouseReleaseEvent(&event);
    EXPECT_EQ(m_CalcelButton->buttonPressed, false)
            << "check calcelButton mouseReleaseEvent";
}

TEST_F(UT_CancelButton, UT_CancelButton_enterEvent)
{
    QEvent event(QEvent::Enter);
    m_CalcelButton->enterEvent(&event);
    EXPECT_EQ(m_CalcelButton->currentPixmap, m_CalcelButton->buttonHover)
            << "check calcelButton enterEvent";
}

TEST_F(UT_CancelButton, UT_CancelButton_leaveEvent)
{
    QEvent event(QEvent::Leave);
    m_CalcelButton->leaveEvent(&event);
    EXPECT_EQ(m_CalcelButton->currentPixmap, m_CalcelButton->buttonNormal)
            << "check calcelButton enterEvent";
}

TEST_F(UT_CancelButton, UT_CancelButton_paintEvent)
{
    QRect rect;
    QPaintEvent event(rect);
    m_CalcelButton->buttonPressed = true;
    m_CalcelButton->paintEvent(&event);
}
