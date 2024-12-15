// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ut_selectbutton.h"

#include <QEvent>
#include <QPaintEvent>

TEST_F(UT_SelectButton, UT_SelectButton_enterEvent)
{
    QEvent event(QEvent::Enter);
    m_SelectButton->enterEvent(&event);
    EXPECT_EQ(m_SelectButton->currentStatus, m_SelectButton->selectHover)
            << "check selectButton enterEvent";
}

TEST_F(UT_SelectButton, UT_SelectButton_leaveEvent1)
{
    QEvent event(QEvent::Leave);
    m_SelectButton->pressStatus = true;
    m_SelectButton->leaveEvent(&event);
    EXPECT_EQ(m_SelectButton->currentStatus, m_SelectButton->selectPress)
            << "check selectButton leaveEvent";
}

TEST_F(UT_SelectButton, UT_SelectButton_leaveEvent2)
{
    QEvent event(QEvent::Leave);
    m_SelectButton->pressStatus = false;
    m_SelectButton->leaveEvent(&event);
    EXPECT_EQ(m_SelectButton->currentStatus, m_SelectButton->selectNormal)
            << "check selectButton leaveEvent";
}

TEST_F(UT_SelectButton, UT_SelectButton_paintEvent1)
{
    QRect rect;
    m_SelectButton->setChecked(true);
    QPaintEvent event(rect);
    m_SelectButton->paintEvent(&event);
}

TEST_F(UT_SelectButton, UT_SelectButton_paintEvent2)
{
    QRect rect;
    m_SelectButton->setChecked(false);
    QPaintEvent event(rect);
    m_SelectButton->paintEvent(&event);
}
