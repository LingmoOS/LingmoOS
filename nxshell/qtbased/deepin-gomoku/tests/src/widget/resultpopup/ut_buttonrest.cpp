// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ut_buttonrest.h"
#include <QMouseEvent>

TEST_F(UT_ButtonRest, UT_ButtonRest_setResult1)
{
    m_ButtonRest->setResult(true);
    EXPECT_EQ(m_ButtonRest->mResult, true)
            << "check buttonRest setResult";
}

TEST_F(UT_ButtonRest, UT_ButtonRest_setResult2)
{
    m_ButtonRest->setResult(false);
    EXPECT_EQ(m_ButtonRest->mResult, false)
            << "check buttonRest setResult";
}

TEST_F(UT_ButtonRest, UT_ButtonRest_mousePressEvent1)
{
    QMouseEvent event(QMouseEvent::Type::MouseButtonPress, QPointF(1, 1), Qt::LeftButton, Qt::LeftButton, nullptr);
    m_ButtonRest->mResult = true;
    m_ButtonRest->mousePressEvent(&event);
    EXPECT_EQ(m_ButtonRest->currentPixmap, m_ButtonRest->winRestPress)
            << "check buttonRest mousePressEvent";
}

TEST_F(UT_ButtonRest, UT_ButtonRest_mousePressEvent2)
{
    QMouseEvent event(QMouseEvent::Type::MouseButtonPress, QPointF(1, 1), Qt::LeftButton, Qt::LeftButton, nullptr);
    m_ButtonRest->mResult = false;
    m_ButtonRest->mousePressEvent(&event);
    EXPECT_EQ(m_ButtonRest->currentPixmap, m_ButtonRest->failRestPress)
            << "check buttonRest mousePressEvent";
}

TEST_F(UT_ButtonRest, UT_ButtonRest_mouseReleaseEvent1)
{
    QMouseEvent event(QMouseEvent::Type::MouseButtonPress, QPointF(1, 1), Qt::LeftButton, Qt::LeftButton, nullptr);
    m_ButtonRest->mResult = true;
    m_ButtonRest->mouseReleaseEvent(&event);
    EXPECT_EQ(m_ButtonRest->currentPixmap, m_ButtonRest->winRestNormal)
            << "check buttonRest mouseReleaseEvent";
}

TEST_F(UT_ButtonRest, UT_ButtonRest_mouseReleaseEvent2)
{
    QMouseEvent event(QMouseEvent::Type::MouseButtonPress, QPointF(1, 1), Qt::LeftButton, Qt::LeftButton, nullptr);
    m_ButtonRest->mResult = false;
    m_ButtonRest->mouseReleaseEvent(&event);
    EXPECT_EQ(m_ButtonRest->currentPixmap, m_ButtonRest->failRestNormal)
            << "check buttonRest mouseReleaseEvent";
}

TEST_F(UT_ButtonRest, UT_ButtonRest_enterEvent1)
{
    QEvent event(QEvent::Enter);
    m_ButtonRest->mResult = true;
    m_ButtonRest->enterEvent(&event);
    EXPECT_EQ(m_ButtonRest->currentPixmap, m_ButtonRest->winRestHover)
            << "check buttonRest enterEvent";
}

TEST_F(UT_ButtonRest, UT_ButtonRest_enterEvent2)
{
    QEvent event(QEvent::Enter);
    m_ButtonRest->mResult = false;
    m_ButtonRest->enterEvent(&event);
    EXPECT_EQ(m_ButtonRest->currentPixmap, m_ButtonRest->failRestHover)
            << "check buttonRest enterEvent";
}

TEST_F(UT_ButtonRest, UT_ButtonRest_leaveEvent1)
{
    QEvent event(QEvent::Leave);
    m_ButtonRest->mResult = true;
    m_ButtonRest->leaveEvent(&event);
    EXPECT_EQ(m_ButtonRest->currentPixmap, m_ButtonRest->winRestNormal)
            << "check buttonRest enterEvent";
}

TEST_F(UT_ButtonRest, UT_ButtonRest_leaveEvent2)
{
    QEvent event(QEvent::Leave);
    m_ButtonRest->mResult = false;
    m_ButtonRest->leaveEvent(&event);
    EXPECT_EQ(m_ButtonRest->currentPixmap, m_ButtonRest->failRestNormal)
            << "check buttonRest enterEvent";
}

TEST_F(UT_ButtonRest, UT_ButtonRest_paintEvent1)
{
    QRect rect;
    QPaintEvent event(rect);
    m_ButtonRest->buttonPressed = true;
    m_ButtonRest->currentPixmap = m_ButtonRest->winRestPress;
    m_ButtonRest->paintEvent(&event);
}

TEST_F(UT_ButtonRest, UT_ButtonRest_paintEvent2)
{
    QRect rect;
    QPaintEvent event(rect);
    m_ButtonRest->buttonPressed = true;
    m_ButtonRest->currentPixmap = m_ButtonRest->failRestPress;
    m_ButtonRest->paintEvent(&event);
}
