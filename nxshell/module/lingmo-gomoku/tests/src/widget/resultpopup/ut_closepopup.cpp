// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ut_closepopup.h"
#include <QMouseEvent>

TEST_F(UT_ClosePopup, UT_ClosePopup_mousePressEvent)
{
    QMouseEvent event(QMouseEvent::Type::MouseButtonPress, QPointF(1, 1), Qt::LeftButton, Qt::LeftButton, nullptr);
    m_ClosePopup->mousePressEvent(&event);
    EXPECT_EQ(m_ClosePopup->currentPixmap, m_ClosePopup->buttonPress)
            << "check closePopup mousePressEvent";
}

TEST_F(UT_ClosePopup, UT_ClosePopup_mouseReleaseEvent)
{
    QMouseEvent event(QMouseEvent::Type::MouseButtonPress, QPointF(1, 1), Qt::LeftButton, Qt::LeftButton, nullptr);
    m_ClosePopup->mouseReleaseEvent(&event);
    EXPECT_EQ(m_ClosePopup->currentPixmap, m_ClosePopup->buttonNormal)
            << "check closePopup mouseReleaseEvent";
}

TEST_F(UT_ClosePopup, UT_ClosePopup_enterEvent)
{
    QEvent event(QEvent::Enter);
    m_ClosePopup->enterEvent(&event);
    EXPECT_EQ(m_ClosePopup->currentPixmap, m_ClosePopup->buttonHover)
            << "check closePopup enterEvent";
}

TEST_F(UT_ClosePopup, UT_ClosePopup_leaveEvent)
{
    QEvent event(QEvent::Leave);
    m_ClosePopup->leaveEvent(&event);
    EXPECT_EQ(m_ClosePopup->currentPixmap, m_ClosePopup->buttonNormal)
            << "check closePopup leaveEvent";
}


TEST_F(UT_ClosePopup, UT_ClosePopup_paintEvent)
{
    QRect rect;
    QPaintEvent event(rect);
    m_ClosePopup->paintEvent(&event);
}
