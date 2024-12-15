// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ut_buttonagain.h"
#include <QMouseEvent>

TEST_F(UT_ButtonAgain, UT_ButtonAgain_setResult1)
{
    m_ButtonAgain->setResult(true);
    EXPECT_EQ(m_ButtonAgain->mResult, true)
            << "check buttonAgain setResult";
}

TEST_F(UT_ButtonAgain, UT_ButtonAgain_setResult2)
{
    m_ButtonAgain->setResult(false);
    EXPECT_EQ(m_ButtonAgain->mResult, false)
            << "check buttonAgain setResult";
}

TEST_F(UT_ButtonAgain, UT_ButtonAgain_mousePressEvent1)
{
    QMouseEvent event(QMouseEvent::Type::MouseButtonPress, QPointF(1, 1), Qt::LeftButton, Qt::LeftButton, nullptr);
    m_ButtonAgain->mResult = true;
    m_ButtonAgain->mousePressEvent(&event);
    EXPECT_EQ(m_ButtonAgain->currentPixmap, m_ButtonAgain->againPress)
            << "check buttonRest mousePressEvent";
}

TEST_F(UT_ButtonAgain, UT_ButtonAgain_mousePressEvent2)
{
    QMouseEvent event(QMouseEvent::Type::MouseButtonPress, QPointF(1, 1), Qt::LeftButton, Qt::LeftButton, nullptr);
    m_ButtonAgain->mResult = false;
    m_ButtonAgain->mousePressEvent(&event);
    EXPECT_EQ(m_ButtonAgain->currentPixmap, m_ButtonAgain->failAgainPress)
            << "check buttonRest mousePressEvent";
}

TEST_F(UT_ButtonAgain, UT_ButtonAgain_mouseReleaseEvent)
{
    QMouseEvent event(QMouseEvent::Type::MouseButtonPress, QPointF(1, 1), Qt::LeftButton, Qt::LeftButton, nullptr);
    m_ButtonAgain->mResult = true;
    m_ButtonAgain->mouseReleaseEvent(&event);
    EXPECT_EQ(m_ButtonAgain->currentPixmap, m_ButtonAgain->againNormal)
            << "check buttonRest mouseReleaseEvent";
}

TEST_F(UT_ButtonAgain, UT_ButtonAgain_enterEvent)
{
    QEvent event(QEvent::Enter);
    m_ButtonAgain->mResult = true;
    m_ButtonAgain->enterEvent(&event);
    EXPECT_EQ(m_ButtonAgain->currentPixmap, m_ButtonAgain->againHover)
            << "check buttonRest enterEvent";
}

TEST_F(UT_ButtonAgain, UT_ButtonAgain_leaveEvent)
{
    QEvent event(QEvent::Leave);
    m_ButtonAgain->mResult = true;
    m_ButtonAgain->leaveEvent(&event);
    EXPECT_EQ(m_ButtonAgain->currentPixmap, m_ButtonAgain->againNormal)
            << "check buttonRest enterEvent";
}

TEST_F(UT_ButtonAgain, UT_ButtonAgain_paintEvent1)
{
    QRect rect;
    QPaintEvent event(rect);
    m_ButtonAgain->buttonPressed = true;
    m_ButtonAgain->currentPixmap = m_ButtonAgain->againPress;
    m_ButtonAgain->paintEvent(&event);
}

TEST_F(UT_ButtonAgain, UT_ButtonAgain_paintEvent2)
{
    QRect rect;
    QPaintEvent event(rect);
    m_ButtonAgain->buttonPressed = true;
    m_ButtonAgain->currentPixmap = m_ButtonAgain->failAgainPress;
    m_ButtonAgain->paintEvent(&event);
}
