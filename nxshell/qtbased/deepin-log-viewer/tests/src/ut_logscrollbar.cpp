// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "logscrollbar.h"
#include <gtest/gtest.h>
#include <stub.h>

#include <QMouseEvent>
#include <QDebug>
TEST(LogScrollBar_Constructor_UT, LogScrollBar_Constructor_UT_001)
{
    LogScrollBar *p = new LogScrollBar(nullptr);
    EXPECT_NE(p, nullptr);

    p->deleteLater();
}

TEST(LogScrollBar_Constructor_UT, LogScrollBar_Constructor_UT_002)
{
    LogScrollBar *p = new LogScrollBar(Qt::Horizontal, nullptr);
    EXPECT_NE(p, nullptr);

    p->deleteLater();
}

TEST(LogScrollBar_mousePressEvent_UT, LogScrollBar_mousePressEvent_UT_002)
{
    LogScrollBar *p = new LogScrollBar(Qt::Horizontal, nullptr);
    EXPECT_NE(p, nullptr);
    QMouseEvent mouseEvent(QEvent::MouseButtonPress, QPoint(1, 1), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    p->mousePressEvent(&mouseEvent);
    EXPECT_EQ(p->m_isOnPress, true);
    p->deleteLater();
}
